#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    inputFilesTable = new InputFilesTable;
    layout->addWidget(inputFilesTable);
    ui->fileTableWidget->setLayout(layout);
    QObject::connect(ui->addFileButton, &QPushButton::clicked,
        inputFilesTable, &InputFilesTable::addFile);
    QObject::connect(ui->changeOutputDirButton, &QPushButton::clicked,
        this, &MainWindow::onChangeOutputDirButtonClicked);
    QObject::connect(ui->startButton, &QPushButton::clicked,
        this, &MainWindow::onStartButtonClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onStartButtonClicked() { workingProcessAsync(); }

void MainWindow::workingProcessAsync()
{
    enableControls(false);
    QtConcurrent::run([this]()
    {
        QObject::connect(this, &MainWindow::unlock, this,
                         [this]() { enableControls(true); });
        filesProcessing();
        emit unlock();
    });
}

void MainWindow::filesProcessing()
{
    QDir outputDirPath = QDir(ui->outputDirLineEdit->text()).absolutePath();
    // if outputDirLineEdit is empty, files are saved in "." directory
    QVector<QString> filePaths;
    QString fileNameMask = ui->fileNameMaskLineEdit->text();
    for (int i = 0; i < inputFilesTable->rowCount(); ++i)
    {
        QString path_ = inputFilesTable->item(i, 0)->text();
        if (fileNameMask.isEmpty() || path_.contains(fileNameMask))
        {
            filePaths.push_back(path_);
        }
    }
    for (int i = 0; i < filePaths.size(); ++i)
    {
        ui->statusBar->showMessage("Обработка файла " + QString::number(i + 1)
            + " из " + QString::number(filePaths.size()));
        QString path = filePaths[i];
        if (path.isEmpty()) continue; // TO DO: add message box with errors
        QFile inputFile(path);
        QFileInfo fileInfo(inputFile);
        if (!inputFile.open(QFile::ReadOnly | QFile::Text)) continue; // TO DO same

        QString str = QTextStream(&inputFile).readAll();

        QString outputFilePath = ui->rewriteCheckBox->isChecked() ?
            outputDirPath.filePath(fileInfo.fileName()) :
            getUniqueFileName(outputDirPath.filePath(fileInfo.fileName()));
        QFile outputFile(outputFilePath);
        QFileInfo outputFileInfo(outputFile);

        outputFile.open(QFile::ReadWrite | QFile::Text); // TO DO: add check
        QTextStream outputFileContent(&outputFile);
        uint64_t mask = (ui->maskLineEdit->text()).toULongLong();
        QString strWithMask = applyMaskToFileContent(str, mask);
        outputFileContent << strWithMask;
        inputFile.close();
        outputFile.close();
    }
    if (ui->deleteInputFilesCheckBox->isChecked())
    {
        for (QString& path : filePaths)
        {
            // files that don't contain the name mask are not deleted
            QFile(path).remove(); // TO DO: add error handling
        }
        int sz = inputFilesTable->rowCount();
        for (int i = 0; i <= sz; ++i)
        {
            inputFilesTable->removeRow(0);
        }
    }
    ui->statusBar->showMessage("");
}

void MainWindow::onChangeOutputDirButtonClicked()
{
    QString outputDirPath = QFileDialog::getExistingDirectory(this,
        "Выберите директорию для выходных файлов", "C:/");
    if (!outputDirPath.isEmpty()) ui->outputDirLineEdit->setText(outputDirPath);
}

void MainWindow::enableControls(bool state)
{
    ui->settingsGroupBox->setEnabled(state);
    ui->startButton->setEnabled(state);
}

QByteArray MainWindow::applyMaskToFileContent(QString& str, uint64_t mask)
{
    uint8_t maskArray[8];
    for (int i = 0; i < 8; ++i)
    {
        maskArray[i] = mask >> (8 * (8 - i - 1));
    }
    QByteArray res = str.toUtf8();
    for (int i = 0, maskIndex = 0; i < res.size(); ++i, (++maskIndex) %= 8)
    {
        uint8_t element = res[i];
        element ^= maskArray[maskIndex];
        res[i] = element;
    }
    return res;
}

QString MainWindow::getUniqueFileName(const QString& newFilePath)
{
    QFileInfo newFileInfo(newFilePath);
    if (newFileInfo.exists())
    {
        for (int i = 2; ; ++i)
        {
            QString baseName_ = newFileInfo.baseName();
            baseName_ += "(" + QString::number(i) + ").";
            baseName_ += newFileInfo.completeSuffix();
            QString absoluteNewFilePath = newFileInfo.dir().filePath(baseName_);
            QFileInfo newFileInfo(absoluteNewFilePath);
            if (!newFileInfo.exists()) return newFileInfo.filePath();
        }
    }
    return newFilePath;
}

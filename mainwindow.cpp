#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget* parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , inputFilesTable(new InputFilesTable)
        , timerId(0)
{
    ui->setupUi(this);
    QBoxLayout* layout = new QBoxLayout(QBoxLayout::LeftToRight);
    layout->addWidget(inputFilesTable);
    ui->fileTableWidget->setLayout(layout);
    QObject::connect(ui->addFileButton, &QPushButton::clicked,
        inputFilesTable, &InputFilesTable::addFile);
    QObject::connect(ui->changeOutputDirButton, &QPushButton::clicked,
        this, &MainWindow::onChangeOutputDirButtonClicked);
    QObject::connect(ui->startButton, &QPushButton::clicked,
        this, &MainWindow::onStartButtonClicked);
    QObject::connect(ui->timerCheckBox, &QCheckBox::stateChanged,
        this, &MainWindow::onTimerCheckBoxStateChanged);
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
    // if outputDirLineEdit is empty, files are saved in the directory with
    // application
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
        if (path.isEmpty()) continue;
        QFile inputFile(path);
        QFileInfo fileInfo(inputFile);
        if (!inputFile.open(QFile::ReadOnly | QFile::Text)) continue;

        QByteArray inputData(inputFile.size(), '\0');
        QDataStream inputDataStream(&inputFile);
        inputDataStream.readRawData(inputData.data(), inputData.size());

        uint64_t mask = (ui->maskLineEdit->text()).toULongLong();
        applyMaskToFileContent(inputData, mask);

        QString outputFilePath = ui->rewriteCheckBox->isChecked() ?
            outputDirPath.filePath(fileInfo.fileName()) :
            getUniqueFileName(outputDirPath.filePath(fileInfo.fileName()));
        QFile outputFile(outputFilePath);
        QFileInfo outputFileInfo(outputFile);

        if (!outputFile.open(QFile::ReadWrite | QFile::Text))
        {
            inputFile.close();
            continue;
        }
        QDataStream outputDataStream(&outputFile);
        outputDataStream.writeRawData(inputData.data(), inputData.size());
        inputFile.close();
        outputFile.close();
    }
    if (ui->deleteInputFilesCheckBox->isChecked())
    {
        for (QString& path : filePaths)
        {
            // files that don't contain the name mask are not deleted
            QFile(path).remove();
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

void MainWindow::onTimerCheckBoxStateChanged(int state)
{
    if (state)
    {
        int period = ui->timerPeriodLineEdit->text().toInt();
        if (period <= 0 || period > INT_MAX / 1000)
        {
            QMessageBox::critical(this, "Ошибка",
                "Некорректное значение периода таймера");
            ui->timerCheckBox->setCheckState(Qt::Unchecked);
            return;
        }
        ui->startButton->setEnabled(false);
        ui->timerPeriodLineEdit->setEnabled(false);
        timerId = startTimer(period * 1000);
        timerEventContent();
    } else {
        ui->startButton->setEnabled(true);
        ui->timerPeriodLineEdit->setEnabled(true);
        killTimer(timerId);
    }
}

void MainWindow::enableControls(bool state)
{
    ui->settingsGroupBox->setEnabled(state);
    ui->startButton->setEnabled(state ?
        !ui->timerCheckBox->isChecked() : false);
    ui->addFileButton->setEnabled(state);
}

void MainWindow::applyMaskToFileContent(QByteArray& fileContent, uint64_t mask)
{
    // using local variable mask as an array of 8 uint8_t
    uint8_t* maskArray = reinterpret_cast<uint8_t*>(&mask);
    for (int i = 0, maskIndex = 0; i < fileContent.size();
        ++i, ++maskIndex %= 8)
    {
        // working with QByteArray's buffer, because QByteRef has no operator^=
        reinterpret_cast<uint8_t&>(fileContent.data()[i]) ^=
            maskArray[maskIndex];
    }
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

void MainWindow::timerEvent(QTimerEvent*) { timerEventContent(); }

void MainWindow::timerEventContent()
{
    if (!ui->settingsGroupBox->isEnabled()) return;
    workingProcessAsync();
}

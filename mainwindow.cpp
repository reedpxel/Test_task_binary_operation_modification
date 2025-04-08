#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    inputFilesTable = new InputFilesTable(ui->fileTableWidget);
    QObject::connect(ui->addFileButton, &QPushButton::clicked,
        inputFilesTable, &InputFilesTable::addFile);
    QObject::connect(ui->changeOutputDirButton, &QPushButton::clicked,
        this, &MainWindow::onChangeOutputDirButtonClicked);
    QObject::connect(ui->startButton, &QPushButton::clicked,
        this, &MainWindow::onStartButtonClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onStartButtonClicked()
{
    QVector<QString> filePaths;
    for (int i = 0; i < inputFilesTable->rowCount(); ++i)
    {
        filePaths.push_back(inputFilesTable->item(i, 0)->text());
    }
    QString outputDirPath = ui->outputDirLineEdit->text();
    for (QString& path : filePaths)
    {
        if (path.isEmpty()) continue; // TO DO: add message box with errors
        QFile inputFile(path);
        if (!inputFile.open(QFile::ReadOnly | QFile::Text)) continue; // TO DO same
        // QTextStream inputFileContent(&inputFile);
        QFileInfo fileInfo(inputFile);
        QString str = QTextStream(&inputFile).readAll();
        qDebug() << str.size() << str; // TO DO: remove
        QFile outputFile(outputDirPath + "\\" + fileInfo.fileName());
        outputFile.open(QFile::ReadWrite | QFile::Text); // TO DO: add check
        QTextStream outputFileContent(&outputFile);
        uint64_t mask = (ui->maskLineEdit->text()).toULongLong();
        qDebug() << "mask is" << mask;
        QString strWithMask = applyMask(str, mask);
        qDebug() << strWithMask.size() << strWithMask;
        outputFileContent << strWithMask;
//        if (ui->deleteInputFileRadioButton->isChecked())
//        {
//            if (!inputFile.remove())
//            {
//                QMessageBox::critical(this, "Oшибка",
//                    "Ошибка удаления входного файла");
//            }
//        } else {
            inputFile.close();
//        }
        outputFile.close();
    }
}

void MainWindow::onChangeOutputDirButtonClicked()
{
    QString outputDirPath = QFileDialog::getExistingDirectory(this,
        "Выберите директорию для выходных файлов", "C:/");
    if (!outputDirPath.isEmpty()) ui->outputDirLineEdit->setText(outputDirPath);
}

//void MainWindow::onInputFileButtonClicked()
//{
//    ui->inputFileLineEdit->setText(QFileDialog::getOpenFileName(this,
//        "Выберите входной файл", "C:/"));
//}

//void MainWindow::onOutputFileButtonClicked()
//{
//    ui->outputFileLineEdit->setText(QFileDialog::getSaveFileName(this,
//        "Выберите путь для выходного файл", "C:/"));
//}

//void MainWindow::onStartButtonClicked()
//{
//    QString inputFilePath = ui->inputFileLineEdit->text();
//    if (inputFilePath.isEmpty())
//    {
//        QMessageBox::critical(this, "Oшибка", "Путь к входному файлу пуст");
//        return;
//    }
//    QString outputFilePath = ui->outputFileLineEdit->text();
//    if (outputFilePath.isEmpty())
//    {
//        QMessageBox::critical(this, "Ошибка", "Путь к выходному файлу пуст");
//        return;
//    }
//    QFile inputFile(inputFilePath);
//    if (!inputFile.open(QFile::ReadOnly | QFile::Text))
//    {
//        QMessageBox::critical(this, "Ошибка",
//            "Не удается открыть файл по пути " + inputFilePath);
//        return;
//    }
//    QFile outputFile(outputFilePath);
//    if (!outputFile.open(QFile::ReadWrite))
//    {
//        QMessageBox::critical(this, "Ошибка",
//            "Не удается открыть файл по пути " + outputFilePath);
//        return;
//    }
//    // QTextStream inputFileContent(&inputFile);
//    QString str = QTextStream(&inputFile).readAll();
//    qDebug() << str.size() << str; // TO DO: remove
//    QTextStream outputFileContent(&outputFile);
//    uint64_t mask = (ui->maskLineEdit->text()).toULongLong();
//    qDebug() << "mask is" << mask;
//    QString strWithMask = applyMask(str, mask);
//    qDebug() << strWithMask.size() << strWithMask;
//    outputFileContent << strWithMask;
//    if (ui->deleteInputFileRadioButton->isChecked())
//    {
//        if (!inputFile.remove())
//        {
//            QMessageBox::critical(this, "Oшибка",
//                "Ошибка удаления входного файла");
//        }
//    } else {
//        inputFile.close();
//    }
//    outputFile.close();
//}

QByteArray MainWindow::applyMask(QString& str, uint64_t mask)
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

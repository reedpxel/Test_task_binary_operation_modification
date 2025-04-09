#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream> // TO DO: remove
#include <QDebug> // TO DO: remove
#include <QDataStream> // mb remove
#include <QTextStream> // mb remove
#include <QtConcurrent/QtConcurrent>

#include "inputfilestable.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    void filesProcessing();
    void workingProcessAsync();

signals:
    void unlock();

private slots:
    void onStartButtonClicked();
    void onChangeOutputDirButtonClicked();
    void enableControls(bool state);

private:
    QByteArray applyMaskToFileContent(QString& str, uint64_t mask);
    QString getUniqueFileName(const QString& newFilePath);

    Ui::MainWindow* ui;
    InputFilesTable* inputFilesTable;
};

#endif // MAINWINDOW_H

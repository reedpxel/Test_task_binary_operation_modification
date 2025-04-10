#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QDataStream>
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
    ~MainWindow() override;
    void filesProcessing();
    void workingProcessAsync();

signals:
    void unlock();

private slots:
    void onStartButtonClicked();
    void onChangeOutputDirButtonClicked();
    void onTimerCheckBoxStateChanged(int state);
    void enableControls(bool state);

private:
    void applyMaskToFileContent(QByteArray& fileContent, uint64_t mask);
    QString getUniqueFileName(const QString& newFilePath);
    void timerEvent(QTimerEvent* evt) override;
    void timerEventContent();

    Ui::MainWindow* ui;
    InputFilesTable* inputFilesTable;
    int timerId;
};

#endif // MAINWINDOW_H

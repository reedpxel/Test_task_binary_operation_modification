#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "inputfilestable.h"

#include <QMainWindow>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <iostream> // TO DO: remove
#include <QDebug> // TO DO: remove
#include <QDataStream> // mb remove
#include <QTextStream> // mb remove

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onStartButtonClicked();
    void onChangeOutputDirButtonClicked();

private:
    QByteArray applyMask(QString& str, uint64_t mask);
    Ui::MainWindow* ui;
    InputFilesTable* inputFilesTable;
};

#endif // MAINWINDOW_H

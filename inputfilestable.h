#ifndef INPUTFILESTABLE_H
#define INPUTFILESTABLE_H

#include <QtWidgets>
#include <QFileDialog>

class InputFilesTable : public QTableWidget
{
Q_OBJECT
public:
    InputFilesTable(QWidget* parent = nullptr);
public slots:
    void addFile();
};

#endif // INPUTFILESTABLE_H

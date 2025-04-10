#include "inputfilestable.h"

InputFilesTable::InputFilesTable(QWidget* parent)
        : QTableWidget(0, 2, parent)
{
    horizontalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    setFocusPolicy(Qt::NoFocus);
}

void InputFilesTable::addFile()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        "Выберите входной файл", "C:/");
    if (filePath.isEmpty()) return;
    insertRow(rowCount());
    setItem(rowCount() - 1, 0, new QTableWidgetItem(filePath));
    QTableWidgetItem* buttonItem = new QTableWidgetItem;
    QPushButton* button = new QPushButton("Удалить");
    QObject::connect(button, &QPushButton::clicked, this,
        [this, buttonItem]() { removeRow(buttonItem->row()); });
    setItem(rowCount() - 1, 1, buttonItem);
    setCellWidget(rowCount() - 1, 1, button);
}

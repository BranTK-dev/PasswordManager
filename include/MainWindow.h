#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "Credential.h"

class QTableWidget;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSelectionChanged();

private:
    void setupUi();
    void refreshTable();
    int selectedRow() const;
    int nextId();

    QTableWidget *m_table;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;

    // In-memory store for now, this becomes DatabaseManager-backed
    // persistence in Phase 5.
    QVector<Credential> m_credentials;
    int m_nextId;
};

#endif // MAINWINDOW_H

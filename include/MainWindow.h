#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "Credential.h"
#include "DatabaseManager.h"

class QTableWidget;
class QPushButton;
class QLabel;

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
    QString databasePath() const;

    QTableWidget *m_table;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QLabel *m_statusLabel;

    // Cached copy of what's in the database, kept in sync after every
    // add/edit/delete so the table can redraw without hitting the DB
    // each time. DatabaseManager is the actual source of truth.
    QVector<Credential> m_credentials;
    DatabaseManager m_db;
};

#endif // MAINWINDOW_H

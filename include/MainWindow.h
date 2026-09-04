#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include "Credential.h"
#include "DatabaseManager.h"
#include "EncryptionManager.h"

class QTableWidget;
class QPushButton;
class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(EncryptionManager &encryptionManager, QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();
    void onSelectionChanged();
    void onFilterChanged();
    void onCopyUsernameClicked();
    void onCopyPasswordClicked();
    void onTogglePasswordVisibility();
    void onSettingsClicked();
    void onAboutClicked();
    void onDarkModeToggled(bool enabled);

private:
    void setupUi();
    void refreshTable();
    void refreshCategoryFilterOptions();
    int selectedRow() const;
    QString databasePath() const;
    // Maps a table row to the credential's real database id (rows
    // reflect the filtered list, not m_credentials' order/index).
    int credentialIdForRow(int row) const;
    int indexForCredentialId(int id) const;

    // Returns the subset of m_credentials that matches the current
    // search text, category filter, and favorites-only toggle.
    QVector<Credential> filteredCredentials() const;

    QTableWidget *m_table;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_copyUsernameButton;
    QPushButton *m_copyPasswordButton;
    QPushButton *m_togglePasswordsButton;
    QLabel *m_statusLabel;
    bool m_passwordsVisible;
    QAction *m_darkModeAction;

    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilterCombo;
    QCheckBox *m_favoritesOnlyCheck;

    // Cached copy of everything in the database. filteredCredentials()
    // derives the visible subset from this; it's what the table
    // actually displays. Row indices in the table always refer to
    // the filtered list, not m_credentials directly.
    QVector<Credential> m_credentials;
    DatabaseManager m_db;
    EncryptionManager &m_encryption;
};

#endif // MAINWINDOW_H

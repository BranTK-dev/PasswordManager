#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;

// Lets the user change their master password. Verification of the
// current password and the actual re-encryption work happens in
// MainWindow (it owns DatabaseManager and EncryptionManager); this
// dialog only collects the three password fields and validates them
// client-side (non-empty, new == confirm, minimum length).
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    QString currentPassword() const;
    QString newPassword() const;

private slots:
    void onSaveClicked();

private:
    void setupUi();

    QLineEdit *m_currentPasswordEdit;
    QLineEdit *m_newPasswordEdit;
    QLineEdit *m_confirmPasswordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_saveButton;
};

#endif // SETTINGSDIALOG_H

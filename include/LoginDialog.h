#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

class QLineEdit;
class QLabel;
class QPushButton;

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    enum class Mode {
        Login,       // vault already has a master password, verify it
        SetupNew     // first run, create a new master password
    };

    explicit LoginDialog(Mode mode, QWidget *parent = nullptr);

    // The password the user typed, valid only after accept()
    QString password() const;

private slots:
    void onUnlockClicked();
    void togglePasswordVisibility();

private:
    void setupUi();

    Mode m_mode;
    QLineEdit *m_passwordEdit;
    QLineEdit *m_confirmPasswordEdit; // only used in SetupNew mode
    QLabel *m_confirmLabel;
    QLabel *m_errorLabel;
    QPushButton *m_unlockButton;
    QPushButton *m_toggleVisibilityButton;
    QString m_password;
};

#endif // LOGINDIALOG_H

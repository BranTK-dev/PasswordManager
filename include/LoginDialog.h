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
    explicit LoginDialog(QWidget *parent = nullptr);

    // The password the user typed, valid only after accept()
    QString password() const;

private slots:
    void onUnlockClicked();
    void togglePasswordVisibility();

private:
    void setupUi();

    QLineEdit *m_passwordEdit;
    QLabel *m_errorLabel;
    QPushButton *m_unlockButton;
    QPushButton *m_toggleVisibilityButton;
    QString m_password;
};

#endif // LOGINDIALOG_H

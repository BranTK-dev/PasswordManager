#ifndef PASSWORDGENERATORDIALOG_H
#define PASSWORDGENERATORDIALOG_H

#include <QDialog>

class QSlider;
class QLabel;
class QCheckBox;
class QLineEdit;
class QPushButton;

// Standalone dialog for generating a strong random password. Can be
// opened from the dashboard (just to generate/copy one) or from
// CredentialDialog (to fill the password field directly).
class PasswordGeneratorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PasswordGeneratorDialog(QWidget *parent = nullptr);

    // The generated password, valid after the dialog is accepted
    // (i.e. the user clicked "Use This Password").
    QString generatedPassword() const;

private slots:
    void regenerate();
    void onLengthChanged(int value);
    void onOptionsChanged();
    void copyToClipboard();

private:
    void setupUi();
    QString buildCharacterPool() const;

    QSlider *m_lengthSlider;
    QLabel *m_lengthValueLabel;
    QCheckBox *m_uppercaseCheck;
    QCheckBox *m_lowercaseCheck;
    QCheckBox *m_numbersCheck;
    QCheckBox *m_symbolsCheck;
    QLineEdit *m_resultEdit;
    QLabel *m_strengthLabel;
    QPushButton *m_useButton;

    QString m_password;
};

#endif // PASSWORDGENERATORDIALOG_H

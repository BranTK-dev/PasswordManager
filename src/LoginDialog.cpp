#include "LoginDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

LoginDialog::LoginDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
    , m_mode(mode)
{
    setupUi();
}

void LoginDialog::setupUi()
{
    const bool isSetup = (m_mode == Mode::SetupNew);

    setWindowTitle(isSetup ? "Password Manager - Create Master Password"
                            : "Password Manager - Login");
    setFixedSize(360, isSetup ? 280 : 220);

    auto *titleLabel = new QLabel("Password Manager", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *subtitleLabel = new QLabel(
        isSetup ? "Create a master password to protect your vault. "
                  "This cannot be recovered if you forget it."
                : "Enter your master password to continue",
        this);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Master password");

    m_toggleVisibilityButton = new QPushButton("Show", this);
    m_toggleVisibilityButton->setFixedWidth(60);
    connect(m_toggleVisibilityButton, &QPushButton::clicked,
            this, &LoginDialog::togglePasswordVisibility);

    auto *passwordRow = new QHBoxLayout();
    passwordRow->addWidget(m_passwordEdit);
    passwordRow->addWidget(m_toggleVisibilityButton);

    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    m_confirmPasswordEdit->setPlaceholderText("Confirm master password");
    m_confirmLabel = new QLabel("Confirm:", this);
    m_confirmPasswordEdit->setVisible(isSetup);
    m_confirmLabel->setVisible(isSetup);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);

    m_unlockButton = new QPushButton(isSetup ? "Create Vault" : "Unlock", this);
    m_unlockButton->setDefault(true);
    connect(m_unlockButton, &QPushButton::clicked,
            this, &LoginDialog::onUnlockClicked);

    connect(m_passwordEdit, &QLineEdit::returnPressed,
            this, &LoginDialog::onUnlockClicked);
    connect(m_confirmPasswordEdit, &QLineEdit::returnPressed,
            this, &LoginDialog::onUnlockClicked);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(passwordRow);
    if (isSetup) {
        mainLayout->addWidget(m_confirmLabel);
        mainLayout->addWidget(m_confirmPasswordEdit);
    }
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addWidget(m_unlockButton);
    mainLayout->addStretch();

    setLayout(mainLayout);
    m_passwordEdit->setFocus();
}

void LoginDialog::togglePasswordVisibility()
{
    const auto mode = (m_passwordEdit->echoMode() == QLineEdit::Password)
                           ? QLineEdit::Normal : QLineEdit::Password;
    m_passwordEdit->setEchoMode(mode);
    m_confirmPasswordEdit->setEchoMode(mode);
    m_toggleVisibilityButton->setText(mode == QLineEdit::Normal ? "Hide" : "Show");
}

void LoginDialog::onUnlockClicked()
{
    const QString entered = m_passwordEdit->text();

    if (entered.isEmpty()) {
        m_errorLabel->setText("Please enter your master password.");
        m_errorLabel->setVisible(true);
        return;
    }

    if (m_mode == Mode::SetupNew) {
        if (entered.length() < 8) {
            m_errorLabel->setText("Master password must be at least 8 characters.");
            m_errorLabel->setVisible(true);
            return;
        }

        if (entered != m_confirmPasswordEdit->text()) {
            m_errorLabel->setText("Passwords do not match.");
            m_errorLabel->setVisible(true);
            return;
        }
    }

    // Actual verification against the stored hash (Login mode) happens
    // in main.cpp after this dialog is accepted, since that's where
    // EncryptionManager and DatabaseManager live. If verification
    // fails there, main.cpp re-shows this dialog with an error.
    m_password = entered;
    accept();
}

QString LoginDialog::password() const
{
    return m_password;
}

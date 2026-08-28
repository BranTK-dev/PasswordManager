#include "LoginDialog.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void LoginDialog::setupUi()
{
    setWindowTitle("Password Manager - Login");
    setFixedSize(360, 220);

    auto *titleLabel = new QLabel("Password Manager", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *subtitleLabel = new QLabel("Enter your master password to continue", this);
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

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setAlignment(Qt::AlignCenter);
    m_errorLabel->setVisible(false);

    m_unlockButton = new QPushButton("Unlock", this);
    m_unlockButton->setDefault(true);
    connect(m_unlockButton, &QPushButton::clicked,
            this, &LoginDialog::onUnlockClicked);

    connect(m_passwordEdit, &QLineEdit::returnPressed,
            this, &LoginDialog::onUnlockClicked);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(subtitleLabel);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(passwordRow);
    mainLayout->addWidget(m_errorLabel);
    mainLayout->addWidget(m_unlockButton);
    mainLayout->addStretch();

    setLayout(mainLayout);
    m_passwordEdit->setFocus();
}

void LoginDialog::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_toggleVisibilityButton->setText("Hide");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_toggleVisibilityButton->setText("Show");
    }
}

void LoginDialog::onUnlockClicked()
{
    const QString entered = m_passwordEdit->text();

    if (entered.isEmpty()) {
        m_errorLabel->setText("Please enter your master password.");
        m_errorLabel->setVisible(true);
        return;
    }

    m_password = entered;
    accept();
}

QString LoginDialog::password() const
{
    return m_password;
}

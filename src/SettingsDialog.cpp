#include "SettingsDialog.h"

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
}

void SettingsDialog::setupUi()
{
    setWindowTitle("Settings");
    setMinimumWidth(360);

    auto *sectionLabel = new QLabel("Change Master Password", this);
    QFont sectionFont = sectionLabel->font();
    sectionFont.setBold(true);
    sectionLabel->setFont(sectionFont);

    m_currentPasswordEdit = new QLineEdit(this);
    m_currentPasswordEdit->setEchoMode(QLineEdit::Password);

    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);

    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setStyleSheet("color: red;");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);

    auto *form = new QFormLayout();
    form->addRow(sectionLabel);
    form->addRow("Current password:", m_currentPasswordEdit);
    form->addRow("New password:", m_newPasswordEdit);
    form->addRow("Confirm new password:", m_confirmPasswordEdit);
    form->addRow(m_errorLabel);

    m_saveButton = new QPushButton("Change Password", this);
    m_saveButton->setDefault(true);
    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);

    auto *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(m_saveButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttonRow);
    setLayout(mainLayout);

    m_currentPasswordEdit->setFocus();
}

void SettingsDialog::onSaveClicked()
{
    if (m_currentPasswordEdit->text().isEmpty() ||
        m_newPasswordEdit->text().isEmpty() ||
        m_confirmPasswordEdit->text().isEmpty()) {
        m_errorLabel->setText("Please fill in all three fields.");
        m_errorLabel->setVisible(true);
        return;
    }

    if (m_newPasswordEdit->text().length() < 8) {
        m_errorLabel->setText("New password must be at least 8 characters.");
        m_errorLabel->setVisible(true);
        return;
    }

    if (m_newPasswordEdit->text() != m_confirmPasswordEdit->text()) {
        m_errorLabel->setText("New passwords do not match.");
        m_errorLabel->setVisible(true);
        return;
    }

    // Actual current-password verification happens in MainWindow,
    // since it has access to the stored salt/hash. If that check
    // fails, MainWindow shows its own error and this dialog is
    // simply reopened by the caller, not retried internally.
    accept();
}

QString SettingsDialog::currentPassword() const
{
    return m_currentPasswordEdit->text();
}

QString SettingsDialog::newPassword() const
{
    return m_newPasswordEdit->text();
}

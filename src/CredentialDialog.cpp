#include "CredentialDialog.h"
#include "PasswordGeneratorDialog.h"

#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QClipboard>
#include <QGuiApplication>

CredentialDialog::CredentialDialog(QWidget *parent)
    : QDialog(parent)
    , m_isEditMode(false)
{
    setupUi();
}

CredentialDialog::CredentialDialog(const Credential &existing, QWidget *parent)
    : QDialog(parent)
    , m_credential(existing)
    , m_isEditMode(true)
{
    setupUi();
    populateFromCredential(existing);
}

void CredentialDialog::setupUi()
{
    setWindowTitle(m_isEditMode ? "Edit Credential" : "Add Credential");
    setMinimumWidth(360);

    m_websiteEdit = new QLineEdit(this);
    m_urlEdit = new QLineEdit(this);
    m_usernameEdit = new QLineEdit(this);
    m_emailEdit = new QLineEdit(this);

    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_toggleVisibilityButton = new QPushButton("Show", this);
    m_toggleVisibilityButton->setFixedWidth(60);
    connect(m_toggleVisibilityButton, &QPushButton::clicked,
            this, &CredentialDialog::togglePasswordVisibility);

    m_generateButton = new QPushButton("Generate", this);
    connect(m_generateButton, &QPushButton::clicked,
            this, &CredentialDialog::onGeneratePasswordClicked);

    auto *copyPasswordButton = new QPushButton("Copy", this);
    connect(copyPasswordButton, &QPushButton::clicked, this, [this]() {
        if (!m_passwordEdit->text().isEmpty()) {
            QGuiApplication::clipboard()->setText(m_passwordEdit->text());
        }
    });

    auto *passwordRow = new QHBoxLayout();
    passwordRow->addWidget(m_passwordEdit);
    passwordRow->addWidget(m_toggleVisibilityButton);
    passwordRow->addWidget(copyPasswordButton);
    passwordRow->addWidget(m_generateButton);

    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setEditable(true);
    m_categoryCombo->addItems({"General", "Work", "Personal", "Finance", "Social", "Development"});

    m_notesEdit = new QTextEdit(this);
    m_notesEdit->setFixedHeight(80);

    m_favoriteCheck = new QCheckBox("Mark as favorite", this);

    auto *form = new QFormLayout();
    form->addRow("Website/App:", m_websiteEdit);
    form->addRow("URL:", m_urlEdit);
    form->addRow("Username:", m_usernameEdit);
    form->addRow("Email:", m_emailEdit);
    form->addRow("Password:", passwordRow);
    form->addRow("Category:", m_categoryCombo);
    form->addRow("Notes:", m_notesEdit);
    form->addRow("", m_favoriteCheck);

    m_saveButton = new QPushButton(m_isEditMode ? "Save Changes" : "Add Credential", this);
    m_saveButton->setDefault(true);
    connect(m_saveButton, &QPushButton::clicked, this, &CredentialDialog::onSaveClicked);

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
    m_websiteEdit->setFocus();
}

void CredentialDialog::populateFromCredential(const Credential &existing)
{
    m_websiteEdit->setText(existing.website());
    m_urlEdit->setText(existing.url());
    m_usernameEdit->setText(existing.username());
    m_emailEdit->setText(existing.email());
    m_passwordEdit->setText(existing.password());
    m_categoryCombo->setCurrentText(existing.category());
    m_notesEdit->setPlainText(existing.notes());
    m_favoriteCheck->setChecked(existing.isFavorite());
}

void CredentialDialog::togglePasswordVisibility()
{
    if (m_passwordEdit->echoMode() == QLineEdit::Password) {
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_toggleVisibilityButton->setText("Hide");
    } else {
        m_passwordEdit->setEchoMode(QLineEdit::Password);
        m_toggleVisibilityButton->setText("Show");
    }
}

void CredentialDialog::onSaveClicked()
{
    if (m_websiteEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                              "Please enter a website or app name.");
        return;
    }

    if (m_usernameEdit->text().trimmed().isEmpty() && m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information",
                              "Please enter a username or email.");
        return;
    }

    m_credential.setWebsite(m_websiteEdit->text().trimmed());
    m_credential.setUrl(m_urlEdit->text().trimmed());
    m_credential.setUsername(m_usernameEdit->text().trimmed());
    m_credential.setEmail(m_emailEdit->text().trimmed());
    m_credential.setPassword(m_passwordEdit->text());
    m_credential.setCategory(m_categoryCombo->currentText().trimmed());
    m_credential.setNotes(m_notesEdit->toPlainText());
    m_credential.setFavorite(m_favoriteCheck->isChecked());
    m_credential.touch();

    accept();
}

void CredentialDialog::onGeneratePasswordClicked()
{
    PasswordGeneratorDialog generatorDialog(this);
    if (generatorDialog.exec() == QDialog::Accepted) {
        m_passwordEdit->setText(generatorDialog.generatedPassword());
        // Reveal the generated password briefly so the user can see
        // what got filled in, rather than leaving it masked.
        m_passwordEdit->setEchoMode(QLineEdit::Normal);
        m_toggleVisibilityButton->setText("Hide");
    }
}

Credential CredentialDialog::credential() const
{
    return m_credential;
}

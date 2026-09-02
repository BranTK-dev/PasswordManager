#include "PasswordGeneratorDialog.h"

#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>

#include <openssl/rand.h>

namespace {
const QString kUppercase = "ABCDEFGHJKLMNPQRSTUVWXYZ"; // no I/O to avoid look-alikes
const QString kLowercase = "abcdefghijkmnopqrstuvwxyz"; // no l
const QString kNumbers = "23456789"; // no 0/1
const QString kSymbols = "!@#$%^&*()-_=+[]{}?";
}

PasswordGeneratorDialog::PasswordGeneratorDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    regenerate();
}

void PasswordGeneratorDialog::setupUi()
{
    setWindowTitle("Password Generator");
    setMinimumWidth(380);

    m_resultEdit = new QLineEdit(this);
    m_resultEdit->setReadOnly(true);
    QFont mono = m_resultEdit->font();
    mono.setFamily("monospace");
    mono.setPointSize(mono.pointSize() + 1);
    m_resultEdit->setFont(mono);

    auto *copyButton = new QPushButton("Copy", this);
    connect(copyButton, &QPushButton::clicked, this, &PasswordGeneratorDialog::copyToClipboard);

    auto *regenButton = new QPushButton("Regenerate", this);
    connect(regenButton, &QPushButton::clicked, this, &PasswordGeneratorDialog::regenerate);

    auto *resultRow = new QHBoxLayout();
    resultRow->addWidget(m_resultEdit, 1);
    resultRow->addWidget(copyButton);
    resultRow->addWidget(regenButton);

    m_strengthLabel = new QLabel(this);

    m_lengthSlider = new QSlider(Qt::Horizontal, this);
    m_lengthSlider->setMinimum(8);
    m_lengthSlider->setMaximum(64);
    m_lengthSlider->setValue(16);
    connect(m_lengthSlider, &QSlider::valueChanged, this, &PasswordGeneratorDialog::onLengthChanged);

    m_lengthValueLabel = new QLabel("16", this);
    m_lengthValueLabel->setFixedWidth(30);

    auto *lengthRow = new QHBoxLayout();
    lengthRow->addWidget(m_lengthSlider);
    lengthRow->addWidget(m_lengthValueLabel);

    m_uppercaseCheck = new QCheckBox("Uppercase (A-Z)", this);
    m_uppercaseCheck->setChecked(true);
    m_lowercaseCheck = new QCheckBox("Lowercase (a-z)", this);
    m_lowercaseCheck->setChecked(true);
    m_numbersCheck = new QCheckBox("Numbers (0-9)", this);
    m_numbersCheck->setChecked(true);
    m_symbolsCheck = new QCheckBox("Symbols (!@#...)", this);
    m_symbolsCheck->setChecked(true);

    connect(m_uppercaseCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::onOptionsChanged);
    connect(m_lowercaseCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::onOptionsChanged);
    connect(m_numbersCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::onOptionsChanged);
    connect(m_symbolsCheck, &QCheckBox::toggled, this, &PasswordGeneratorDialog::onOptionsChanged);

    auto *form = new QFormLayout();
    form->addRow("Password:", resultRow);
    form->addRow("Strength:", m_strengthLabel);
    form->addRow("Length:", lengthRow);
    form->addRow(m_uppercaseCheck);
    form->addRow(m_lowercaseCheck);
    form->addRow(m_numbersCheck);
    form->addRow(m_symbolsCheck);

    m_useButton = new QPushButton("Use This Password", this);
    m_useButton->setDefault(true);
    connect(m_useButton, &QPushButton::clicked, this, &QDialog::accept);

    auto *cancelButton = new QPushButton("Cancel", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addStretch();
    buttonRow->addWidget(cancelButton);
    buttonRow->addWidget(m_useButton);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(form);
    mainLayout->addLayout(buttonRow);
    setLayout(mainLayout);
}

QString PasswordGeneratorDialog::buildCharacterPool() const
{
    QString pool;
    if (m_uppercaseCheck->isChecked()) pool += kUppercase;
    if (m_lowercaseCheck->isChecked()) pool += kLowercase;
    if (m_numbersCheck->isChecked()) pool += kNumbers;
    if (m_symbolsCheck->isChecked()) pool += kSymbols;
    return pool;
}

void PasswordGeneratorDialog::onLengthChanged(int value)
{
    m_lengthValueLabel->setText(QString::number(value));
    regenerate();
}

void PasswordGeneratorDialog::onOptionsChanged()
{
    // Guard against the user unchecking every category, which would
    // leave an empty character pool. Silently re-check "Lowercase"
    // rather than letting the generator produce nothing.
    if (buildCharacterPool().isEmpty()) {
        m_lowercaseCheck->blockSignals(true);
        m_lowercaseCheck->setChecked(true);
        m_lowercaseCheck->blockSignals(false);
    }
    regenerate();
}

void PasswordGeneratorDialog::regenerate()
{
    const QString pool = buildCharacterPool();
    const int length = m_lengthSlider->value();

    if (pool.isEmpty()) {
        m_resultEdit->clear();
        m_strengthLabel->setText("N/A");
        m_password.clear();
        return;
    }

    // Use OpenSSL's CSPRNG rather than qrand()/QRandomGenerator, this
    // is a security-relevant randomness source, same reasoning as
    // EncryptionManager's key/IV generation.
    QByteArray randomBytes(length, '\0');
    RAND_bytes(reinterpret_cast<unsigned char *>(randomBytes.data()), length);

    QString result;
    result.reserve(length);
    for (int i = 0; i < length; ++i) {
        // Map each random byte into the pool's index range. This has
        // a very slight modulo bias for pools that don't evenly
        // divide 256, acceptable here since pool sizes are small
        // relative to 256 and this is a password generator, not a
        // cryptographic primitive itself.
        const unsigned char byte = static_cast<unsigned char>(randomBytes[i]);
        result.append(pool.at(byte % pool.length()));
    }

    m_password = result;
    m_resultEdit->setText(m_password);

    // Rough strength label based on length + variety, purely a UX hint
    int varietyCount = 0;
    if (m_uppercaseCheck->isChecked()) varietyCount++;
    if (m_lowercaseCheck->isChecked()) varietyCount++;
    if (m_numbersCheck->isChecked()) varietyCount++;
    if (m_symbolsCheck->isChecked()) varietyCount++;

    QString strength;
    if (length >= 16 && varietyCount >= 3) {
        strength = "Strong";
    } else if (length >= 12 && varietyCount >= 2) {
        strength = "Good";
    } else {
        strength = "Weak";
    }
    m_strengthLabel->setText(strength);
}

void PasswordGeneratorDialog::copyToClipboard()
{
    if (m_password.isEmpty()) {
        return;
    }
    QGuiApplication::clipboard()->setText(m_password);
}

QString PasswordGeneratorDialog::generatedPassword() const
{
    return m_password;
}

#include "AboutDialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("About Password Manager");
    setFixedSize(340, 220);

    auto *titleLabel = new QLabel("Password Manager", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *versionLabel = new QLabel("Version 0.8 (Phase 8)", this);
    versionLabel->setAlignment(Qt::AlignCenter);

    auto *descriptionLabel = new QLabel(
        "A desktop password manager built with C++ and Qt6. "
        "Credentials are encrypted with AES-256-GCM, with keys "
        "derived from your master password via PBKDF2.",
        this);
    descriptionLabel->setAlignment(Qt::AlignCenter);
    descriptionLabel->setWordWrap(true);

    auto *stackLabel = new QLabel("Built with: C++17, Qt6, SQLite, OpenSSL", this);
    stackLabel->setAlignment(Qt::AlignCenter);
    stackLabel->setStyleSheet("color: gray;");

    auto *closeButton = new QPushButton("Close", this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);
    layout->addWidget(versionLabel);
    layout->addSpacing(10);
    layout->addWidget(descriptionLabel);
    layout->addStretch();
    layout->addWidget(stackLabel);
    layout->addWidget(closeButton);

    setLayout(layout);
}

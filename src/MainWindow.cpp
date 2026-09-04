#include "MainWindow.h"
#include "CredentialDialog.h"
#include "SettingsDialog.h"
#include "AboutDialog.h"
#include "Theme.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QSet>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QGuiApplication>
#include <algorithm>

namespace {
constexpr int ColWebsite = 0;
constexpr int ColUsername = 1;
constexpr int ColPassword = 2;
constexpr int ColCategory = 3;
constexpr int ColFavorite = 4;
constexpr int ColModified = 5;
const QString kAllCategories = QStringLiteral("All Categories");
const QString kMaskedPassword = QStringLiteral("••••••••");
}

MainWindow::MainWindow(EncryptionManager &encryptionManager, QWidget *parent)
    : QMainWindow(parent)
    , m_passwordsVisible(false)
    , m_encryption(encryptionManager)
{
    setupUi();

    if (!m_db.open(databasePath())) {
        QMessageBox::critical(this, "Database Error",
            "Could not open the credentials database:\n" + m_db.lastError());
    }

    // Now that the DB is open, sync the checkbox to the saved
    // preference without re-triggering onDarkModeToggled (which would
    // otherwise immediately re-write the same value back to disk).
    m_darkModeAction->blockSignals(true);
    m_darkModeAction->setChecked(m_db.isDarkModeEnabled());
    m_darkModeAction->blockSignals(false);

    m_credentials = m_db.loadAll();

    // Passwords are stored encrypted; decrypt each one into memory now
    // that we're unlocked. If a password fails to decrypt (corrupted
    // data or wrong key), leave it blank rather than showing garbage.
    for (Credential &c : m_credentials) {
        const QString decrypted = m_encryption.decrypt(c.password());
        c.setPassword(decrypted);
    }

    refreshCategoryFilterOptions();
    refreshTable();
}

MainWindow::~MainWindow()
{
    m_db.close();
}

QString MainWindow::databasePath() const
{
    QDir appDir(QCoreApplication::applicationDirPath());
    return appDir.filePath("../database/vault.db");
}

void MainWindow::setupUi()
{
    setWindowTitle("Password Manager");
    resize(1000, 620);

    // --- Menu bar ---
    auto *fileMenu = menuBar()->addMenu("&File");
    auto *settingsAction = fileMenu->addAction("Settings...");
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    fileMenu->addSeparator();
    auto *quitAction = fileMenu->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    auto *helpMenu = menuBar()->addMenu("&Help");
    auto *aboutAction = helpMenu->addAction("About");
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutClicked);

    auto *viewMenu = menuBar()->addMenu("&View");
    m_darkModeAction = viewMenu->addAction("Dark Mode");
    m_darkModeAction->setCheckable(true);
    connect(m_darkModeAction, &QAction::toggled, this, &MainWindow::onDarkModeToggled);
    // Actual checked state is set in the constructor, after m_db is
    // open, setupUi() runs before the database connection exists.

    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *titleLabel = new QLabel("Your Credentials", central);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // --- Search / filter row ---
    m_searchEdit = new QLineEdit(central);
    m_searchEdit->setPlaceholderText("Search by website or username...");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onFilterChanged);

    m_categoryFilterCombo = new QComboBox(central);
    m_categoryFilterCombo->addItem(kAllCategories);
    connect(m_categoryFilterCombo, &QComboBox::currentTextChanged, this, &MainWindow::onFilterChanged);

    m_favoritesOnlyCheck = new QCheckBox("Favorites only", central);
    connect(m_favoritesOnlyCheck, &QCheckBox::toggled, this, &MainWindow::onFilterChanged);

    m_togglePasswordsButton = new QPushButton("Show Passwords", central);
    m_togglePasswordsButton->setCheckable(true);
    connect(m_togglePasswordsButton, &QPushButton::clicked, this, &MainWindow::onTogglePasswordVisibility);

    auto *filterRow = new QHBoxLayout();
    filterRow->addWidget(m_searchEdit, 1);
    filterRow->addWidget(m_categoryFilterCombo);
    filterRow->addWidget(m_favoritesOnlyCheck);
    filterRow->addWidget(m_togglePasswordsButton);

    // --- Table ---
    m_table = new QTableWidget(0, 6, central);
    m_table->setHorizontalHeaderLabels({"Website/App", "Username", "Password", "Category", "Favorite", "Last Modified"});
    m_table->horizontalHeader()->setSectionResizeMode(ColWebsite, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(ColUsername, QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_table, &QTableWidget::itemSelectionChanged, this, &MainWindow::onSelectionChanged);
    connect(m_table, &QTableWidget::itemDoubleClicked, this, &MainWindow::onEditClicked);

    m_addButton = new QPushButton("Add", central);
    m_editButton = new QPushButton("Edit", central);
    m_deleteButton = new QPushButton("Delete", central);
    m_copyUsernameButton = new QPushButton("Copy Username", central);
    m_copyPasswordButton = new QPushButton("Copy Password", central);
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);
    m_copyUsernameButton->setEnabled(false);
    m_copyPasswordButton->setEnabled(false);

    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(m_copyUsernameButton, &QPushButton::clicked, this, &MainWindow::onCopyUsernameClicked);
    connect(m_copyPasswordButton, &QPushButton::clicked, this, &MainWindow::onCopyPasswordClicked);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(m_addButton);
    buttonRow->addWidget(m_editButton);
    buttonRow->addWidget(m_deleteButton);
    buttonRow->addWidget(m_copyUsernameButton);
    buttonRow->addWidget(m_copyPasswordButton);
    buttonRow->addStretch();

    m_statusLabel = new QLabel(central);
    m_statusLabel->setStyleSheet("color: gray;");

    layout->addWidget(titleLabel);
    layout->addLayout(filterRow);
    layout->addLayout(buttonRow);
    layout->addWidget(m_table);
    layout->addWidget(m_statusLabel);

    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::refreshCategoryFilterOptions()
{
    // Preserve whatever is currently selected so refreshing the list
    // (e.g. after adding a credential with a new category) doesn't
    // reset the user's filter back to "All Categories".
    const QString currentSelection = m_categoryFilterCombo->currentText();

    QSet<QString> categories;
    for (const Credential &c : m_credentials) {
        if (!c.category().trimmed().isEmpty()) {
            categories.insert(c.category().trimmed());
        }
    }

    QStringList sorted = categories.values();
    sorted.sort(Qt::CaseInsensitive);

    m_categoryFilterCombo->blockSignals(true);
    m_categoryFilterCombo->clear();
    m_categoryFilterCombo->addItem(kAllCategories);
    m_categoryFilterCombo->addItems(sorted);

    const int idx = m_categoryFilterCombo->findText(currentSelection);
    m_categoryFilterCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    m_categoryFilterCombo->blockSignals(false);
}

QVector<Credential> MainWindow::filteredCredentials() const
{
    const QString searchText = m_searchEdit->text().trimmed();
    const QString categoryFilter = m_categoryFilterCombo->currentText();
    const bool favoritesOnly = m_favoritesOnlyCheck->isChecked();

    QVector<Credential> result;
    for (const Credential &c : m_credentials) {
        if (favoritesOnly && !c.isFavorite()) {
            continue;
        }

        if (categoryFilter != kAllCategories && c.category() != categoryFilter) {
            continue;
        }

        if (!searchText.isEmpty()) {
            const bool matchesWebsite = c.website().contains(searchText, Qt::CaseInsensitive);
            const bool matchesUsername = c.username().contains(searchText, Qt::CaseInsensitive);
            const bool matchesEmail = c.email().contains(searchText, Qt::CaseInsensitive);
            if (!matchesWebsite && !matchesUsername && !matchesEmail) {
                continue;
            }
        }

        result.append(c);
    }

    return result;
}

void MainWindow::onFilterChanged()
{
    refreshTable();
}

void MainWindow::refreshTable()
{
    const QVector<Credential> visible = filteredCredentials();

    m_table->setRowCount(0);

    for (const Credential &c : visible) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, ColWebsite, new QTableWidgetItem(c.website()));
        m_table->setItem(row, ColUsername, new QTableWidgetItem(
            c.username().isEmpty() ? c.email() : c.username()));
        m_table->setItem(row, ColPassword, new QTableWidgetItem(
            m_passwordsVisible ? c.password() : kMaskedPassword));
        m_table->setItem(row, ColCategory, new QTableWidgetItem(c.category()));
        m_table->setItem(row, ColFavorite, new QTableWidgetItem(c.isFavorite() ? "Yes" : ""));
        m_table->setItem(row, ColModified, new QTableWidgetItem(
            c.dateModified().toString("yyyy-MM-dd hh:mm")));

        // Stash the credential's real id in the row so we can map
        // back to m_credentials without relying on filtered/unfiltered
        // index alignment.
        m_table->item(row, ColWebsite)->setData(Qt::UserRole, c.id());
    }

    if (visible.size() == m_credentials.size()) {
        m_statusLabel->setText(QString("%1 credential(s) saved").arg(m_credentials.size()));
    } else {
        m_statusLabel->setText(QString("Showing %1 of %2 credential(s)")
                                    .arg(visible.size()).arg(m_credentials.size()));
    }
}

int MainWindow::selectedRow() const
{
    const auto selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return -1;
    }
    return selected.first().row();
}

int MainWindow::credentialIdForRow(int row) const
{
    if (row < 0 || row >= m_table->rowCount()) {
        return -1;
    }
    return m_table->item(row, ColWebsite)->data(Qt::UserRole).toInt();
}

int MainWindow::indexForCredentialId(int id) const
{
    const auto it = std::find_if(m_credentials.begin(), m_credentials.end(),
        [id](const Credential &c) { return c.id() == id; });
    if (it == m_credentials.end()) {
        return -1;
    }
    return static_cast<int>(it - m_credentials.begin());
}

void MainWindow::onSelectionChanged()
{
    const bool hasSelection = selectedRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_copyUsernameButton->setEnabled(hasSelection);
    m_copyPasswordButton->setEnabled(hasSelection);
}

void MainWindow::onTogglePasswordVisibility()
{
    m_passwordsVisible = m_togglePasswordsButton->isChecked();
    m_togglePasswordsButton->setText(m_passwordsVisible ? "Hide Passwords" : "Show Passwords");
    refreshTable();
}

void MainWindow::onCopyUsernameClicked()
{
    const int row = selectedRow();
    const int id = credentialIdForRow(row);
    const int idx = indexForCredentialId(id);
    if (idx < 0) {
        return;
    }

    const Credential &c = m_credentials[idx];
    const QString value = c.username().isEmpty() ? c.email() : c.username();
    QGuiApplication::clipboard()->setText(value);
    m_statusLabel->setText("Username copied to clipboard.");
}

void MainWindow::onCopyPasswordClicked()
{
    const int row = selectedRow();
    const int id = credentialIdForRow(row);
    const int idx = indexForCredentialId(id);
    if (idx < 0) {
        return;
    }

    QGuiApplication::clipboard()->setText(m_credentials[idx].password());
    m_statusLabel->setText("Password copied to clipboard.");
}

void MainWindow::onAddClicked()
{
    CredentialDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Credential toSave = dialog.credential();
        const QString plainPassword = toSave.password();
        toSave.setPassword(m_encryption.encrypt(plainPassword));

        Credential saved = m_db.insert(toSave);
        if (saved.id() < 0) {
            QMessageBox::warning(this, "Save Failed",
                "Could not save the credential:\n" + m_db.lastError());
            return;
        }

        // Keep the plaintext password in memory for display/editing;
        // only the database copy is encrypted.
        saved.setPassword(plainPassword);
        m_credentials.append(saved);
        refreshCategoryFilterOptions();
        refreshTable();
    }
}

void MainWindow::onEditClicked()
{
    const int row = selectedRow();
    const int id = credentialIdForRow(row);
    const int masterIndex = indexForCredentialId(id);
    if (masterIndex < 0) {
        return;
    }

    CredentialDialog dialog(m_credentials[masterIndex], this);
    if (dialog.exec() == QDialog::Accepted) {
        Credential updated = dialog.credential();
        updated.setId(id);
        const QString plainPassword = updated.password();
        updated.setPassword(m_encryption.encrypt(plainPassword));

        if (!m_db.update(updated)) {
            QMessageBox::warning(this, "Update Failed",
                "Could not update the credential:\n" + m_db.lastError());
            return;
        }

        updated.setPassword(plainPassword);
        m_credentials[masterIndex] = updated;
        refreshCategoryFilterOptions();
        refreshTable();
    }
}

void MainWindow::onDeleteClicked()
{
    const int row = selectedRow();
    const int id = credentialIdForRow(row);
    const int masterIndex = indexForCredentialId(id);
    if (masterIndex < 0) {
        return;
    }

    const QString website = m_credentials[masterIndex].website();
    const auto reply = QMessageBox::question(
        this, "Delete Credential",
        QString("Delete the credential for \"%1\"? This can't be undone.").arg(website),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (!m_db.remove(id)) {
            QMessageBox::warning(this, "Delete Failed",
                "Could not delete the credential:\n" + m_db.lastError());
            return;
        }
        m_credentials.removeAt(masterIndex);
        refreshCategoryFilterOptions();
        refreshTable();
    }
}

void MainWindow::onSettingsClicked()
{
    SettingsDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    // Verify the current password against what's actually stored,
    // using a throwaway EncryptionManager so we don't disturb the
    // live, already-unlocked m_encryption unless verification passes.
    QByteArray salt, hash;
    if (!m_db.loadMasterPasswordSetup(salt, hash)) {
        QMessageBox::critical(this, "Settings Error",
            "Could not load current vault security settings:\n" + m_db.lastError());
        return;
    }

    EncryptionManager verifier;
    if (!verifier.unlock(dialog.currentPassword(), salt, hash)) {
        QMessageBox::warning(this, "Incorrect Password",
            "Your current password was incorrect. Master password not changed.");
        return;
    }

    // Derive a brand-new key from the new password. This mutates
    // m_encryption in place (it's the same object main.cpp created
    // and passed in), so every subsequent encrypt/decrypt call in
    // this session uses the new key from here on.
    EncryptionManager::SetupResult newSetup = m_encryption.setupMasterPassword(dialog.newPassword());

    // Re-encrypt every credential's password with the new key and
    // write it back. m_credentials already holds plaintext passwords
    // in memory, so no decryption step is needed here.
    for (Credential &c : m_credentials) {
        Credential toUpdate = c;
        toUpdate.setPassword(m_encryption.encrypt(c.password()));
        if (!m_db.update(toUpdate)) {
            QMessageBox::critical(this, "Settings Error",
                QString("Failed to re-encrypt \"%1\" with the new password. "
                        "Your vault may be in a mixed state, please contact support "
                        "or check the database directly.\n%2")
                    .arg(c.website(), m_db.lastError()));
            return;
        }
    }

    if (!m_db.saveMasterPasswordSetup(newSetup.salt, newSetup.verificationHash)) {
        QMessageBox::critical(this, "Settings Error",
            "Credentials were re-encrypted, but the new master password "
            "settings could not be saved:\n" + m_db.lastError());
        return;
    }

    QMessageBox::information(this, "Password Changed",
        "Your master password has been changed successfully.");
}

void MainWindow::onAboutClicked()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onDarkModeToggled(bool enabled)
{
    Theme::apply(enabled);

    if (!m_db.setDarkModeEnabled(enabled)) {
        // Not worth blocking the user over, the theme still applied
        // for this session, it just won't be remembered next launch.
        QMessageBox::warning(this, "Settings Warning",
            "Dark mode was applied, but the preference could not be saved:\n"
            + m_db.lastError());
    }
}

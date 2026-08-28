#include "MainWindow.h"
#include "CredentialDialog.h"

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
#include <algorithm>

namespace {
constexpr int ColWebsite = 0;
constexpr int ColUsername = 1;
constexpr int ColCategory = 2;
constexpr int ColFavorite = 3;
constexpr int ColModified = 4;
const QString kAllCategories = QStringLiteral("All Categories");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    if (!m_db.open(databasePath())) {
        QMessageBox::critical(this, "Database Error",
            "Could not open the credentials database:\n" + m_db.lastError());
    }

    m_credentials = m_db.loadAll();
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
    resize(900, 600);

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

    auto *filterRow = new QHBoxLayout();
    filterRow->addWidget(m_searchEdit, 1);
    filterRow->addWidget(m_categoryFilterCombo);
    filterRow->addWidget(m_favoritesOnlyCheck);

    // --- Table ---
    m_table = new QTableWidget(0, 5, central);
    m_table->setHorizontalHeaderLabels({"Website/App", "Username", "Category", "Favorite", "Last Modified"});
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
    m_editButton->setEnabled(false);
    m_deleteButton->setEnabled(false);

    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditClicked);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteClicked);

    auto *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(m_addButton);
    buttonRow->addWidget(m_editButton);
    buttonRow->addWidget(m_deleteButton);
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
        m_table->setItem(row, ColCategory, new QTableWidgetItem(c.category()));
        m_table->setItem(row, ColFavorite, new QTableWidgetItem(c.isFavorite() ? "Yes" : ""));
        m_table->setItem(row, ColModified, new QTableWidgetItem(
            c.dateModified().toString("yyyy-MM-dd hh:mm")));

        // Stash the credential's real id in the row so selectedRow()
        // consumers can map back to m_credentials without relying on
        // filtered/unfiltered index alignment.
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

void MainWindow::onSelectionChanged()
{
    const bool hasSelection = selectedRow() >= 0;
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onAddClicked()
{
    CredentialDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Credential saved = m_db.insert(dialog.credential());
        if (saved.id() < 0) {
            QMessageBox::warning(this, "Save Failed",
                "Could not save the credential:\n" + m_db.lastError());
            return;
        }
        m_credentials.append(saved);
        refreshCategoryFilterOptions();
        refreshTable();
    }
}

void MainWindow::onEditClicked()
{
    const int row = selectedRow();
    if (row < 0) {
        return;
    }

    const int id = m_table->item(row, ColWebsite)->data(Qt::UserRole).toInt();
    const int masterIndex = std::find_if(m_credentials.begin(), m_credentials.end(),
        [id](const Credential &c) { return c.id() == id; }) - m_credentials.begin();

    if (masterIndex < 0 || masterIndex >= m_credentials.size()) {
        return;
    }

    CredentialDialog dialog(m_credentials[masterIndex], this);
    if (dialog.exec() == QDialog::Accepted) {
        Credential updated = dialog.credential();
        updated.setId(id);

        if (!m_db.update(updated)) {
            QMessageBox::warning(this, "Update Failed",
                "Could not update the credential:\n" + m_db.lastError());
            return;
        }

        m_credentials[masterIndex] = updated;
        refreshCategoryFilterOptions();
        refreshTable();
    }
}

void MainWindow::onDeleteClicked()
{
    const int row = selectedRow();
    if (row < 0) {
        return;
    }

    const int id = m_table->item(row, ColWebsite)->data(Qt::UserRole).toInt();
    const int masterIndex = std::find_if(m_credentials.begin(), m_credentials.end(),
        [id](const Credential &c) { return c.id() == id; }) - m_credentials.begin();

    if (masterIndex < 0 || masterIndex >= m_credentials.size()) {
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

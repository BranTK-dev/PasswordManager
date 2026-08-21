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
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

namespace {
constexpr int ColWebsite = 0;
constexpr int ColUsername = 1;
constexpr int ColCategory = 2;
constexpr int ColFavorite = 3;
constexpr int ColModified = 4;
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
    refreshTable();
}

MainWindow::~MainWindow()
{
    m_db.close();
}

QString MainWindow::databasePath() const
{
    // Store the database alongside the executable's "database" folder
    // during development. A packaged build would use a proper user
    // data location instead (QStandardPaths::AppDataLocation).
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
    layout->addLayout(buttonRow);
    layout->addWidget(m_table);
    layout->addWidget(m_statusLabel);

    central->setLayout(layout);
    setCentralWidget(central);
}

void MainWindow::refreshTable()
{
    m_table->setRowCount(0);

    for (const Credential &c : m_credentials) {
        const int row = m_table->rowCount();
        m_table->insertRow(row);

        m_table->setItem(row, ColWebsite, new QTableWidgetItem(c.website()));
        m_table->setItem(row, ColUsername, new QTableWidgetItem(
            c.username().isEmpty() ? c.email() : c.username()));
        m_table->setItem(row, ColCategory, new QTableWidgetItem(c.category()));
        m_table->setItem(row, ColFavorite, new QTableWidgetItem(c.isFavorite() ? "Yes" : ""));
        m_table->setItem(row, ColModified, new QTableWidgetItem(
            c.dateModified().toString("yyyy-MM-dd hh:mm")));
    }

    m_statusLabel->setText(QString("%1 credential(s) saved").arg(m_credentials.size()));
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
        refreshTable();
    }
}

void MainWindow::onEditClicked()
{
    const int row = selectedRow();
    if (row < 0 || row >= m_credentials.size()) {
        return;
    }

    CredentialDialog dialog(m_credentials[row], this);
    if (dialog.exec() == QDialog::Accepted) {
        Credential updated = dialog.credential();
        updated.setId(m_credentials[row].id());

        if (!m_db.update(updated)) {
            QMessageBox::warning(this, "Update Failed",
                "Could not update the credential:\n" + m_db.lastError());
            return;
        }

        m_credentials[row] = updated;
        refreshTable();
        m_table->selectRow(row);
    }
}

void MainWindow::onDeleteClicked()
{
    const int row = selectedRow();
    if (row < 0 || row >= m_credentials.size()) {
        return;
    }

    const QString website = m_credentials[row].website();
    const auto reply = QMessageBox::question(
        this, "Delete Credential",
        QString("Delete the credential for \"%1\"? This can't be undone.").arg(website),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        const int id = m_credentials[row].id();
        if (!m_db.remove(id)) {
            QMessageBox::warning(this, "Delete Failed",
                "Could not delete the credential:\n" + m_db.lastError());
            return;
        }
        m_credentials.removeAt(row);
        refreshTable();
    }
}

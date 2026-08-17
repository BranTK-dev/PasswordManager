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

namespace {
constexpr int ColWebsite = 0;
constexpr int ColUsername = 1;
constexpr int ColCategory = 2;
constexpr int ColFavorite = 3;
constexpr int ColModified = 4;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_nextId(1)
{
    setupUi();
    refreshTable();
}

MainWindow::~MainWindow() = default;

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
    // Double-click a row to edit it, same as clicking Edit
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

    layout->addWidget(titleLabel);
    layout->addLayout(buttonRow);
    layout->addWidget(m_table);

    central->setLayout(layout);
    setCentralWidget(central);
}

int MainWindow::nextId()
{
    return m_nextId++;
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
        Credential c = dialog.credential();
        c.setId(nextId());
        m_credentials.append(c);
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
        m_credentials[row] = dialog.credential();
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
        m_credentials.removeAt(row);
        refreshTable();
    }
}

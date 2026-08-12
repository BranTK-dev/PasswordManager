#include "MainWindow.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Password Manager");
    resize(900, 600);

    // Placeholder central widget for now, this becomes the real
    // dashboard (credential list, toolbar, search bar) in Phase 4.
    auto *central = new QWidget(this);
    auto *layout = new QVBoxLayout(central);

    auto *placeholder = new QLabel("Password Manager - Dashboard coming soon", central);
    placeholder->setAlignment(Qt::AlignCenter);

    layout->addWidget(placeholder);
    central->setLayout(layout);

    setCentralWidget(central);
}

MainWindow::~MainWindow() = default;

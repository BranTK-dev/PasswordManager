#include "MainWindow.h"
#include "LoginDialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginDialog loginDialog;

    // exec() blocks until the dialog is accepted (Unlock) or rejected
    // (closed/cancelled). If the user closes the login screen, quit
    // instead of falling through to the main window.
    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window;
    window.show();

    return app.exec();
}

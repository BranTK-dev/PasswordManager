#include "MainWindow.h"
#include "LoginDialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginDialog loginDialog;

    if (loginDialog.exec() != QDialog::Accepted) {
        return 0;
    }

    MainWindow window;
    window.show();

    return app.exec();
}

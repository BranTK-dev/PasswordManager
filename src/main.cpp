#include "MainWindow.h"
#include "LoginDialog.h"
#include "DatabaseManager.h"
#include "EncryptionManager.h"
#include "Theme.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QIcon>

namespace {

QString databasePath()
{
    QDir appDir(QCoreApplication::applicationDirPath());
    return appDir.filePath("../database/vault.db");
}

// Returns true if the user successfully created or unlocked the vault.
// On success, encryptionManager is left unlocked and ready to use.
bool authenticate(DatabaseManager &db, EncryptionManager &encryptionManager)
{
    if (!db.hasMasterPasswordSetup()) {
        // First run: create a new master password
        LoginDialog setupDialog(LoginDialog::Mode::SetupNew);
        if (setupDialog.exec() != QDialog::Accepted) {
            return false;
        }

        EncryptionManager::SetupResult setup =
            encryptionManager.setupMasterPassword(setupDialog.password());

        if (!db.saveMasterPasswordSetup(setup.salt, setup.verificationHash)) {
            QMessageBox::critical(nullptr, "Setup Failed",
                "Could not save the master password:\n" + db.lastError());
            return false;
        }

        return true;
    }

    // Vault already exists: verify the entered password, retry on
    // failure rather than quitting immediately.
    QByteArray salt;
    QByteArray verificationHash;
    if (!db.loadMasterPasswordSetup(salt, verificationHash)) {
        QMessageBox::critical(nullptr, "Vault Error",
            "Could not load vault security settings:\n" + db.lastError());
        return false;
    }

    while (true) {
        LoginDialog loginDialog(LoginDialog::Mode::Login);
        if (loginDialog.exec() != QDialog::Accepted) {
            return false; // user closed the dialog
        }

        if (encryptionManager.unlock(loginDialog.password(), salt, verificationHash)) {
            return true;
        }

        QMessageBox::warning(nullptr, "Incorrect Password",
            "That master password is incorrect. Please try again.");
    }
}

} // namespace

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/app_icon.png"));

    DatabaseManager db;
    if (!db.open(databasePath())) {
        QMessageBox::critical(nullptr, "Database Error",
            "Could not open the credentials database:\n" + db.lastError());
        return 1;
    }

    // Apply the saved theme before any window is shown, so the very
    // first thing the user sees (the login screen) is already correct
    // rather than flashing light-then-dark.
    Theme::apply(db.isDarkModeEnabled());

    EncryptionManager encryptionManager;
    if (!authenticate(db, encryptionManager)) {
        db.close();
        return 0;
    }

    db.close(); // MainWindow opens its own connection to the same file

    MainWindow window(encryptionManager);
    window.show();

    return app.exec();
}

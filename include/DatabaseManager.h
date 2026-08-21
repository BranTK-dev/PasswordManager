#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QVector>
#include <QSqlDatabase>
#include "Credential.h"

// Owns the SQLite connection and all reads/writes for credentials.
// Passwords are stored as plain text at this phase, encryption is
// layered in on top of this in Phase 7 (EncryptionManager encrypts
// before DatabaseManager writes, and decrypts after it reads).
class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    // Opens (creating if needed) the SQLite file at dbPath and
    // ensures the credentials table exists. Returns false on failure,
    // check lastError() for details.
    bool open(const QString &dbPath);
    void close();
    bool isOpen() const;

    QString lastError() const;

    // Loads every credential from the database, ordered by website.
    QVector<Credential> loadAll();

    // Inserts a new credential and returns it with its assigned id
    // (id -1 on failure).
    Credential insert(const Credential &credential);

    // Updates an existing credential (matched by id). Returns false
    // if the credential has no valid id or the update fails.
    bool update(const Credential &credential);

    // Deletes a credential by id. Returns false on failure.
    bool remove(int id);

private:
    bool createTableIfNeeded();

    QSqlDatabase m_db;
    QString m_lastError;
    // Each DatabaseManager instance uses its own named connection so
    // multiple instances (e.g. in tests) don't collide.
    QString m_connectionName;
};

#endif // DATABASEMANAGER_H

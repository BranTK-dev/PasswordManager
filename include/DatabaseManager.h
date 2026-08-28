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

    bool open(const QString &dbPath);
    void close();
    bool isOpen() const;

    QString lastError() const;

    QVector<Credential> loadAll();
    Credential insert(const Credential &credential);
    bool update(const Credential &credential);
    bool remove(int id);

private:
    bool createTableIfNeeded();

    QSqlDatabase m_db;
    QString m_lastError;
    QString m_connectionName;
};

#endif // DATABASEMANAGER_H

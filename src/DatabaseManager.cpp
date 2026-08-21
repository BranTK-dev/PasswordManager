#include "DatabaseManager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QUuid>
#include <QFileInfo>
#include <QDir>

DatabaseManager::DatabaseManager()
{
    // Unique connection name per instance, avoids "connection already
    // exists" warnings if more than one DatabaseManager is ever created
    // (e.g. in unit tests).
    m_connectionName = QStringLiteral("pw_manager_conn_%1")
                           .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &dbPath)
{
    QFileInfo info(dbPath);
    QDir dir = info.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    if (!createTableIfNeeded()) {
        return false;
    }

    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }

    // m_db must be cleared (reset to a null QSqlDatabase) before
    // removeDatabase() runs, otherwise Qt sees this object as still
    // holding a reference to the connection and logs a warning.
    m_db = QSqlDatabase();

    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::createTableIfNeeded()
{
    QSqlQuery query(m_db);
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS credentials ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "website TEXT NOT NULL, "
        "url TEXT, "
        "username TEXT, "
        "email TEXT, "
        "password TEXT, "
        "category TEXT, "
        "notes TEXT, "
        "favorite INTEGER NOT NULL DEFAULT 0, "
        "date_created TEXT NOT NULL, "
        "date_modified TEXT NOT NULL"
        ")"
    );

    if (!ok) {
        m_lastError = query.lastError().text();
        return false;
    }
    return true;
}

QVector<Credential> DatabaseManager::loadAll()
{
    QVector<Credential> results;

    if (!m_db.isOpen()) {
        return results;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, website, url, username, email, password, category, "
        "notes, favorite, date_created, date_modified "
        "FROM credentials ORDER BY website COLLATE NOCASE ASC"
    );

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return results;
    }

    while (query.next()) {
        Credential c;
        c.setId(query.value("id").toInt());
        c.setWebsite(query.value("website").toString());
        c.setUrl(query.value("url").toString());
        c.setUsername(query.value("username").toString());
        c.setEmail(query.value("email").toString());
        c.setPassword(query.value("password").toString());
        c.setCategory(query.value("category").toString());
        c.setNotes(query.value("notes").toString());
        c.setFavorite(query.value("favorite").toInt() != 0);
        c.setDateCreated(QDateTime::fromString(query.value("date_created").toString(), Qt::ISODate));
        c.setDateModified(QDateTime::fromString(query.value("date_modified").toString(), Qt::ISODate));
        results.append(c);
    }

    return results;
}

Credential DatabaseManager::insert(const Credential &credential)
{
    Credential result = credential;

    if (!m_db.isOpen()) {
        m_lastError = "Database is not open.";
        result.setId(-1);
        return result;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO credentials "
        "(website, url, username, email, password, category, notes, favorite, date_created, date_modified) "
        "VALUES (:website, :url, :username, :email, :password, :category, :notes, :favorite, :date_created, :date_modified)"
    );

    query.bindValue(":website", credential.website());
    query.bindValue(":url", credential.url());
    query.bindValue(":username", credential.username());
    query.bindValue(":email", credential.email());
    query.bindValue(":password", credential.password());
    query.bindValue(":category", credential.category());
    query.bindValue(":notes", credential.notes());
    query.bindValue(":favorite", credential.isFavorite() ? 1 : 0);
    query.bindValue(":date_created", credential.dateCreated().toString(Qt::ISODate));
    query.bindValue(":date_modified", credential.dateModified().toString(Qt::ISODate));

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        result.setId(-1);
        return result;
    }

    result.setId(query.lastInsertId().toInt());
    return result;
}

bool DatabaseManager::update(const Credential &credential)
{
    if (!m_db.isOpen() || credential.id() < 0) {
        m_lastError = "Database not open or credential has no valid id.";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "UPDATE credentials SET "
        "website = :website, url = :url, username = :username, email = :email, "
        "password = :password, category = :category, notes = :notes, "
        "favorite = :favorite, date_modified = :date_modified "
        "WHERE id = :id"
    );

    query.bindValue(":website", credential.website());
    query.bindValue(":url", credential.url());
    query.bindValue(":username", credential.username());
    query.bindValue(":email", credential.email());
    query.bindValue(":password", credential.password());
    query.bindValue(":category", credential.category());
    query.bindValue(":notes", credential.notes());
    query.bindValue(":favorite", credential.isFavorite() ? 1 : 0);
    query.bindValue(":date_modified", credential.dateModified().toString(Qt::ISODate));
    query.bindValue(":id", credential.id());

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::remove(int id)
{
    if (!m_db.isOpen() || id < 0) {
        m_lastError = "Database not open or invalid id.";
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM credentials WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return false;
    }

    return true;
}

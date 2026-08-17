#include "Credential.h"

Credential::Credential()
    : m_id(-1)
    , m_favorite(false)
    , m_dateCreated(QDateTime::currentDateTime())
    , m_dateModified(QDateTime::currentDateTime())
{
}

Credential::Credential(const QString &website,
                        const QString &username,
                        const QString &password)
    : m_id(-1)
    , m_website(website)
    , m_username(username)
    , m_password(password)
    , m_favorite(false)
    , m_dateCreated(QDateTime::currentDateTime())
    , m_dateModified(QDateTime::currentDateTime())
{
}

int Credential::id() const
{
    return m_id;
}

void Credential::setId(int id)
{
    m_id = id;
}

QString Credential::website() const
{
    return m_website;
}

void Credential::setWebsite(const QString &website)
{
    m_website = website;
}

QString Credential::url() const
{
    return m_url;
}

void Credential::setUrl(const QString &url)
{
    m_url = url;
}

QString Credential::username() const
{
    return m_username;
}

void Credential::setUsername(const QString &username)
{
    m_username = username;
}

QString Credential::email() const
{
    return m_email;
}

void Credential::setEmail(const QString &email)
{
    m_email = email;
}

QString Credential::password() const
{
    return m_password;
}

void Credential::setPassword(const QString &password)
{
    m_password = password;
}

QString Credential::category() const
{
    return m_category;
}

void Credential::setCategory(const QString &category)
{
    m_category = category;
}

QString Credential::notes() const
{
    return m_notes;
}

void Credential::setNotes(const QString &notes)
{
    m_notes = notes;
}

bool Credential::isFavorite() const
{
    return m_favorite;
}

void Credential::setFavorite(bool favorite)
{
    m_favorite = favorite;
}

QDateTime Credential::dateCreated() const
{
    return m_dateCreated;
}

void Credential::setDateCreated(const QDateTime &dateTime)
{
    m_dateCreated = dateTime;
}

QDateTime Credential::dateModified() const
{
    return m_dateModified;
}

void Credential::setDateModified(const QDateTime &dateTime)
{
    m_dateModified = dateTime;
}

void Credential::touch()
{
    m_dateModified = QDateTime::currentDateTime();
}

bool Credential::isValid() const
{
    const bool hasIdentifier = !m_username.isEmpty() || !m_email.isEmpty();
    return !m_website.isEmpty() && hasIdentifier;
}

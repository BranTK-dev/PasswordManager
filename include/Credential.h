#ifndef CREDENTIAL_H
#define CREDENTIAL_H

#include <QString>
#include <QDateTime>

// Represents a single stored login/credential entry.
// This is a plain data holder, no database or encryption logic here,
// that's DatabaseManager and EncryptionManager's job (Phase 5 / 7).
class Credential
{
public:
    Credential();

    // Convenience constructor for creating a brand-new credential.
    // id, dateCreated, and dateModified are set automatically.
    Credential(const QString &website,
               const QString &username,
               const QString &password);

    // --- Identity ---
    int id() const;
    void setId(int id);

    // --- Core fields ---
    QString website() const;
    void setWebsite(const QString &website);

    QString url() const;
    void setUrl(const QString &url);

    QString username() const;
    void setUsername(const QString &username);

    QString email() const;
    void setEmail(const QString &email);

    QString password() const;
    void setPassword(const QString &password);

    QString category() const;
    void setCategory(const QString &category);

    QString notes() const;
    void setNotes(const QString &notes);

    bool isFavorite() const;
    void setFavorite(bool favorite);

    // --- Timestamps ---
    QDateTime dateCreated() const;
    void setDateCreated(const QDateTime &dateTime);

    QDateTime dateModified() const;
    void setDateModified(const QDateTime &dateTime);

    // Call whenever the credential's fields are edited, updates
    // dateModified to now. CRUD/edit flows in Phase 4 should call this.
    void touch();

    // True if website and username/email are both non-empty.
    // Used to validate before saving in the add/edit dialogs.
    bool isValid() const;

private:
    int m_id;
    QString m_website;
    QString m_url;
    QString m_username;
    QString m_email;
    QString m_password;
    QString m_category;
    QString m_notes;
    bool m_favorite;
    QDateTime m_dateCreated;
    QDateTime m_dateModified;
};

#endif // CREDENTIAL_H

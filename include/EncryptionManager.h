#ifndef ENCRYPTIONMANAGER_H
#define ENCRYPTIONMANAGER_H

#include <QString>
#include <QByteArray>

// Handles everything related to the master password:
// - Deriving an encryption key from it (PBKDF2)
// - Encrypting/decrypting credential passwords with that key (AES-256-GCM)
// - Verifying a master password is correct without ever storing it
//
// The master password itself is NEVER stored anywhere. What gets
// stored (in the settings table) is a random salt and a verification
// hash. On login, we re-derive a key from the entered password and
// the stored salt, then check the hash matches.
class EncryptionManager
{
public:
    EncryptionManager();

    // Call once, at first-run, to set up a brand-new master password.
    // Generates a random salt, derives a key, and returns the salt +
    // verification hash to be persisted by SettingsManager/DatabaseManager.
    struct SetupResult {
        QByteArray salt;
        QByteArray verificationHash;
    };
    SetupResult setupMasterPassword(const QString &masterPassword);

    // Call on login. Derives a key from the entered password and the
    // stored salt, then compares against the stored verification hash.
    // If it matches, the manager is "unlocked" and ready to
    // encrypt/decrypt; if not, it stays locked.
    bool unlock(const QString &masterPassword,
                const QByteArray &storedSalt,
                const QByteArray &storedVerificationHash);

    bool isUnlocked() const;

    // Encrypts plaintext using the key derived during unlock().
    // Returns a base64-encoded string safe to store in SQLite,
    // formatted as: base64(iv + ciphertext + authTag).
    // Returns an empty string if not unlocked or on failure.
    QString encrypt(const QString &plaintext) const;

    // Reverses encrypt(). Returns an empty string if not unlocked,
    // the input is malformed, or authentication fails (meaning the
    // data was tampered with or the key is wrong).
    QString decrypt(const QString &encoded) const;

private:
    // PBKDF2-HMAC-SHA256, 32-byte key, configurable iteration count.
    QByteArray deriveKey(const QString &password, const QByteArray &salt) const;

    static constexpr int kSaltSize = 16;
    static constexpr int kKeySize = 32;   // AES-256
    static constexpr int kIvSize = 12;    // recommended IV size for GCM
    static constexpr int kTagSize = 16;   // GCM auth tag size
    static constexpr int kIterations = 200000;

    QByteArray m_key;
    bool m_unlocked;
};

#endif // ENCRYPTIONMANAGER_H

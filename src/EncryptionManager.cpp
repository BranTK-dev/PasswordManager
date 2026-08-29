#include "EncryptionManager.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/kdf.h>

#include <QByteArray>
#include <cstring>

EncryptionManager::EncryptionManager()
    : m_unlocked(false)
{
}

QByteArray EncryptionManager::deriveKey(const QString &password, const QByteArray &salt) const
{
    QByteArray key(kKeySize, '\0');
    const QByteArray passwordBytes = password.toUtf8();

    const int ok = PKCS5_PBKDF2_HMAC(
        passwordBytes.constData(), passwordBytes.size(),
        reinterpret_cast<const unsigned char *>(salt.constData()), salt.size(),
        kIterations,
        EVP_sha256(),
        kKeySize,
        reinterpret_cast<unsigned char *>(key.data())
    );

    if (ok != 1) {
        return QByteArray();
    }
    return key;
}

EncryptionManager::SetupResult EncryptionManager::setupMasterPassword(const QString &masterPassword)
{
    SetupResult result;

    QByteArray salt(kSaltSize, '\0');
    RAND_bytes(reinterpret_cast<unsigned char *>(salt.data()), kSaltSize);

    QByteArray key = deriveKey(masterPassword, salt);

    // The "verification hash" is just the derived key run through the
    // KDF a second time with a fixed, different salt suffix. This way
    // we can confirm the password is correct on future logins without
    // storing the actual encryption key anywhere.
    QByteArray verificationSalt = salt + QByteArrayLiteral("verify");
    QByteArray verificationHash = deriveKey(masterPassword, verificationSalt);

    result.salt = salt;
    result.verificationHash = verificationHash;

    m_key = key;
    m_unlocked = true;

    return result;
}

bool EncryptionManager::unlock(const QString &masterPassword,
                                const QByteArray &storedSalt,
                                const QByteArray &storedVerificationHash)
{
    QByteArray verificationSalt = storedSalt + QByteArrayLiteral("verify");
    QByteArray candidateHash = deriveKey(masterPassword, verificationSalt);

    if (candidateHash.isEmpty() || candidateHash != storedVerificationHash) {
        m_unlocked = false;
        m_key.clear();
        return false;
    }

    m_key = deriveKey(masterPassword, storedSalt);
    m_unlocked = !m_key.isEmpty();
    return m_unlocked;
}

bool EncryptionManager::isUnlocked() const
{
    return m_unlocked;
}

QString EncryptionManager::encrypt(const QString &plaintext) const
{
    if (!m_unlocked || m_key.size() != kKeySize) {
        return QString();
    }

    QByteArray iv(kIvSize, '\0');
    RAND_bytes(reinterpret_cast<unsigned char *>(iv.data()), kIvSize);

    QByteArray plainBytes = plaintext.toUtf8();
    QByteArray cipherBytes(plainBytes.size(), '\0');
    QByteArray tag(kTagSize, '\0');

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return QString();
    }

    bool ok = true;
    int len = 0;
    int cipherLen = 0;

    ok = ok && EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
    ok = ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvSize, nullptr) == 1;
    ok = ok && EVP_EncryptInit_ex(ctx, nullptr, nullptr,
                                   reinterpret_cast<const unsigned char *>(m_key.constData()),
                                   reinterpret_cast<const unsigned char *>(iv.constData())) == 1;

    if (ok && plainBytes.size() > 0) {
        ok = EVP_EncryptUpdate(ctx,
                                reinterpret_cast<unsigned char *>(cipherBytes.data()), &len,
                                reinterpret_cast<const unsigned char *>(plainBytes.constData()),
                                plainBytes.size()) == 1;
        cipherLen = len;
    }

    ok = ok && EVP_EncryptFinal_ex(ctx,
                                    reinterpret_cast<unsigned char *>(cipherBytes.data()) + cipherLen,
                                    &len) == 1;
    cipherLen += len;

    ok = ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kTagSize, tag.data()) == 1;

    EVP_CIPHER_CTX_free(ctx);

    if (!ok) {
        return QString();
    }

    cipherBytes.resize(cipherLen);

    // Layout: iv || ciphertext || tag, then base64-encode the whole thing
    QByteArray combined = iv + cipherBytes + tag;
    return QString::fromLatin1(combined.toBase64());
}

QString EncryptionManager::decrypt(const QString &encoded) const
{
    if (!m_unlocked || m_key.size() != kKeySize || encoded.isEmpty()) {
        return QString();
    }

    QByteArray combined = QByteArray::fromBase64(encoded.toLatin1());
    if (combined.size() < kIvSize + kTagSize) {
        return QString(); // malformed, too short to contain iv + tag
    }

    QByteArray iv = combined.left(kIvSize);
    QByteArray tag = combined.right(kTagSize);
    QByteArray cipherBytes = combined.mid(kIvSize, combined.size() - kIvSize - kTagSize);

    QByteArray plainBytes(cipherBytes.size(), '\0');

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        return QString();
    }

    bool ok = true;
    int len = 0;
    int plainLen = 0;

    ok = ok && EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1;
    ok = ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, kIvSize, nullptr) == 1;
    ok = ok && EVP_DecryptInit_ex(ctx, nullptr, nullptr,
                                   reinterpret_cast<const unsigned char *>(m_key.constData()),
                                   reinterpret_cast<const unsigned char *>(iv.constData())) == 1;

    if (ok && cipherBytes.size() > 0) {
        ok = EVP_DecryptUpdate(ctx,
                                reinterpret_cast<unsigned char *>(plainBytes.data()), &len,
                                reinterpret_cast<const unsigned char *>(cipherBytes.constData()),
                                cipherBytes.size()) == 1;
        plainLen = len;
    }

    ok = ok && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kTagSize,
                                    const_cast<char *>(tag.constData())) == 1;

    // EVP_DecryptFinal_ex returns 0 (failure) if the auth tag doesn't
    // match, meaning the ciphertext was tampered with or the key is
    // wrong. This is GCM's built-in integrity check.
    int finalRet = 0;
    if (ok) {
        finalRet = EVP_DecryptFinal_ex(ctx,
                                        reinterpret_cast<unsigned char *>(plainBytes.data()) + plainLen,
                                        &len);
    }

    EVP_CIPHER_CTX_free(ctx);

    if (!ok || finalRet != 1) {
        return QString();
    }

    plainLen += len;
    plainBytes.resize(plainLen);
    return QString::fromUtf8(plainBytes);
}

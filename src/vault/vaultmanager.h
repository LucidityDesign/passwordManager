#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QString>
#include <QList>
#include <QObject>
#include <QTimerEvent>
#include <sodium.h>
#include "../crypto/cryptoutils.h"
#include "../utils/fileutils.h"

struct VaultEntry
{
  QString username;
  QString password;             // Temporary plaintext storage - will be cleared after encryption
  QByteArray encryptedPassword; // Binary encrypted storage with individual salt

  /**
   * @brief Encrypts the plaintext password with military-grade security
   * Each password gets its own unique salt for maximum security
   * @param masterKey The derived master key (QByteArray) for encryption
   */
  void encryptPassword(const QByteArray &masterKey)
  {
    if (password.isEmpty())
    {
      throw CryptoUtils::CryptoOperationError("Cannot encrypt empty password");
    }

    // Generate a unique salt for this specific password entry
    QByteArray individualSalt = FileUtils::generateSalt();

    // Use the master key directly with the individual salt for key derivation
    // This creates a unique key for each password without storing the master password
    QByteArray derivedKey = CryptoUtils::deriveKeyFromPassword(QString::fromUtf8(masterKey), individualSalt);

    QByteArray nonce, ciphertext;

    try
    {
      CryptoUtils::encrypt(password.toUtf8(), derivedKey, ciphertext, nonce);
    }
    catch (const CryptoUtils::CryptoOperationError &e)
    {
      // Clear sensitive data before throwing
      derivedKey.fill(0);
      throw;
    }

    // Store format: individual_salt (16 bytes) + nonce (24 bytes) + ciphertext
    encryptedPassword = individualSalt + nonce + ciphertext;

    // Securely clear sensitive data from memory
    clearSensitiveData();
    derivedKey.fill(0);
  }

  /**
   * @brief Decrypts the password on-demand with military-grade security
   * @param masterKey The derived master key (QByteArray) for decryption
   * @return Decrypted password as QString
   */
  QString decryptPassword(const QByteArray &masterKey) const
  {
    if (encryptedPassword.isEmpty())
    {
      throw CryptoUtils::CryptoOperationError("No encrypted password data");
    }

    // Parse encrypted data format: individual_salt + nonce + ciphertext
    const int SALT_SIZE = crypto_pwhash_SALTBYTES;                       // crypto_pwhash_SALTBYTES
    const int NONCE_SIZE = crypto_aead_xchacha20poly1305_ietf_NPUBBYTES; // crypto_aead_xchacha20poly1305_ietf_NPUBBYTES

    if (encryptedPassword.size() < SALT_SIZE + NONCE_SIZE)
    {
      throw CryptoUtils::CryptoOperationError("Invalid encrypted password format");
    }

    QByteArray individualSalt = encryptedPassword.left(SALT_SIZE);
    QByteArray nonce = encryptedPassword.mid(SALT_SIZE, NONCE_SIZE);
    QByteArray ciphertext = encryptedPassword.mid(SALT_SIZE + NONCE_SIZE);

    // Use the same key derivation as during encryption
    QByteArray derivedKey = CryptoUtils::deriveKeyFromPassword(QString::fromUtf8(masterKey), individualSalt);

    QByteArray decrypted;

    try
    {
      CryptoUtils::decrypt(ciphertext, derivedKey, nonce, decrypted);
    }
    catch (const CryptoUtils::CryptoOperationError &e)
    {
      derivedKey.fill(0);
      throw;
    }

    QString result = QString::fromUtf8(decrypted);

    // Securely clear sensitive data from memory
    decrypted.fill(0);
    derivedKey.fill(0);

    return result;
  }

  /**
   * @brief Check if this entry contains encrypted password data
   * @return true if password is encrypted, false otherwise
   */
  bool isEncrypted() const { return !encryptedPassword.isEmpty(); }

  /**
   * @brief Securely clear all sensitive data from memory
   * Call this when the entry is no longer needed
   */
  void clearSensitiveData()
  {
    password.fill(QChar(0));
    password.clear();
    // Note: We don't clear encryptedPassword as it's needed for storage
  }
};

class VaultManager : public QObject
{
  Q_OBJECT

public:
  VaultManager();
  ~VaultManager();
  void openVault(const QString &filePath, const QString &password);
  // void saveVault(const QString &filePath);
  void addEntry(const VaultEntry &entry);
  void removeEntry(const QString &username);
  void updateEntry(const QString &username, const QString &newPassword);
  QList<VaultEntry> getEntries() const;
  bool isVaultOpen() const { return m_isVaultOpen; }
  void closeVault();
  void startSession(const QString &password);
  void extendSession();

  // Method to get password on demand
  QString getPassword(const QString &username);

  // Method to get password securely with automatic memory clearing
  QString getPasswordSecure(const QString &username);

signals:
  /**
   * @brief Emitted when the vault is successfully opened
   * @param filePath Path to the opened vault file
   */
  void vaultOpened(const QString &filePath);

  /**
   * @brief Emitted when the vault is closed (manually or due to timeout)
   * @param reason Reason for closure ("manual", "timeout", "error", etc.)
   */
  void vaultClosed(const QString &reason);

  /**
   * @brief Emitted when a new entry is added to the vault
   * @param entry The entry that was added
   */
  void entryAdded(const VaultEntry &entry);

private:
  QByteArray m_vaultSessionKey;   // For vault operations
  QByteArray m_passwordMasterKey; // Derived key for password encryption (no plaintext password stored)
  int m_sessionTimer;
  QString m_filePath;
  QByteArray m_decrypted;
  QList<VaultEntry> m_entries; // List of username-password pairs
  bool m_isVaultOpen = false;
  void loadEntries(QByteArray decryptedData);
  void saveEntries(QList<VaultEntry> entries);

protected:
  void timerEvent(QTimerEvent *event) override;
};

#endif // VAULTMANAGER_H

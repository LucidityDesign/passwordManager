#include "vaultmanager.h"
#include "../utils/fileutils.h"
#include "../crypto/cryptoutils.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPalette>
#include <QTimerEvent>
#include <QCryptographicHash>

constexpr int SESSION_TIMEOUT = 15 * 60 * 1000; // 15 minutes in milliseconds

VaultManager::VaultManager()
{
}

void VaultManager::openVault(const QString &filePath, const QString &password)
{

  if (!FileUtils::exists(filePath))
  {
    FileUtils::createVault(filePath, password);
  }
  // Implementation for opening the vault
  m_isVaultOpen = true;

  QByteArray decrypted = FileUtils::readVault(filePath, password);

  qDebug() << "Decrypted Data is: " << decrypted << "\n";

  // TODO: Don't store plain text password in memory
  m_decrypted = decrypted;
  m_filePath = filePath;

  startSession(password);
  loadEntries(m_decrypted);

  // ✅ Emit signal that vault was opened
  emit vaultOpened(filePath);
}

void VaultManager::closeVault()
{
  killTimer(m_sessionTimer);

  // Securely clear all sensitive data
  for (VaultEntry &entry : m_entries)
  {
    entry.clearSensitiveData();
  }

  // Implementation for closing the vault
  m_decrypted.fill(0);
  m_decrypted.clear();
  m_entries.clear();
  m_filePath.clear();

  // Securely clear cryptographic keys
  m_vaultSessionKey.fill(0);
  m_vaultSessionKey.clear();
  m_passwordMasterKey.fill(0);
  m_passwordMasterKey.clear();

  m_isVaultOpen = false;

  // ✅ Emit signal that vault was closed
  emit vaultClosed("manual");
}

VaultManager::~VaultManager()
{
  closeVault();
}

QList<VaultEntry> VaultManager::getEntries() const
{
  return m_entries;
}

void VaultManager::loadEntries(QByteArray decryptedData)
{
  QJsonDocument doc = QJsonDocument::fromJson(decryptedData);
  if (doc.isArray())
  {
    QJsonArray array = doc.array();
    for (const QJsonValue &value : array)
    {
      if (value.isObject())
      {
        QJsonObject obj = value.toObject();
        QString username = obj.value("username").toString();

        // Check for new format (encrypted password)
        if (obj.contains("encryptedPassword"))
        {
          QByteArray encryptedPassword = QByteArray::fromBase64(obj.value("encryptedPassword").toString().toUtf8());
          VaultEntry entry;
          entry.username = username;
          entry.encryptedPassword = encryptedPassword;
          m_entries.append(entry);
        }
        else
        {
          // Legacy format - plaintext password, encrypt it now
          QString plainPassword = obj.value("password").toString();
          VaultEntry entry;
          entry.username = username;
          entry.password = plainPassword;
          entry.encryptPassword(m_passwordMasterKey);
          m_entries.append(entry);
        }
      }
    }
  }
}

void VaultManager::addEntry(const VaultEntry &entry)
{
  // Create a copy to encrypt
  VaultEntry encryptedEntry = entry;

  // Encrypt the password if it's not already encrypted
  if (!encryptedEntry.isEncrypted())
  {
    encryptedEntry.encryptPassword(m_passwordMasterKey);
  }

  // Add to our list
  m_entries.append(encryptedEntry);

  // Save to disk
  saveEntries(m_entries);

  // ✅ Emit signal that entry was added
  emit entryAdded(encryptedEntry);
}

void VaultManager::saveEntries(QList<VaultEntry> entries)
{
  // Implementation for saving entries
  QJsonArray array;
  for (const VaultEntry &entry : entries)
  {
    QJsonObject obj;
    obj["username"] = entry.username;
    // Store encrypted password as base64 string
    obj["encryptedPassword"] = QString::fromUtf8(entry.encryptedPassword.toBase64());
    array.append(obj);
  }

  QJsonDocument doc(array);
  FileUtils::updateVault(m_filePath, m_vaultSessionKey, doc.toJson());
}

void VaultManager::startSession(const QString &password)
{
  QByteArray vaultSalt = FileUtils::extractSalt(m_filePath);

  // Derive session key for vault operations
  m_vaultSessionKey = deriveKeyFromPassword(password, vaultSalt);

  // Create a separate salt for password encryption to ensure key independence
  // We need a deterministic but different salt, so we'll derive it from the vault salt
  QByteArray passwordSaltBase = vaultSalt + QByteArray("PASSWORD_ENCRYPTION", 18);

  // Hash the combined data to create a proper salt
  QByteArray passwordSalt = QCryptographicHash::hash(passwordSaltBase, QCryptographicHash::Sha256).left(crypto_pwhash_SALTBYTES);

  // Derive a separate master key for password encryption/decryption using independent salt
  m_passwordMasterKey = deriveKeyFromPassword(password, passwordSalt);

  m_sessionTimer = startTimer(SESSION_TIMEOUT);
  m_isVaultOpen = true;
}

void VaultManager::extendSession()
{
  if (m_isVaultOpen)
  {
    killTimer(m_sessionTimer);
    m_sessionTimer = startTimer(SESSION_TIMEOUT);
  }
}

void VaultManager::timerEvent(QTimerEvent *event)
{
  if (event->timerId() == m_sessionTimer)
  {
    // Handle session timeout
    qDebug() << "Session timed out";
    closeVault();
  }
  QObject::timerEvent(event);
}

QString VaultManager::getPassword(const QString &username)
{
  // Find the entry and decrypt its password on demand
  for (const VaultEntry &entry : m_entries)
  {
    if (entry.username == username)
    {
      try
      {
        return entry.decryptPassword(m_passwordMasterKey);
      }
      catch (const FileUtils::CryptoOperationError &e)
      {
        qWarning() << "Failed to decrypt password for" << username << ":" << e.what();
        return QString();
      }
    }
  }

  qWarning() << "Entry not found:" << username;
  return QString(); // Entry not found
}

QString VaultManager::getPasswordSecure(const QString &username)
{
  // Extend session when accessing sensitive data
  extendSession();

  // Find the entry and decrypt its password on demand
  for (const VaultEntry &entry : m_entries)
  {
    if (entry.username == username)
    {
      try
      {
        QString decryptedPassword = entry.decryptPassword(m_passwordMasterKey);

        // Note: The caller is responsible for securely handling the returned password
        // Consider using it immediately and not storing it in variables
        return decryptedPassword;
      }
      catch (const FileUtils::CryptoOperationError &e)
      {
        qWarning() << "Failed to decrypt password for" << username << ":" << e.what();
        return QString();
      }
    }
  }

  qWarning() << "Entry not found:" << username;
  return QString(); // Entry not found
}

void VaultManager::removeEntry(const QString &username)
{
  for (int i = 0; i < m_entries.size(); ++i)
  {
    if (m_entries[i].username == username)
    {
      // Securely clear the entry before removal
      m_entries[i].clearSensitiveData();
      m_entries.removeAt(i);

      // Save updated entries
      saveEntries(m_entries);
      return;
    }
  }

  qWarning() << "Entry not found for removal:" << username;
}

void VaultManager::updateEntry(const QString &username, const QString &newPassword)
{
  for (VaultEntry &entry : m_entries)
  {
    if (entry.username == username)
    {
      // Clear old sensitive data
      entry.clearSensitiveData();

      // Set new password and encrypt it
      entry.password = newPassword;
      entry.encryptPassword(m_passwordMasterKey);

      // Save updated entries
      saveEntries(m_entries);
      return;
    }
  }

  qWarning() << "Entry not found for update:" << username;
}

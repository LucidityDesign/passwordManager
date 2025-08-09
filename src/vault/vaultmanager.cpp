#include "vaultmanager.h"
#include "../utils/fileutils.h"
#include "../crypto/cryptoutils.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPalette>
#include <QTimerEvent>

constexpr int SESSION_TIMEOUT = 15 * 60 * 1000; // 15 minutes in milliseconds

VaultManager::VaultManager()
{
}

void VaultManager::openVault(const QString &filePath, const QString &password)
{

    if (!FileUtils::exists(filePath)) {
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
}

void VaultManager::closeVault()
{
  killTimer(m_sessionTimer);

  // Implementation for closing the vault
  m_decrypted.clear();
  m_entries.clear();

  m_isVaultOpen = false;
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
        QString password = obj.value("password").toString();
        m_entries.append({username, password});
      }
    }
  }
}

void VaultManager::addEntry(const VaultEntry &entry)
{
  // Implementation for adding an entry
  m_entries.append(entry);

  saveEntries(m_entries);
}

void VaultManager::saveEntries(QList<VaultEntry> entries)
{
  // Implementation for saving entries
  QJsonArray array;
  for (const VaultEntry &entry : entries)
  {
    QJsonObject obj;
    obj["username"] = entry.username;
    obj["password"] = entry.password;
    array.append(obj);
  }

  QJsonDocument doc(array);
  FileUtils::updateVault(m_filePath, m_sessionKey, doc.toJson());
}

void VaultManager::startSession(const QString &password)
{
  QByteArray salt = FileUtils::extractSalt(m_filePath);
  // Derive a session key from the password
  m_sessionKey = deriveKeyFromPassword(password, salt);

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

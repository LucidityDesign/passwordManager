#include "vaultmanager.h"
#include "../utils/fileutils.h"
#include "../crypto/cryptoutils.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPalette>

VaultManager::VaultManager()
{
}

void VaultManager::openVault(const QString &filePath, const QString &password)
{
  // Implementation for opening the vault

  QByteArray decrypted = readEncryptedFile(filePath, password);

  qDebug() << "Input was:" << password << "\n";
  qDebug() << "Decrypted Data is: " << decrypted << "\n";

  m_decrypted = decrypted;
  m_password = password;
  m_filePath = filePath;

  loadEntries(m_decrypted);
}

void VaultManager::closeVault()
{
  // Implementation for closing the vault
  m_decrypted.clear();
  m_entries.clear();
  m_password.clear();
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
  writeEncryptedFile(m_filePath, m_password, doc.toJson());
}

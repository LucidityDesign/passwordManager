#include "vaultmanager.h"
#include "../utils/fileutils.h"
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

  loadEntries(m_decrypted);
}

void VaultManager::closeVault()
{
  // Implementation for closing the vault
  m_decrypted.clear();
  m_entries.clear();
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

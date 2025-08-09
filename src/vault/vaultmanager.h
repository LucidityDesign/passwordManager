#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QString>
#include <QList>

struct VaultEntry
{
  QString username;
  QString password;
};

class VaultManager
{
public:
  VaultManager();
  ~VaultManager();
  void openVault(const QString &filePath, const QString &password);
  // void saveVault(const QString &filePath);
  void addEntry(const VaultEntry &entry);
  void removeEntry(const QString &username);
  void updateEntry(const QString &username, const QString &newPassword);
  QList<VaultEntry> getEntries() const;
  // bool isVaultOpen() const;
  void closeVault();

private:
  QString m_filePath;
  QString m_password;
  QByteArray m_decrypted;
  QList<VaultEntry> m_entries; // List of username-password pairs
  bool m_isVaultOpen = false;
  void loadEntries(QByteArray decryptedData);
  void saveEntries(QList<VaultEntry> entries);
};

#endif // VAULTMANAGER_H

#ifndef VAULTMANAGER_H
#define VAULTMANAGER_H

#include <QString>
#include <QList>
#include <QObject>
#include <QTimerEvent>

struct VaultEntry
{
  QString username;
  QString password;
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
  // bool isVaultOpen() const;
  void closeVault();
  void startSession(const QString &password);
  void extendSession();

private:
  QByteArray m_sessionKey;
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

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
  bool isVaultOpen() const { return m_isVaultOpen; }
  void closeVault();
  void startSession(const QString &password);
  void extendSession();

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

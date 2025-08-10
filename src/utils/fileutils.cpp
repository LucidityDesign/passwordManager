#include "fileutils.h"
#include "../crypto/cryptoutils.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <sodium.h>

namespace FileUtils
{

  // =============================================================================
  // HIGH-LEVEL API IMPLEMENTATION
  // =============================================================================

  bool createVault(const QString &filePath, const QString &password, const QByteArray &data)
  {
    if (password.isEmpty())
    {
      throw CryptoOperationError("Password cannot be empty");
    }

    try
    {
      // Generate new salt for new vault
      QByteArray salt = generateSalt();
      QByteArray key = deriveKeyFromPassword(password, salt);

      // Encrypt the data
      QByteArray nonce, ciphertext;
      if (!encrypt(data, key, ciphertext, nonce))
      {
        throw CryptoOperationError("Failed to encrypt vault data");
      }

      // Write to file
      if (!Detail::writeVaultToFile(filePath, salt, nonce, ciphertext))
      {
        throw FileOperationError("Failed to write vault file: " + filePath.toStdString());
      }

      return true;
    }
    catch (const std::exception &e)
    {
      qWarning() << "createVault failed:" << e.what();
      throw;
    }
  }

  QByteArray readVault(const QString &filePath, const QString &password)
  {
    if (password.isEmpty())
    {
      throw CryptoOperationError("Password cannot be empty");
    }

    if (!exists(filePath))
    {
      throw FileOperationError("Vault file does not exist: " + filePath.toStdString());
    }

    if (!Detail::isValidVaultFile(filePath))
    {
      throw FileOperationError("Invalid vault file format: " + filePath.toStdString());
    }

    try
    {
      QFile file(filePath);
      if (!file.open(QIODevice::ReadOnly))
      {
        throw FileOperationError("Cannot open vault file for reading: " + filePath.toStdString());
      }

      // Read file components
      QByteArray salt = Detail::readSaltFromFile(file);
      QByteArray nonce = file.read(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
      QByteArray ciphertext = file.readAll();
      file.close();

      // Validate components
      if (salt.size() != crypto_pwhash_SALTBYTES)
      {
        throw FileOperationError("Invalid salt size in vault file");
      }
      if (nonce.size() != crypto_aead_xchacha20poly1305_ietf_NPUBBYTES)
      {
        throw FileOperationError("Invalid nonce size in vault file");
      }
      if (ciphertext.isEmpty())
      {
        throw FileOperationError("No encrypted data found in vault file");
      }

      // Decrypt
      QByteArray key = deriveKeyFromPassword(password, salt);
      QByteArray decrypted;
      if (!decrypt(ciphertext, key, nonce, decrypted))
      {
        throw CryptoOperationError("Decryption failed - incorrect password or corrupted file");
      }

      return decrypted;
    }
    catch (const std::exception &e)
    {
      qWarning() << "readVault failed:" << e.what();
      throw;
    }
  }

  bool updateVault(const QString &filePath, const QByteArray &sessionKey, const QByteArray &data)
  {
    if (sessionKey.isEmpty())
    {
      throw CryptoOperationError("Session key cannot be empty");
    }

    if (!exists(filePath))
    {
      throw FileOperationError("Vault file does not exist: " + filePath.toStdString());
    }

    try
    {
      // Get existing salt (must preserve it!)
      QByteArray salt = extractSalt(filePath);
      if (salt.isEmpty())
      {
        throw FileOperationError("Cannot extract salt from existing vault file");
      }

      // Encrypt with session key
      QByteArray nonce, ciphertext;
      if (!encrypt(data, sessionKey, ciphertext, nonce))
      {
        throw CryptoOperationError("Failed to encrypt vault update");
      }

      // Write updated vault
      if (!Detail::writeVaultToFile(filePath, salt, nonce, ciphertext))
      {
        throw FileOperationError("Failed to write updated vault file");
      }

      return true;
    }
    catch (const std::exception &e)
    {
      qWarning() << "updateVault failed:" << e.what();
      throw;
    }
  }

  // =============================================================================
  // LOW-LEVEL API IMPLEMENTATION
  // =============================================================================

  bool exists(const QString &filePath)
  {
    return QFileInfo::exists(filePath);
  }

  QByteArray extractSalt(const QString &filePath)
  {
    if (!exists(filePath))
    {
      qWarning() << "Cannot extract salt: file does not exist:" << filePath;
      return {};
    }

    try
    {
      QFile file(filePath);
      if (!file.open(QIODevice::ReadOnly))
      {
        qWarning() << "Cannot open file to extract salt:" << filePath;
        return {};
      }

      QByteArray salt = Detail::readSaltFromFile(file);
      file.close();
      return salt;
    }
    catch (const std::exception &e)
    {
      qWarning() << "extractSalt failed:" << e.what();
      return {};
    }
  }

  // TODO: move to CryptoUtils
  QByteArray generateSalt()
  {
    QByteArray salt(crypto_pwhash_SALTBYTES, 0);
    randombytes_buf(salt.data(), salt.size());
    return salt;
  }

  // =============================================================================
  // INTERNAL HELPERS IMPLEMENTATION
  // =============================================================================

  namespace Detail
  {

    bool isValidVaultFile(const QString &filePath)
    {
      QFile file(filePath);
      if (!file.open(QIODevice::ReadOnly))
      {
        return false;
      }

      // Check minimum file size
      qint64 minSize = crypto_pwhash_SALTBYTES +
                       crypto_aead_xchacha20poly1305_ietf_NPUBBYTES +
                       crypto_aead_xchacha20poly1305_ietf_ABYTES; // Minimum for empty data

      bool valid = file.size() >= minSize;
      file.close();
      return valid;
    }

    QByteArray readSaltFromFile(QFile &file)
    {
      if (!file.isOpen() || !file.isReadable())
      {
        throw FileOperationError("File is not open for reading");
      }

      // Ensure we're at the beginning
      file.seek(0);

      QByteArray salt = file.read(crypto_pwhash_SALTBYTES);
      if (salt.size() != crypto_pwhash_SALTBYTES)
      {
        throw FileOperationError("Failed to read complete salt from file");
      }

      return salt;
    }

    bool writeVaultToFile(const QString &filePath, const QByteArray &salt,
                          const QByteArray &nonce, const QByteArray &ciphertext)
    {
      // Validate inputs
      if (salt.size() != crypto_pwhash_SALTBYTES)
      {
        qWarning() << "Invalid salt size:" << salt.size() << "expected:" << crypto_pwhash_SALTBYTES;
        return false;
      }

      if (nonce.size() != crypto_aead_xchacha20poly1305_ietf_NPUBBYTES)
      {
        qWarning() << "Invalid nonce size:" << nonce.size() << "expected:" << crypto_aead_xchacha20poly1305_ietf_NPUBBYTES;
        return false;
      }

      if (ciphertext.isEmpty())
      {
        qWarning() << "Ciphertext cannot be empty";
        return false;
      }

      QFile file(filePath);
      if (!file.open(QIODevice::WriteOnly))
      {
        qWarning() << "Cannot open file for writing:" << filePath;
        return false;
      }

      // Write components in order
      if (file.write(salt) != salt.size() ||
          file.write(nonce) != nonce.size() ||
          file.write(ciphertext) != ciphertext.size())
      {
        qWarning() << "Failed to write complete data to file";
        file.close();
        return false;
      }

      file.close();

      // Write to file atomically (write to temp, then rename)
      // QString tempPath = filePath + ".tmp";

      // QFile file(tempPath);
      // if (!file.open(QIODevice::WriteOnly))
      // {
      //   qWarning() << "Cannot create temporary file:" << tempPath;
      //   return false;
      // }

      // // Write components in order
      // if (file.write(salt) != salt.size() ||
      //     file.write(nonce) != nonce.size() ||
      //     file.write(ciphertext) != ciphertext.size())
      // {
      //   qWarning() << "Failed to write complete data to temporary file";
      //   file.close();
      //   QFile::remove(tempPath);
      //   return false;
      // }

      // file.close();

      // // Atomic rename
      // if (!QFile::rename(tempPath, filePath))
      // {
      //   qWarning() << "Failed to rename temporary file to final location";
      //   QFile::remove(tempPath);
      //   return false;
      // }

      return true;
    }

  } // namespace Detail

} // namespace FileUtils

#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QByteArray>
#include <QString>
#include <QFile>
#include <stdexcept>

namespace FileUtils
{

  /**
   * @brief Exception class for file operation errors
   */
  class FileOperationError : public std::runtime_error
  {
  public:
    explicit FileOperationError(const std::string &message)
        : std::runtime_error(message) {}
  };

  // =============================================================================
  // HIGH-LEVEL API (Recommended for most use cases)
  // =============================================================================

  /**
   * @brief Create a new encrypted vault file
   * @param filePath Path to the vault file to create
   * @param password Master password for encryption
   * @param data Initial data (default: empty JSON array)
   * @return true if successful
   * @throws FileOperationError if file creation fails
   * @throws CryptoOperationError if encryption fails
   */
  bool createVault(const QString &filePath, const QString &password,
                   const QByteArray &data = "[]");

  /**
   * @brief Read and decrypt an entire vault file
   * @param filePath Path to the vault file
   * @param password Master password for decryption
   * @return Decrypted data
   * @throws FileOperationError if file reading fails
   * @throws CryptoOperationError if decryption fails (wrong password)
   */
  QByteArray readVault(const QString &filePath, const QString &password);

  /**
   * @brief Update an existing vault file with new data
   * @param filePath Path to the vault file
   * @param sessionKey Pre-derived encryption key
   * @param data New data to encrypt and save
   * @return true if successful
   * @throws FileOperationError if file operations fail
   * @throws CryptoOperationError if encryption fails
   */
  bool updateVault(const QString &filePath, const QByteArray &sessionKey,
                   const QByteArray &data);

  // =============================================================================
  // LOW-LEVEL API (For advanced use cases)
  // =============================================================================

  /**
   * @brief Check if a file exists
   */
  bool exists(const QString &filePath);

  /**
   * @brief Extract salt from an encrypted vault file
   * @param filePath Path to the vault file
   * @return Salt bytes, or empty array if file doesn't exist/error
   */
  QByteArray extractSalt(const QString &filePath);

  /**
   * @brief Generate a new random salt
   * @return Random salt of appropriate size
   */
  QByteArray generateSalt();

  // =============================================================================
  // INTERNAL HELPERS (Private implementation details)
  // =============================================================================

  namespace Detail
  {
    /**
     * @brief Validate vault file format
     */
    bool isValidVaultFile(const QString &filePath);

    /**
     * @brief Read salt from an open file handle
     */
    QByteArray readSaltFromFile(QFile &file);

    /**
     * @brief Write vault data to file with proper format
     */
    bool writeVaultToFile(const QString &filePath, const QByteArray &salt,
                          const QByteArray &nonce, const QByteArray &ciphertext);
  }

} // namespace FileUtils

#endif // FILEUTILS_H

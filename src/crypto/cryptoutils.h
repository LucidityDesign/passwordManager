#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <QByteArray>
#include <QString>

namespace CryptoUtils
{
  /**
   * @brief Exception class for encryption/decryption errors
   */
  class CryptoOperationError : public std::runtime_error
  {
  public:
    explicit CryptoOperationError(const std::string &message)
        : std::runtime_error(message) {}
  };

  /**
   * @brief Derives a cryptographic key from a password and salt using Argon2
   * @param password The user's password
   * @param salt A unique salt for this operation
   * @return The derived key as QByteArray
   * @throws CryptoOperationError if key derivation fails
   */
  QByteArray deriveKeyFromPassword(const QString &password, const QByteArray &salt);

  /**
   * @brief Encrypts plaintext data using a symmetric key
   * @param plain The plaintext data to encrypt
   * @param key The symmetric key to use for encryption
   * @param outCiphertext The resulting ciphertext
   * @param outNonce The nonce used for encryption
   * @return true if encryption was successful, false otherwise
   * * @throws CryptoOperationError if encryption fails
   */
  bool encrypt(const QByteArray &plain, const QByteArray &key, QByteArray &outCiphertext, QByteArray &outNonce);

  /**
   * @brief Decrypts ciphertext data using a symmetric key
   * @param ciphertext The data to decrypt
   * @param key The symmetric key to use for decryption
   * @param nonce The nonce used for decryption
   * @param outPlain The resulting plaintext
   * @return true if decryption was successful, false otherwise
   * @throws CryptoOperationError if decryption fails
   */
  bool decrypt(const QByteArray &ciphertext, const QByteArray &key, const QByteArray &nonce, QByteArray &outPlain);
}

#endif // CRYPTOUTILS_H

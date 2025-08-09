#include "fileutils.h"
#include "../crypto/cryptoutils.h"
#include <QFile>
#include <sodium.h>

bool writeEncryptedFile(const QString &filePath, const QString &password, const QByteArray &data)
{
  QByteArray salt(crypto_pwhash_SALTBYTES, 0);
  randombytes_buf(salt.data(), salt.size());

  QByteArray plaintext = data; // Use the provided data

  QByteArray key = deriveKeyFromPassword(password, salt);

  QByteArray nonce, ciphertext;
  if (!encrypt(plaintext, key, ciphertext, nonce))
    return false;

  QFile file(filePath);
  if (!file.open(QIODevice::WriteOnly))
    return false;

  file.write(salt);
  file.write(nonce);
  file.write(ciphertext);
  file.close();
  return true;
}

QByteArray readEncryptedFile(const QString &filePath, const QString &password)
{
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly))
    return {};

  QByteArray salt = file.read(crypto_pwhash_SALTBYTES);
  QByteArray nonce = file.read(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
  QByteArray ciphertext = file.readAll();
  file.close();

  QByteArray key = deriveKeyFromPassword(password, salt);
  QByteArray decrypted;
  if (!decrypt(ciphertext, key, nonce, decrypted))
    throw "wrong password";

  return decrypted;
}

bool fileExists(const QString &filePath)
{
  QFile file(filePath);
  return file.exists();
}

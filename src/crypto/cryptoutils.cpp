#include "cryptoutils.h"
#include <sodium.h>
#include <QDebug>

namespace CryptoUtils
{
  QByteArray deriveKeyFromPassword(const QString &password, const QByteArray &salt)
  {
    QByteArray key(crypto_aead_xchacha20poly1305_ietf_KEYBYTES, 0);
    if (crypto_pwhash(
            reinterpret_cast<unsigned char *>(key.data()), key.size(),
            password.toUtf8().constData(), password.toUtf8().size(),
            reinterpret_cast<const unsigned char *>(salt.constData()),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE,
            crypto_pwhash_ALG_DEFAULT) != 0)
    {
      qWarning() << "Key derivation failed!";
      throw CryptoOperationError("Key derivation failed");
    }
    return key;
  }

  bool encrypt(const QByteArray &plain, const QByteArray &key, QByteArray &outCiphertext, QByteArray &outNonce)
  {
    outNonce.resize(crypto_aead_xchacha20poly1305_ietf_NPUBBYTES);
    randombytes_buf(outNonce.data(), outNonce.size());

    QByteArray ciphertext(plain.size() + crypto_aead_xchacha20poly1305_ietf_ABYTES, 0);

    unsigned long long ciphertext_len;
    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            reinterpret_cast<unsigned char *>(ciphertext.data()), &ciphertext_len,
            reinterpret_cast<const unsigned char *>(plain.data()), plain.size(),
            nullptr, 0, // Additional data (not used)
            nullptr,
            reinterpret_cast<const unsigned char *>(outNonce.data()),
            reinterpret_cast<const unsigned char *>(key.data())) != 0)
    {
      throw CryptoOperationError("Failed to encrypt vault data");
    }

    ciphertext.resize(ciphertext_len);
    outCiphertext = ciphertext;
    return true;
  }

  bool decrypt(const QByteArray &ciphertext, const QByteArray &key, const QByteArray &nonce, QByteArray &outPlain)
  {
    QByteArray decrypted(ciphertext.size() - crypto_aead_xchacha20poly1305_ietf_ABYTES, 0);
    unsigned long long decrypted_len;

    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            reinterpret_cast<unsigned char *>(decrypted.data()), &decrypted_len,
            nullptr,
            reinterpret_cast<const unsigned char *>(ciphertext.data()), ciphertext.size(),
            nullptr, 0,
            reinterpret_cast<const unsigned char *>(nonce.data()),
            reinterpret_cast<const unsigned char *>(key.data())) != 0)
    {
      throw CryptoOperationError("Decryption failed - incorrect password or corrupted file");
    }

    decrypted.resize(decrypted_len);
    outPlain = decrypted;
    return true;
  }
}

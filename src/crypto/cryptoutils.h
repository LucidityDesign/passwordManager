#ifndef CRYPTOUTILS_H
#define CRYPTOUTILS_H

#include <QByteArray>
#include <QString>

// Functions moved from mainwindow.cpp - same signatures and behavior
QByteArray deriveKeyFromPassword(const QString &password, const QByteArray &salt);
bool encrypt(const QByteArray &plain, const QByteArray &key, QByteArray &outCiphertext, QByteArray &outNonce);
bool decrypt(const QByteArray &ciphertext, const QByteArray &key, const QByteArray &nonce, QByteArray &outPlain);

#endif // CRYPTOUTILS_H

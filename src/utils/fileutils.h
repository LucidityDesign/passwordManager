#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QByteArray>
#include <QString>

// Functions moved from mainwindow.cpp - same signatures and behavior
bool writeEncryptedFile(const QString &filePath, const QString &password);
QByteArray readEncryptedFile(const QString &filePath, const QString &password);
bool fileExists(const QString &filePath);

#endif // FILEUTILS_H

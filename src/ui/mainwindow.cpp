#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../utils/fileutils.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
  connect(ui->lineEdit_2, &QLineEdit::returnPressed, this, &MainWindow::onPasswordEntered);

  ui->label->setVisible(false);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onButtonClicked()
{
  MainWindow::onPasswordEntered();
}

void MainWindow::onPasswordEntered()
{

  QString password = ui->lineEdit_2->text();

  ui->label->setVisible(false);

  // writeEncryptedFile("vault.txt", text);

  if (!fileExists("vault.txt"))
  {
    writeEncryptedFile("vault.txt", password);
    ui->label->setText("File does not exist, created a new vault.");
    ui->label->setVisible(true);
    return;
  }

  try
  {
    QByteArray decrypted = readEncryptedFile("vault.txt", password);

    qDebug() << "Input was:" << password << "\n";
    qDebug() << "Decrypted Data is: " << decrypted << "\n";
  }
  catch (...)
  {
    ui->label->setText("Wrong password");
    ui->label->setVisible(true);
    return;
  }
}

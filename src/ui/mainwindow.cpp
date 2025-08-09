#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../utils/fileutils.h"
#include "stackedwidget.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::onButtonClicked);
  connect(ui->lineEdit_2, &QLineEdit::returnPressed, this, &MainWindow::onPasswordEntered);

  ui->label->setVisible(false);

  QPushButton *lockButton = new QPushButton("Lock");
  ui->statusbar->addWidget(lockButton);

  connect(lockButton, &QPushButton::clicked, this, &MainWindow::lockVault);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::onButtonClicked()
{
  MainWindow::onPasswordEntered();
}

void MainWindow::openPasswordlist()
{
  // Create the widget first
  StackedWidget *passwordWidget = new StackedWidget(this);

  // Pass the VaultManager instance to the child widget
  passwordWidget->setVaultManager(&m_vaultManager);

  // Add to stack and switch
  ui->stackedWidget->insertWidget(1, passwordWidget);
  ui->stackedWidget->setCurrentIndex(1);
}

void MainWindow::onPasswordEntered()
{

  QString password = ui->lineEdit_2->text();

  ui->lineEdit_2->clear();

  ui->label->setVisible(false);

  // writeEncryptedFile("vault.txt", text);

  try
  {
    m_vaultManager.openVault("vault.txt", password);

    openPasswordlist();
  }
  catch (...)
  {
    ui->label->setText("Wrong password");
    ui->label->setVisible(true);
    return;
  }
}

void MainWindow::lockVault()
{
  ui->stackedWidget->setCurrentIndex(0);
  m_vaultManager.closeVault();
}

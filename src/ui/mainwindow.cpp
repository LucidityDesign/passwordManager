#include "mainwindow.h"
#include "ui_mainwindow.h"
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

  // ✅ Connect VaultManager signals to MainWindow slots
  connect(&m_vaultManager, &VaultManager::vaultOpened, this, &MainWindow::onVaultOpened);
  connect(&m_vaultManager, &VaultManager::vaultClosed, this, &MainWindow::onVaultClosed);
  connect(&m_vaultManager, &VaultManager::entryAdded, this, &MainWindow::onEntryAdded);
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
  m_vaultManager.closeVault(); // This will emit vaultClosed("manual")
}

// ============================================================================
// VAULT MANAGER EVENT HANDLERS
// ============================================================================

void MainWindow::onVaultOpened(const QString &filePath)
{
  qDebug() << "Vault opened successfully:" << filePath;
}

void MainWindow::onVaultClosed(const QString &reason)
{
  qDebug() << "Vault closed, reason:" << reason;

  // Update UI to show vault is closed
  setWindowTitle("Password Manager - Locked");

  // Switch back to login screen
  ui->stackedWidget->setCurrentIndex(0);
}

void MainWindow::onEntryAdded(const VaultEntry &entry)
{
  qDebug() << "New entry added:" << entry.username;

  // You could show a notification, update counters, etc.
  // The password list will auto-refresh through other mechanisms
}

#include "stackedwidget.h"
#include "ui_stackedwidget.h"
#include "newlogindialog.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPalette>
#include <QPushButton>
#include <QMessageBox>
#include <QTimer>
#include <QHBoxLayout>
#include <QWidget>
#include <QApplication>
#include <QClipboard>

StackedWidget::StackedWidget(QWidget *parent)
    : QStackedWidget(parent), ui(new Ui::StackedWidget)
{
    ui->setupUi(this);

    connect(ui->addLoginButton, &QPushButton::clicked, this, &StackedWidget::openNewPasswordDialog);
}

StackedWidget::~StackedWidget()
{
    delete ui;
}

void StackedWidget::setVaultManager(VaultManager *vaultManager)
{
    m_vaultManager = vaultManager;
    populatePasswordList();
}

void StackedWidget::populatePasswordList()
{
    // Safety check: ensure we have a valid VaultManager
    if (!m_vaultManager)
    {
        qDebug() << "No vault manager or vault not open";
        ui->tableWidget->clear();
        ui->tableWidget->setRowCount(0);
        return;
    }

    auto entries = m_vaultManager->getEntries(); // Use -> for pointer access

    ui->tableWidget->clear();        // Clear existing items
    ui->tableWidget->setRowCount(0); // Reset row count

    // Set up table headers for better UX
    ui->tableWidget->setColumnCount(3);
    QStringList headers;
    headers << "Username" << "Password" << "Actions";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    for (int i = 0; i < entries.size(); ++i)
    {
        const VaultEntry &entry = entries[i];
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        // Set username
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(entry.username));

        // For security: Show masked password initially - only decrypt on user interaction
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("••••••••"));

        // Add a button to reveal/copy password securely
        QPushButton *revealButton = new QPushButton("Reveal", this);
        revealButton->setProperty("username", entry.username); // Store username for retrieval

        // Add copy button for secure clipboard operations
        QPushButton *copyButton = new QPushButton("Copy", this);
        copyButton->setProperty("username", entry.username);

        // Create a widget to hold both buttons
        QWidget *buttonWidget = new QWidget();
        QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
        buttonLayout->addWidget(revealButton);
        buttonLayout->addWidget(copyButton);
        buttonLayout->setContentsMargins(0, 0, 0, 0);

        // Connect reveal button
        connect(revealButton, &QPushButton::clicked, this, [this, revealButton]()
                {
            QString username = revealButton->property("username").toString();
            revealPasswordSecurely(username, revealButton); });

        // Connect copy button for secure clipboard copy
        connect(copyButton, &QPushButton::clicked, this, [this, copyButton]()
                {
            QString username = copyButton->property("username").toString();
            copyPasswordToClipboard(username); });

        ui->tableWidget->setCellWidget(row, 2, buttonWidget);
    }

    ui->tableWidget->resizeColumnsToContents(); // Adjust column widths
}

void StackedWidget::revealPasswordSecurely(const QString &username, QPushButton *button)
{
    if (!m_vaultManager)
    {
        qWarning() << "No vault manager available";
        return;
    }

    try
    {
        // Get the password securely (this also extends the session)
        QString password = m_vaultManager->getPasswordSecure(username);

        if (password.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Failed to decrypt password or entry not found");
            return;
        }

        // Find the row for this username to update the password cell
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            QTableWidgetItem *usernameItem = ui->tableWidget->item(row, 0);
            if (usernameItem && usernameItem->text() == username)
            {

                // Show password temporarily
                ui->tableWidget->setItem(row, 1, new QTableWidgetItem(password));

                // Change button to "Hide" and update its function
                button->setText("Hide");
                button->disconnect(); // Remove old connections

                connect(button, &QPushButton::clicked, this, [this, username, button, row]()
                        {
                    // Hide password again
                    ui->tableWidget->setItem(row, 1, new QTableWidgetItem("••••••••"));
                    button->setText("Reveal");
                    button->disconnect();

                    // Reconnect reveal function
                    connect(button, &QPushButton::clicked, this, [this, username, button]() {
                        revealPasswordSecurely(username, button);
                    }); });

                // Optional: Auto-hide password after a timeout for additional security
                QTimer::singleShot(30000, this, [this, username, button, row]() { // 30 seconds
                    if (button->text() == "Hide")
                    {
                        ui->tableWidget->setItem(row, 1, new QTableWidgetItem("••••••••"));
                        button->setText("Reveal");
                        button->disconnect();

                        connect(button, &QPushButton::clicked, this, [this, username, button]()
                                { revealPasswordSecurely(username, button); });
                    }
                });

                break;
            }
        }

        // Securely clear the password from memory
        password.fill(QChar(0));
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Decryption Error",
                              QString("Failed to decrypt password: %1").arg(e.what()));
    }
}

void StackedWidget::openNewPasswordDialog()
{
    // void
    auto newLoginDialog = new NewLoginDialog(this);

    auto vaultManager = &m_vaultManager;

    connect(newLoginDialog, &QDialog::accepted, this, [this, newLoginDialog, vaultManager]()
            {
                qDebug() << "data";

                QString password = newLoginDialog->getPassword();
                QString username = newLoginDialog->getUsername();

                qDebug() << username << "\n";
                qDebug() << password << "\n";

                VaultEntry entry = {username, password};

                (*vaultManager)->addEntry(entry);

                this->populatePasswordList(); });

    newLoginDialog->exec();
}

void StackedWidget::copyPasswordToClipboard(const QString &username)
{
    if (!m_vaultManager)
    {
        qWarning() << "No vault manager available";
        return;
    }

    try
    {
        // Get the password securely
        QString password = m_vaultManager->getPasswordSecure(username);

        if (password.isEmpty())
        {
            QMessageBox::warning(this, "Error", "Failed to decrypt password or entry not found");
            return;
        }

        // Copy to clipboard
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(password);

        // Show confirmation using debug output instead of blocking dialog
        qDebug() << QString("Password for '%1' copied to clipboard. Clipboard will be cleared in 30 seconds for security.").arg(username);

        // Store the password for comparison (make a copy for the lambda)
        QString passwordCopy = password;

        // Clear clipboard after 30 seconds for security
        QTimer::singleShot(30000, this, [clipboard, passwordCopy]()
                           {
            // Only clear if our password is still in clipboard
            if (clipboard->text() == passwordCopy) {
                clipboard->clear();
                qDebug() << "Clipboard cleared for security";
            } });

        // Securely clear the password from memory
        password.fill(QChar(0));
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(this, "Decryption Error",
                              QString("Failed to decrypt password: %1").arg(e.what()));
    }
}

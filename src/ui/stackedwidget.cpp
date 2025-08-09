#include "stackedwidget.h"
#include "ui_stackedwidget.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPalette>

StackedWidget::StackedWidget(QWidget *parent)
    : QStackedWidget(parent), ui(new Ui::StackedWidget)
{
    ui->setupUi(this);

    // connect(ui, )
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
    // Add your logic here to parse the decrypted data and populate UI elements

    ui->tableWidget->clear();        // Clear existing items
    ui->tableWidget->setRowCount(0); // Reset row count

    for (const VaultEntry &entry : entries)
    {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(entry.username));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(entry.password));
    }

    ui->tableWidget->resizeColumnsToContents(); // Adjust column widths
}

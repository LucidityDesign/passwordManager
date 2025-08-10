#include "newlogindialog.h"
#include "ui_newlogindialog.h"
#include "../crypto/cryptoutils.h"

NewLoginDialog::NewLoginDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::NewLoginDialog)
{
    ui->setupUi(this);

    connect(ui->newPasswordButton, &QPushButton::clicked, this, &NewLoginDialog::generatePassword);
}

NewLoginDialog::~NewLoginDialog()
{
    delete ui;
}

QString NewLoginDialog::getUsername()
{
    return ui->lineEditUsername->text();
}

QString NewLoginDialog::getPassword()
{
    return ui->lineEditPassword->text();
}

void NewLoginDialog::generatePassword()
{
    QString newPassword = CryptoUtils::generateRandomPassword();
    ui->lineEditPassword->setText(newPassword);
}

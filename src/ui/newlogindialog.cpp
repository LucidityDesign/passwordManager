#include "newlogindialog.h"
#include "ui_newlogindialog.h"

NewLoginDialog::NewLoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewLoginDialog)
{
    ui->setupUi(this);
}

NewLoginDialog::~NewLoginDialog()
{
    delete ui;
}

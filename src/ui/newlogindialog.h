#ifndef NEWLOGINDIALOG_H
#define NEWLOGINDIALOG_H

#include <QDialog>

namespace Ui {
class NewLoginDialog;
}

class NewLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewLoginDialog(QWidget *parent = nullptr);
    ~NewLoginDialog();

private:
    Ui::NewLoginDialog *ui;
};

#endif // NEWLOGINDIALOG_H

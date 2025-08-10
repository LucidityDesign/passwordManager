#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>
#include <QPushButton>
#include "../vault/vaultmanager.h"

namespace Ui
{
    class StackedWidget;
}

class StackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    explicit StackedWidget(QWidget *parent = nullptr);
    ~StackedWidget();

    // Better approach: setter method
    void setVaultManager(VaultManager *vaultManager);

private:
    Ui::StackedWidget *ui;
    VaultManager *m_vaultManager;

    void populatePasswordList();
    void openNewPasswordDialog();

    // Secure password reveal method
    void revealPasswordSecurely(const QString &username, QPushButton *button);

    // Secure clipboard copy method
    void copyPasswordToClipboard(const QString &username);
};

#endif // STACKEDWIDGET_H

#ifndef STACKEDWIDGET_H
#define STACKEDWIDGET_H

#include <QStackedWidget>
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
};

#endif // STACKEDWIDGET_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "../vault/vaultmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    VaultManager m_vaultManager;
    // StackedWidget *stackedWidget;

private slots:
    void onButtonClicked();
    void onPasswordEntered();
    void openPasswordlist();
    void lockVault();
};
#endif // MAINWINDOW_H

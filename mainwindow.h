#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "accountsstore.h"
#include "sync.h"
#include "settings.h"
#include "accountsstandarditemmodel.h"

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    void on_actionCreate_triggered();
    void on_actionEdit_triggered();
    void on_actionDelete_triggered();
    void on_actionCopy_Password_triggered();
    void on_actionSynchornize_triggered();
    void on_actionPreferences_triggered();
    void on_actionLaunch_Browser_triggered();
    void on_actionCopy_Old_Password_2_triggered();

    void on_accountsListView_activated(const QModelIndex &index);



private:
    Ui::MainWindow *ui;

    AccountsStore *store;
    Account *currentAccount;
    AccountsStandardItemModel *accountsModel;
    AccountsSortProxy *sortProxy;

    QString syncMsg;
    bool syncRunning;

    QPoint getContextPosition();
    void showContextMenu(QPoint);
    void sort();
    void setCurrentAccount();

    void disableBeforeSync();
    void enableAfterSync();

    void displayError(const QString &title, const QString &msg);

    void signal_handler(int);
};

#endif // MAINWINDOW_H

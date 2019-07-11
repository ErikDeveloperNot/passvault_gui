#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createaccountdialog.h"
#include "getkeydialog.h"
#include "preferencesdialog.h"

#include "statusdefs.h"
#include "opensslaesengine.h"
#include "settings.h"
#include "utils.h"
#include "sync.h"
#include "rescuedialog.h"

#include <csignal>

#include <QDebug>
#include <QInputDialog>
#include <QList>
#include <QClipboard>
#include <QMessageBox>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    store = new AccountsStore{};
    Settings settings = store->getSettings();
    std::string key{""};

    if (settings.general.saveKey) {
        std::string encrypted = settings.general.key.toStdString();
        key = store->getEncryptionEngine()->decryptPassword(encrypted);
    } else {
        GetKeyDialog dialog{};
        dialog.exec();
        key = dialog.getKey().toStdString();
        qDebug() << "key: " << QString::fromStdString(key);

        if (key.length() < 1)
            exit(1);
    }

    store->setEncryptionKey(key);

    accountsModel = new AccountsStandardItemModel{store, this};
    sortProxy = new AccountsSortProxy{accountsModel, store};
    sortProxy->setSourceModel(accountsModel);
    sortProxy->setDynamicSortFilter(false);
    ui->accountsListView->setModel(sortProxy);

    setWindowTitle("Passvault");

    QMenu *helpMenu = ui->menuBar->addMenu(tr("&Help"));
    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

    // call back for sync calls
    connect(store, &AccountsStore::readyMessage, [=](QString msg) {
        qDebug() << "\n\nMessage received in MainWindow: " << msg << ", sync running: " << syncRunning;

        if (syncRunning) {
            syncMsg = msg;
            syncRunning = false;
        }

    });

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (ui->accountsListView->isActiveWindow()) {
        if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        {
            showContextMenu(getContextPosition());
        }
    }
}


void MainWindow::on_accountsListView_activated(const QModelIndex &)
{
//    setCurrentAccount();
    showContextMenu(getContextPosition());
}


void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
//    qDebug() << "its a context menu event, may not use this, pos: " << event->globalPos();
//    setCurrentAccount();

    showContextMenu(event->globalPos());
}


QPoint MainWindow::getContextPosition()
{
    /*
     * Revisit to see how to get specific cell location
     */
    QSize frame = ui->accountsListView->frameSize();
    QPoint center{ui->accountsListView->pos()};
    center.setX(center.x() / 2 + frame.width()/2);
    center.setY(center.y() / 2 + frame.height()/2);
    return (mapToGlobal(center));
}


void MainWindow::showContextMenu(QPoint point)
{
    setCurrentAccount();

    if (currentAccount->getCorrectKey()) {
        QMenu menu(this);
        menu.addAction(ui->actionLaunch_Browser);
        menu.addAction(ui->actionCopy_Password);
        menu.addAction(ui->actionCopy_Old_Password_2);
        menu.addSeparator();
        menu.addAction(ui->actionEdit);
        menu.addSeparator();
        menu.addAction(ui->actionDelete);
        menu.setDefaultAction(ui->actionLaunch_Browser);
        menu.exec(point);
    } else {
        QMenu menu(this);

        menu.addAction(QIcon{"://icons8-medical-bag-50.png"}, "Rescue", [&]() {
            qDebug() << "Will rescue dialog for: " << currentAccount->getAccountName();
            RescueDialog dialog{currentAccount, this};
            int result = dialog.exec();

            if (result) {
                try {
                    QString plainTxt = dialog.getNewPassword();
                    QString encTxt =
                      QString::fromStdString(store->getEncryptionEngine()->encryptPassword(plainTxt.toStdString()));

                    currentAccount->setPassword(encTxt);
                    currentAccount->setOldPassword(encTxt);
                    currentAccount->setCorrectKey(true);
                    store->storeAccounts();
                } catch(EncryptionException) {
                    this->displayError("Encryption Error", "An impossible error has been thrown");
                }
            }
        });

        menu.addSeparator();
        menu.addAction(ui->actionDelete);
        menu.exec(point);
    }
}



/*
 * Slot Implementations
 */
void MainWindow::on_actionCreate_triggered()
{
    CreateAccountDialog diag{store->getEncryptionEngine(), store->getSettings().generator};
    QString statusMessage{};

    try {
        if (diag.exec()) {
            qDebug() << "getting new account";
            Account *new_account = diag.getNewAccount();
            qDebug() << "Account name: " << new_account->getAccountName();
            StatusDefs::Account_Status status = store->addAccount(new_account);

            if (status == StatusDefs::success) {
                accountsModel->add(new_account);
                sort();
            }

            statusMessage = "Create status: " +
                                      QString::fromStdString(StatusDefs::get_Account_Status(status));
        }
    } catch (EncryptionException &ex) {
        statusMessage = "Create status: " + QString{ex.what()};
    }

    ui->statusBar->showMessage(statusMessage, 10000);
}


void MainWindow::on_actionEdit_triggered()
{
    setCurrentAccount();
    CreateAccountDialog diag{store->getEncryptionEngine(), store->getSettings().generator, currentAccount};


    QString statusMessage{};

    try {
        if (diag.exec()) {
            qDebug() << "getting new account";
            Account *new_account = diag.getNewAccount();
            qDebug() << "Account name: " << new_account->getAccountName();
            StatusDefs::Account_Status status = store->updateAccount(new_account);

            statusMessage = "Edit status: " +
                                          QString::fromStdString(StatusDefs::get_Account_Status(status));
        }
    } catch (EncryptionException &ex) {
        statusMessage = "Create status: " + QString{ex.what()};
    }

    ui->statusBar->showMessage(statusMessage, 10000);
}


void MainWindow::on_actionDelete_triggered()
{
    setCurrentAccount();
    QMessageBox::StandardButton val = QMessageBox::question(this,
                         "Confirm Account Deletion", "Delete Account: " + currentAccount->getAccountName() + "?");

    if (val == QMessageBox::Yes) {
        accountsModel->remove(ui->accountsListView->currentIndex().row());
        store->deleteAccount(currentAccount);
    }
}


void MainWindow::on_actionCopy_Password_triggered()
{
    setCurrentAccount();
    QClipboard *clip = QGuiApplication::clipboard();
    QString pWord;

    try {
        pWord = QString::fromStdString(store->getPassword(currentAccount));
    } catch (EncryptionException) {
        displayError("Decryption Error", "There was an error decrypting the password");
    }

    clip->setText(pWord);

    if (store->getSettings().general.sortMRU)
        sort();
}


void MainWindow::on_actionLaunch_Browser_triggered()
{
    setCurrentAccount();
    on_actionCopy_Password_triggered();
    Utils::launchBrowserForAccount(currentAccount);
}


void MainWindow::on_actionCopy_Old_Password_2_triggered()
{
    setCurrentAccount();
    QClipboard *clip = QGuiApplication::clipboard();
    QString pWord;

    try {
        pWord = QString::fromStdString(store->getOldPassword(currentAccount));
    } catch (EncryptionException) {
        displayError("Decryption Error", "There was an error decrypting the password");
    }

    clip->setText(pWord);
}


void MainWindow::on_actionSynchornize_triggered()
{
    disableBeforeSync();
    syncMsg = "";
    syncRunning = true;

    QTimer *timer = new QTimer{this};
    connect(timer, &QTimer::timeout, [=]() {
        if (!syncRunning) {
//            qDebug() << "****: " << syncMsg;
            this->ui->statusBar->clearMessage();
            this->ui->statusBar->showMessage(syncMsg, 10000);

            timer->stop();
            timer->disconnect();
            delete timer;
            this->enableAfterSync();
        }
    });
    timer->start(100);

    ui->statusBar->clearMessage();
    ui->statusBar->showMessage("Syncing accounts...");
    store->syncAccounts();
}


void MainWindow::on_actionPreferences_triggered()
{
    bool sorting = store->getSettings().general.sortMRU;
    PreferencesDialog *diag = new PreferencesDialog{store, this};
    diag->exec();
    delete diag;

    if (sorting != store->getSettings().general.sortMRU)
        sort();
}


void MainWindow::sort()
{
    sortProxy->sort(0);
//    accountsModel->sort(0);
}


void MainWindow::setCurrentAccount()
{
//    currentAccount = static_cast<Account*>(accountsModel->item(ui->accountsListView->currentIndex().row()));
    currentAccount =
        static_cast<Account*>(accountsModel->item(sortProxy->mapToSource(ui->accountsListView->currentIndex()).row()));
}

/*
 * Disable syncing, pref changes, and account create/edit/del. keep password copy and url launch
 */
void MainWindow::disableBeforeSync()
{
    ui->actionEdit->setEnabled(false);
    ui->actionCreate->setEnabled(false);
    ui->actionDelete->setEnabled(false);
    ui->actionSynchornize->setEnabled(false);
}

/*
 * Enable all disable from disable before sync
 */
void MainWindow::enableAfterSync()
{
    ui->actionEdit->setEnabled(true);
    ui->actionCreate->setEnabled(true);
    ui->actionDelete->setEnabled(true);
    ui->actionSynchornize->setEnabled(true);
}


void MainWindow::displayError(const QString &title, const QString &msg)
{
    QMessageBox::critical(this, title, msg);
}














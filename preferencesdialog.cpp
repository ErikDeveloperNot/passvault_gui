#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QMovie>
#include <QMessageBox>
#include <QTimer>



PreferencesDialog::PreferencesDialog(AccountsStore *_store, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog),
    store{_store},
    navigationEnabled{true}
{
    ui->setupUi(this);

    setWindowTitle("Preferences");
    ui->tabWidget->setTabText(0, "General");
//    ui->tabWidget->setTabText(1, "Generator");

    generatorDiag = new GeneratorOptionsDialog{store->getSettings().generator, this};
    generatorDiag->hideButtons();
    connect(generatorDiag, &QDialog::finished, [&]() {
        prefsChanged.generator = true;
        store->getSettings().generator = generatorDiag->getGenerator();
        this->accept();
    });
    ui->tabWidget->insertTab(1, generatorDiag, "Generator");

    ui->tabWidget->setTabText(2, "Sync");
    ui->tabWidget->setCurrentIndex(0);
//    currentTab = 0;

    Settings s = store->getSettings();

    //setup general tab
    if (s.general.saveKey) {
        ui->saveKeyCheckBox->setCheckState(Qt::CheckState::Checked);
        OpenSSLAESEngine e{};

        try {
            QString pw = QString::fromStdString(e.decryptPassword(s.general.key.toStdString()));
            ui->saveKeyLineEdit->setText(pw);
        } catch (EncryptionException &ex) {
            /*
             * TODO
             */
        }

    } else {
        ui->saveKeyCheckBox->setCheckState(Qt::CheckState::Unchecked);
        ui->saveKeyLineEdit->setText("");
        ui->saveKeyLineEdit->setEnabled(false);
    }

    if (s.general.sortMRU)
        ui->sortMRUCheckBox->setCheckState(Qt::CheckState::Checked);
    else
        ui->sortMRUCheckBox->setCheckState(Qt::CheckState::Unchecked);

    ui->databaseLocationLabel->setText(store->getStoreLocation());

    if (s.database.purge) {
        ui->purgeCheckBox->setCheckState(Qt::CheckState::Checked);
        ui->purgeSpinBox->setValue(s.database.numberOfDaysBeforePurge);
    } else {
        ui->purgeCheckBox->setCheckState(Qt::CheckState::Unchecked);
        ui->purgeSpinBox->setValue(0);
        ui->purgeSpinBox->setEnabled(false);
    }

    //generator tab is setup from the generator dialog using the generator from settings


    //setup sync tab
    if (s.sync.remote.userName == "") {
        ui->syncStackedWidget->setCurrentIndex(1);
    } else {
        ui->syncStackedWidget->setCurrentIndex(0);
        ui->syncEmailExistingLineEdit->setText(s.sync.remote.userName);
        ui->syncEmailExistingLineEdit->setEnabled(false);
        ui->syncPassExistingLineEdit->setText(s.sync.remote.password);
        ui->syncPassExistingLineEdit->setEnabled(false);
    }

    prefsChanged.sync=false;
    prefsChanged.saveKey=false;
    prefsChanged.sortMRU=false;
    prefsChanged.generator=false;
    prefsChanged.databasePurge=false;
    prefsChanged.databaseLocation=false;

    storeConnection = connect(store, &AccountsStore::readyMessage, [&](QString msg) {
        syncMsg = msg;
        syncComplete = true;
    });
}


PreferencesDialog::~PreferencesDialog()
{
    qDebug() << "Checking to save preferences";

    if (prefsChanged.sortMRU) {
        // sorting
        if (ui->sortMRUCheckBox->checkState() == Qt::CheckState::Checked)
            store->getSettings().general.sortMRU = true;
        else
            store->getSettings().general.sortMRU = false;
    }

    if (prefsChanged.saveKey) {
        // save key
        if (ui->saveKeyCheckBox->checkState() == Qt::CheckState::Checked) {
            store->getSettings().general.saveKey = true;
            QString key = ui->saveKeyLineEdit->text();
            store->setEncryptionKey(key.toStdString());
            OpenSSLAESEngine e{};

            try {
                store->getSettings().general.key = QString::fromStdString(e.encryptPassword(key.toStdString()));
            } catch (EncryptionException &ex) {
                qDebug() << "need to setup a global alert";
            }
        } else {
            store->getSettings().general.saveKey = false;
            store->getSettings().general.key = "";
        }
    }

    if (prefsChanged.databasePurge) {
        // database purge
        if (ui->purgeCheckBox->isChecked()) {
            store->getSettings().database.purge = true;
            store->getSettings().database.numberOfDaysBeforePurge = ui->purgeSpinBox->value();
        } else {
            store->getSettings().database.purge = false;
            store->getSettings().database.numberOfDaysBeforePurge = 0;
        }
    }

    if (prefsChanged.databaseLocation) {
        store->setStoreLocation(ui->databaseLocationLabel->text());
    }

    if (prefsChanged.sync || prefsChanged.saveKey || prefsChanged.sortMRU || prefsChanged.generator ||
            prefsChanged.databasePurge || prefsChanged.databaseLocation) {
        qDebug() << "Saving Settings";

        if (prefsChanged.databaseLocation) {
            store->storeAccounts();
        }

        store->storeSettings();
    }

    generatorDiag->disconnect();
    delete  generatorDiag;
    disconnect(storeConnection);
    this->disconnect();
    delete ui;
}


void PreferencesDialog::showBusy(QLabel *label)
{
    label->clear();
    label->show();

    file = new QFile{":/busy.gif"};

    if (file->open(QIODevice::ReadOnly)) {
        busyMovie = new QMovie(file);
//        delete f;
    } else {
        qDebug() << "This should neve happen";
    }


    label->setMovie(busyMovie);
    busyMovie->start();
}


void PreferencesDialog::startTimer()
{
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
//        qDebug() << "Timer: , " << timer->isActive();

        if (syncComplete) {
            timer->stop();

            if (busyMovie->state() == QMovie::MovieState::Running) {
                busyMovie->stop();

                delete file;
                delete busyMovie;
            }

            ui->busyLabel->clear();
            ui->busyLabel2->clear();

            if (syncMsg.contains("Sync Server Error")) {
                ui->busyLabel->setText(syncMsg);
                ui->busyLabel2->setText(syncMsg);
            } else {
                ui->syncEmailLineEdit->clear();
                ui->syncPasswordLineEdit->clear();
            }

            syncMsg = "";
            syncComplete = false;
            this->enableNavigation();

            timer->disconnect();
            delete timer;
        }
    });
    timer->start(100);

}


void PreferencesDialog::enableNavigation()
{
    ui->tabWidget->setTabEnabled(0, true);
    ui->tabWidget->setTabEnabled(1, true);

    Settings settings = store->getSettings();

    if (settings.sync.remote.userName == "") {
        ui->syncStackedWidget->setCurrentIndex(1);
        ui->syncEmailLineEdit->setEnabled(true);
//        ui->syncEmailLineEdit->clear();
        ui->syncPasswordLineEdit->setEnabled(true);
//        ui->syncPasswordLineEdit->clear();
        ui->syncCreatePushButton->setEnabled(true);
        ui->syncConfigPushButton->setEnabled(true);
    } else {
        ui->syncStackedWidget->setCurrentIndex(0);
        ui->syncEmailExistingLineEdit->setText(settings.sync.remote.userName);
        ui->syncEmailExistingLineEdit->setEnabled(false);
        ui->syncPassExistingLineEdit->setText(settings.sync.remote.password);
        ui->syncPassExistingLineEdit->setEnabled(false);
        ui->syncDeletePushButton->setEnabled(true);
        ui->syncRemovePushButton->setEnabled(true);
    }

    navigationEnabled = true;
}


void PreferencesDialog::disableNavigation()
{
    ui->tabWidget->setTabEnabled(0, false);
    ui->tabWidget->setTabEnabled(1, false);
    navigationEnabled = false;
}



/*
 * Slots for general tab
 */
void PreferencesDialog::on_changeLocationPushButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Passvault Store Directory"),
                  store->getStoreLocation(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.length() > 0) {
        ui->databaseLocationLabel->setText(dir + "/passvault_store.json");
        prefsChanged.databaseLocation = true;
    }
}

void PreferencesDialog::on_saveKeyCheckBox_stateChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        ui->saveKeyLineEdit->setEnabled(true);
    } else {
        ui->saveKeyLineEdit->setEnabled(false);
        ui->saveKeyLineEdit->setText("");
    }

    prefsChanged.saveKey = true;
}

void PreferencesDialog::on_purgeCheckBox_stateChanged(int state)
{
    if (state == Qt::CheckState::Checked) {
        ui->purgeSpinBox->setEnabled(true);
    } else {
        ui->purgeSpinBox->setEnabled(false);
        ui->purgeSpinBox->setValue(0);
    }

    prefsChanged.databasePurge = true;
}

void PreferencesDialog::on_sortMRUCheckBox_stateChanged(int)
{
    prefsChanged.sortMRU = true;
}



/*
 * Slots for sync tab
 */
void PreferencesDialog::on_syncConfigPushButton_clicked()
{
    registerCreateSync(false);
}

void PreferencesDialog::on_syncCreatePushButton_clicked()
{
    registerCreateSync(true);
}

void PreferencesDialog::registerCreateSync(bool create)
{
    QString user = ui->syncEmailLineEdit->text();
    QString pass = ui->syncPasswordLineEdit->text();

    ui->syncCreatePushButton->setEnabled(false);
    ui->syncConfigPushButton->setEnabled(false);
    ui->syncEmailLineEdit->setEnabled(false);
    ui->syncPasswordLineEdit->setEnabled(false);

    disableNavigation();
    showBusy(ui->busyLabel);

    startTimer();
    store->registerSync(user, pass, create);
}

void PreferencesDialog::on_syncDeletePushButton_clicked()
{
    QString user = ui->syncEmailExistingLineEdit->text();
    QString pass = ui->syncPassExistingLineEdit->text();
    ui->syncDeletePushButton->setEnabled(false);
    ui->syncRemovePushButton->setEnabled(false);

    disableNavigation();
    showBusy(ui->busyLabel2);
    startTimer();
    store->deleteRemoveSync(user, pass, false);
}

void PreferencesDialog::on_syncRemovePushButton_clicked()
{
    QString user = ui->syncEmailExistingLineEdit->text();
    QString pass = ui->syncPassExistingLineEdit->text();
    ui->syncDeletePushButton->setEnabled(false);
    ui->syncRemovePushButton->setEnabled(false);
    store->deleteRemoveSync(user, pass, true);
    ui->syncEmailExistingLineEdit->clear();
    ui->syncPassExistingLineEdit->clear();
    enableNavigation();
}





void PreferencesDialog::reject()
{
//    qDebug() << "REJECT GOT CALLEd";

    if (navigationEnabled)
        QDialog::reject();
}



/*
 * tab changed slot
 */

void PreferencesDialog::on_tabWidget_currentChanged(int index)
{
    ui->busyLabel->clear();
    ui->busyLabel2->clear();

    if (index == 1)
        prefsChanged.generator = true;      //better than always saving
}

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "accountsstore.h"
#include "generatoroptionsdialog.h"

#include <QDialog>
#include <QLabel>
#include <QMovie>



struct PrefsChanged {
    bool sortMRU{false};
    bool saveKey{false};
    bool databasePurge{false};
    bool databaseLocation{false};
    bool generator{false};
    bool sync{false};
};


namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(AccountsStore *, QWidget *parent = nullptr);
    ~PreferencesDialog();

private slots:
    void on_changeLocationPushButton_clicked();


    void on_saveKeyCheckBox_stateChanged(int arg1);

    void on_purgeCheckBox_stateChanged(int arg1);

    void on_syncConfigPushButton_clicked();

    void on_syncCreatePushButton_clicked();

    void on_syncDeletePushButton_clicked();

    void on_syncRemovePushButton_clicked();

    void on_sortMRUCheckBox_stateChanged(int arg1);

    void reject() override;

    void on_tabWidget_currentChanged(int index);

    void on_purgeSpinBox_valueChanged(int arg1);

private:
    Ui::PreferencesDialog *ui;
    AccountsStore *store;
    GeneratorOptionsDialog *generatorDiag;
    PrefsChanged prefsChanged;
    QMovie *busyMovie;
    QFile *file;
    bool navigationEnabled;
    QMetaObject::Connection storeConnection;

    bool syncComplete;
    QString syncMsg;

    void showBusy(QLabel *label);
    void startTimer();
    void enableNavigation();
    void disableNavigation();

    void registerCreateSync(bool);
};

#endif // PREFERENCESDIALOG_H

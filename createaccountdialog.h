#ifndef CREATEACCOUNTDIALOG_H
#define CREATEACCOUNTDIALOG_H

#include "account.h"
#include "opensslaesengine.h"
#include "passwordgenerator.h"

#include <QDialog>


namespace Ui {
class CreateAccountDialog;
}

class CreateAccountDialog : public QDialog
{
    Q_OBJECT

public:
    CreateAccountDialog(OpenSSLAESEngine *_engine, Generator _generator, QWidget *parent = nullptr);
    CreateAccountDialog(OpenSSLAESEngine * engine, Generator _generator, Account *_account,
                                 QWidget *parent = nullptr);
    ~CreateAccountDialog();

    Account * getNewAccount();

private slots:
    void on_optionsPushButton_clicked();
    void on_generatePushButton_clicked();
    void on_savePushButton_clicked();
    void on_cancelPushButton_clicked();

    void editingFinished();
private:
    Ui::CreateAccountDialog *ui;

    Account *new_account;
    OpenSSLAESEngine *engine;
    Generator generator;
    bool create;

    void initialize(QString);

};

#endif // CREATEACCOUNTDIALOG_H

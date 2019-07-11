#ifndef RESCUEDIALOG_H
#define RESCUEDIALOG_H

class Account;

#include "passwordgenerator.h"

#include <QDialog>

namespace Ui {
class RescueDialog;
}

class RescueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RescueDialog(Account *account, QWidget *parent = nullptr);
    ~RescueDialog();


    QString getNewPassword() const;

private slots:
    void on_optionsPushButton_clicked();

    void on_generatePushButton_clicked();

    void on_cancelPushButton_clicked();

    void on_savePushButton_clicked();

    void on_rescuePushButton_clicked();

    void on_passwordLineEdit_textEdited(const QString &arg1);

private:
    Ui::RescueDialog *ui;

    Account *account;
    Generator generator;
    QString newPassword;


    void displayError(QString title, QString message);
};

#endif // RESCUEDIALOG_H

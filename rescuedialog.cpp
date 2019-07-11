#include "rescuedialog.h"
#include "ui_rescuedialog.h"

#include "account.h"
#include "generatoroptionsdialog.h"
#include "opensslaesengine.h"

#include <QMessageBox>


RescueDialog::RescueDialog(Account *_account, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RescueDialog),
    account{_account}
{
    ui->setupUi(this);

    this->setWindowTitle("Resuce Password");
    ui->savePushButton->setDefault(true);
    ui->savePushButton->setEnabled(false);

}

RescueDialog::~RescueDialog()
{
    delete ui;
}

void RescueDialog::on_optionsPushButton_clicked()
{
    GeneratorOptionsDialog diag{generator};

    if (diag.exec()) {
        generator = diag.getGenerator();
    }
}

void RescueDialog::on_generatePushButton_clicked()
{
    PasswordGenerator passwordGenerator{generator};
    QString password = passwordGenerator.generate();

    ui->passwordLineEdit->setText(password);
    ui->savePushButton->setEnabled(true);
}

void RescueDialog::on_cancelPushButton_clicked()
{
    reject();
}

void RescueDialog::on_savePushButton_clicked()
{
    if (ui->passwordLineEdit->text().length() > 0) {
        newPassword = ui->passwordLineEdit->text();
        accept();
    } else {
        reject();
    }
}


void RescueDialog::on_rescuePushButton_clicked()
{
    QString key = ui->rescueKeyLineEdit->text();

    if (key.length() > 0) {
        OpenSSLAESEngine engine{key.toStdString()};

        try {
            QString pWord = QString::fromStdString(engine.decryptPassword(account->getPassword().toStdString()));
            ui->passwordLineEdit->setText(pWord);
            ui->savePushButton->setEnabled(true);
        } catch (EncryptionException) {
            displayError("Key Error", "The password could not be decrypted with supplied key");
        }

    } else {
        displayError("Key Error", "A key needs to be entered");
    }
}


void RescueDialog::on_passwordLineEdit_textEdited(const QString &text)
{
    if (text.length() > 0)
        ui->savePushButton->setEnabled(true);
    else
        ui->savePushButton->setEnabled(false);
}


QString RescueDialog::getNewPassword() const
{
    return newPassword;
}


void RescueDialog::displayError(QString title, QString message)
{
    QMessageBox::critical(this, title, message);
}

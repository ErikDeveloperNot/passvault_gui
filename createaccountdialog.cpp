#include "createaccountdialog.h"
#include "ui_createaccountdialog.h"
#include "generatoroptionsdialog.h"

#include <QDebug>
#include <QMessageBox>


/*
 * Used to create a new account
 */
CreateAccountDialog::CreateAccountDialog(OpenSSLAESEngine *_engine, Generator _generator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAccountDialog),
    engine{_engine},
    generator{_generator}
{
    ui->setupUi(this);
    create = true;

    initialize("Create New Account");
}

/*
 * Used to edit existing account
 */
CreateAccountDialog::CreateAccountDialog(OpenSSLAESEngine *_engine, Generator _generator, Account *_account,
                                         QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CreateAccountDialog),
    engine{_engine},
    generator{_generator}
{
    ui->setupUi(this);
    new_account = _account;
    create = false;
    ui->accountNameLineEdit->setReadOnly(true);
    ui->accountNameLineEdit->setEnabled(false);
//    QPalette p{};
//    p.setColor(QPalette::Base, Qt::lightGray);
//    ui->accountNameLineEdit->setPalette(p);
    ui->accountNameLineEdit->setText(new_account->getAccountName());
    ui->usernameLineEdit->setText(new_account->getUserName());
    ui->urlLineEdit->setText(new_account->getUrl());

    try {
        QString pWord = QString::fromStdString(engine->decryptPassword(new_account->getPassword().toStdString()));
        ui->passwordLineEdit->setText(pWord);
        ui->pasword2LineEdit->setText(pWord);
    } catch (EncryptionException &ex) {
        qDebug() << "Unable to Decrypt password... Should never see this anymore";
    }

    initialize("Edit Account");
}


void CreateAccountDialog::initialize(QString title)
{
    this->setWindowTitle(title);
    ui->generatePushButton->setDefault(true);
    ui->savePushButton->setEnabled(false);

    connect(ui->usernameLineEdit, &QLineEdit::textChanged, this, &CreateAccountDialog::editingFinished);
    connect(ui->accountNameLineEdit, &QLineEdit::textChanged, this, &CreateAccountDialog::editingFinished);
    connect(ui->passwordLineEdit, &QLineEdit::textChanged, this, &CreateAccountDialog::editingFinished);
    connect(ui->pasword2LineEdit, &QLineEdit::textChanged, this, &CreateAccountDialog::editingFinished);
    connect(ui->urlLineEdit, &QLineEdit::textChanged, this, &CreateAccountDialog::editingFinished);
}


CreateAccountDialog::~CreateAccountDialog()
{
    qDebug() << "Destroying CreateAccountDialog";
    delete ui;
}


Account * CreateAccountDialog::getNewAccount()
{
    return new_account;
}


void CreateAccountDialog::on_optionsPushButton_clicked()
{
    GeneratorOptionsDialog diag{generator, this};

    if (diag.exec()) {
        qDebug() << "Using custom options";
        generator = diag.getGenerator();
    }
}


void CreateAccountDialog::on_generatePushButton_clicked()
{
    PasswordGenerator gen{generator};
    QString password = gen.generate();
    ui->passwordLineEdit->setText(password);
    ui->pasword2LineEdit->setText(password);
}


void CreateAccountDialog::on_savePushButton_clicked()
{
    qDebug() << "Accept";
    QString accountName = ui->accountNameLineEdit->text();
    QString userName = ui->usernameLineEdit->text();
    QString url = ui->urlLineEdit->text();
    QString password = ui->passwordLineEdit->text();
    QString password2 = ui->pasword2LineEdit->text();

    if (accountName.length() < 1 || userName.length() < 1 || password.length() < 1 ||
            password2.length() < 1 || password != password2) {

        QMessageBox::critical(this, "Validation Error",
                              "Account, user, and password fields can not be empty. Passwords must match");
    } else {
        QString encrypted = QString::fromStdString(engine->encryptPassword(password.toStdString()));

        if (create) {
            new_account = new Account{accountName, userName, encrypted, encrypted, url};
        } else {
            new_account->setUserName(userName);
            new_account->setUrl(url);

            if (encrypted != new_account->getPassword()) {
                new_account->setOldPassword(new_account->getPassword());
                new_account->setPassword(encrypted);
            }
        }

        accept();
    }
}


void CreateAccountDialog::on_cancelPushButton_clicked()
{
    qDebug() << "Rejected";
    reject();
}


void CreateAccountDialog::editingFinished()
{
    if (ui->usernameLineEdit->text().length() > 0 &&
            ui->accountNameLineEdit->text().length() > 0 &&
            ui->passwordLineEdit->text().length() > 0 &&
            ui->pasword2LineEdit->text().length() > 0) {

        ui->savePushButton->setEnabled(true);
        ui->savePushButton->setDefault(true);
    } else {
        ui->savePushButton->setEnabled(false);
        ui->generatePushButton->setDefault(true);
    }
}



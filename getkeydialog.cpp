#include "getkeydialog.h"
#include "ui_getkeydialog.h"



GetKeyDialog::GetKeyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetKeyDialog)
{
    ui->setupUi(this);
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
}

GetKeyDialog::~GetKeyDialog()
{
    delete ui;
}

QString GetKeyDialog::getKey()
{
    return key;
}

void GetKeyDialog::on_buttonBox_accepted()
{
    key = ui->passwordLineEdit->text();
}

void GetKeyDialog::on_buttonBox_rejected()
{
//    key = "password";
}

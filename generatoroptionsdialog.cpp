#include "generatoroptionsdialog.h"
#include "ui_generatoroptionsdialog.h"

#include <QDebug>

GeneratorOptionsDialog::GeneratorOptionsDialog(Generator _generator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GeneratorOptionsDialog),
    generator{_generator}
{
    ui->setupUi(this);
    this->setWindowTitle("Password Generator Options");
    ui->savePushButton->setDefault(true);
    ui->charClassAllowedTextEdit->setReadOnly(true);

    passwordGenerator = new PasswordGenerator{generator};

    passwordGenerator->lowerEnabled() ? ui->lowercaseCheckBox->setCheckState(Qt::CheckState::Checked) :
                                        ui->lowercaseCheckBox->setCheckState(Qt::CheckState::Unchecked);
    passwordGenerator->upperEnabled() ? ui->uppercaseCheckBox->setCheckState(Qt::CheckState::Checked) :
                                        ui->uppercaseCheckBox->setCheckState(Qt::CheckState::Unchecked);
    passwordGenerator->digitsEnabled() ? ui->digitsCheckBox->setCheckState(Qt::CheckState::Checked) :
                                        ui->digitsCheckBox->setCheckState(Qt::CheckState::Unchecked);
    passwordGenerator->specialsEnabled() ? ui->specialCheckBox->setCheckState(Qt::CheckState::Checked) :
                                        ui->specialCheckBox->setCheckState(Qt::CheckState::Unchecked);

    ui->lengthSpinBox->setValue(passwordGenerator->getPasswordLength());
    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());

}


GeneratorOptionsDialog::~GeneratorOptionsDialog()
{
    delete passwordGenerator;
    delete ui;
}


Generator GeneratorOptionsDialog::getGenerator()
{
    passwordGenerator->setPasswordLength(ui->lengthSpinBox->value());

    QStringList manualList = ui->manualAllowedTextEdit->toPlainText().split("", QString::SkipEmptyParts);
    for (QString val : manualList)
        if (val != " ")
            passwordGenerator->addChar(val.toStdString()[0]);

    return passwordGenerator->getGeneratorOptions();
}


void GeneratorOptionsDialog::hideButtons()
{
    ui->savePushButton->setEnabled(false);
    ui->savePushButton->setHidden(true);
    ui->cancelPushButton->setEnabled(false);
    ui->cancelPushButton->setHidden(true);
}


void GeneratorOptionsDialog::on_lowercaseCheckBox_stateChanged(int checked)
{
    if (checked == Qt::CheckState::Checked) {
        passwordGenerator->enableLower(true);
    } else {
        passwordGenerator->enableLower(false);
    }

    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());
}


void GeneratorOptionsDialog::on_uppercaseCheckBox_stateChanged(int checked)
{
    if (checked == Qt::CheckState::Checked) {
        passwordGenerator->enableUpper(true);
    } else {
        passwordGenerator->enableUpper(false);
    }

    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());
}


void GeneratorOptionsDialog::on_digitsCheckBox_stateChanged(int checked)
{
    if (checked == Qt::CheckState::Checked) {
        passwordGenerator->enableDigits(true);
    } else {
        passwordGenerator->enableDigits(false);
    }

    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());
}


void GeneratorOptionsDialog::on_specialCheckBox_stateChanged(int checked)
{
    if (checked == Qt::CheckState::Checked) {
        passwordGenerator->enableSpecial(true);
    } else {
        passwordGenerator->enableSpecial(false);
    }

    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());
}


void GeneratorOptionsDialog::on_cancelPushButton_clicked()
{
    reject();
}


void GeneratorOptionsDialog::on_savePushButton_clicked()
{
//    passwordGenerator->setPasswordLength(ui->lengthSpinBox->value());

//    QStringList manualList = ui->manualAllowedTextEdit->toPlainText().split("", QString::SkipEmptyParts);
//    for (QString val : manualList)
//        if (val != " ")
//            passwordGenerator->addChar(val.toStdString()[0]);

    accept();
}


void GeneratorOptionsDialog::displayAllowedCharacters(std::set<char> chars)
{
    ui->charClassAllowedTextEdit->clear();

    QString allowed{};

    for (char val : chars)
        allowed.append(QString{val}+" ");

    ui->charClassAllowedTextEdit->setText(allowed);
}


void GeneratorOptionsDialog::on_resetPushButton_clicked()
{
    passwordGenerator->enableLower(true);
    passwordGenerator->enableUpper(true);
    passwordGenerator->enableDigits(true);
    passwordGenerator->enableSpecial(true);
    passwordGenerator->setPasswordLength(32);
    ui->lengthSpinBox->setValue(32);
    ui->manualAllowedTextEdit->clear();
    displayAllowedCharacters(passwordGenerator->getAllowedCharacters());
}

#ifndef GENERATOROPTIONSDIALOG_H
#define GENERATOROPTIONSDIALOG_H

#include "settings.h"
#include "passwordgenerator.h"

#include <QDialog>

#include <set>

namespace Ui {
class GeneratorOptionsDialog;
}

class GeneratorOptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GeneratorOptionsDialog(Generator _generator, QWidget *parent = nullptr);
    ~GeneratorOptionsDialog();

    Generator getGenerator();

    // used to support preferences window from unneeded saves
    void hideButtons();

private slots:
    void on_lowercaseCheckBox_stateChanged(int arg1);

    void on_uppercaseCheckBox_stateChanged(int arg1);

    void on_digitsCheckBox_stateChanged(int arg1);

    void on_specialCheckBox_stateChanged(int arg1);

    void on_cancelPushButton_clicked();

    void on_savePushButton_clicked();

    void on_resetPushButton_clicked();

private:
    Ui::GeneratorOptionsDialog *ui;

    Generator generator;
    PasswordGenerator *passwordGenerator;

    void displayAllowedCharacters(std::set<char> chars);


};

#endif // GENERATOROPTIONSDIALOG_H

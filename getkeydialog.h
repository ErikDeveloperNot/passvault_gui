#ifndef GETKEYDIALOG_H
#define GETKEYDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class GetKeyDialog;
}

class GetKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GetKeyDialog(QWidget *parent = nullptr);
    ~GetKeyDialog();

    QString getKey();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::GetKeyDialog *ui;
    QString key;
};

#endif // GETKEYDIALOG_H

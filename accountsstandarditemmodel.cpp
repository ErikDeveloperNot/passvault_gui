#include "accountsstandarditemmodel.h"

#include <QDebug>


/*
 * NEVER call clear(), deletes the pointer
 */


AccountsStandardItemModel::AccountsStandardItemModel(AccountsStore *store, QObject *parent) : QStandardItemModel{parent}
{
    this->store = store;

    setColumnCount(1);

    for (Account *val : store->getAccounts()) {
        if (!val->getDeleted()) {
            appendRow(val);
            count++;
            names.insert(val->getAccountName(), val->getAccountName());
        }
    }

//    qDebug() << "row count:" << count << ", store count:" << store->getAccounts().size();
//    qDebug() << store->getAccounts()[0]->getAccountName();
//    clear();
//    setColumnCount(1);
//    appendRow(new Account{"a name", "u", "p", "p", "u"});
//    Account *a = new Account{"a name2", "u", "p", "p", "u"};
//    appendRow(a);
//    qDebug() << "a name: " << a->getAccountName();
//    insertRow(0, a);


//    appendRow(a);
//    qDebug() << store->getAccounts()[0]->getAccountName();
//    setSortRole(Qt::UserRole);
//    testSort();

}


int AccountsStandardItemModel::rowCount(const QModelIndex &parent) const
{
    return count;
}


int AccountsStandardItemModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}


QVariant AccountsStandardItemModel::data(const QModelIndex &index, int role) const
{
    Account *account = static_cast<Account*>(item(index.row()));
    if (account == nullptr) {
        qDebug() << "ITs null, " << index.row();
        return QVariant{};

    }

    switch (role) {
    case Qt::DisplayRole:
        if (account->getCorrectKey())
            return account->getAccountName();
        else
            return account->getAccountName() + "  <locked>";

    case Qt::FontRole:
        {
            QFont font{};
            font.setPointSize(14);
            if (account->getCorrectKey())
                font.setBold(true);

            return font;
        }
        break;

    case Qt::ForegroundRole:
        if (!account->getCorrectKey()) {
            QBrush backColor = QBrush{Qt::lightGray};
            return backColor;
        }
        break;

    case Qt::TextAlignmentRole:
        return Qt::AlignCenter;

    }

    return QVariant{};
}


void AccountsStandardItemModel::remove(int row)
{
    count--;
    takeRow(row);
}


void AccountsStandardItemModel::add(QStandardItem * item)
{
    count++;
    appendRow(item);
}


// used after sync to reload any changes to accounts, first clear then reload
void AccountsStandardItemModel::reset()
{
    //either have to loop through 1 list for each account then loop through the other
    //to see if it is present, or just remove all then add from accounts - wil try 1st
    int cnt = count;

    for (int var = 0; var < cnt; ++var) {
        QStandardItem *i = item(var);
        count--;
        takeRow(0);
    }

    count = 0;
    //try adding each account from store, if the object already exists think it is ignored
    for (Account *a : store->getAccounts()) {
        add(a);
    }
}



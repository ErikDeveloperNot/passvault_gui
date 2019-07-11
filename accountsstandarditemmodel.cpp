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
        qDebug() << "ITs null,";
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



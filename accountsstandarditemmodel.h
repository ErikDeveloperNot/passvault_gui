#ifndef ACCOUNTSSTANDARDITEMMODEL_H
#define ACCOUNTSSTANDARDITEMMODEL_H

#include "accountsstore.h"

#include <QStandardItemModel>
#include <QSortFilterProxyModel>



class AccountsStandardItemModel : public QStandardItemModel
{
public:
    AccountsStandardItemModel(AccountsStore *, QObject *);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    void remove(int);
    void add(QStandardItem *);

private:
    AccountsStore *store;
    int count{0};
};



class AccountsSortProxy : public QSortFilterProxyModel
{
public:
    AccountsSortProxy(AccountsStandardItemModel *model, AccountsStore *store) {
        this->model = model;
        this->store = store;
    }

    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override {
//        qDebug() << "testing: " << left.row();
        Account *a = static_cast<Account*>(model->item(left.row()));
        Account *b = static_cast<Account*>(model->item(right.row()));

        if (a->getDeleted())
            return false;
        if (b->getDeleted())
            return true;

        if (store->getSettings().general.sortMRU) {
            int a_cnt{0};
            int b_cnt{0};

            for (int i=0; i<Account::MRA_DAYS; i++) {
                a_cnt += a->getMap()[i];
                b_cnt += b->getMap()[i];

                // divide up by 7 day chunks
                if ((i+1) % 7 == 0) {
                    if (a_cnt > b_cnt)
                        return true;
                    else if (b_cnt > a_cnt)
                        return false;
                }
            }

            // if maps are the same then do alphabetic
            return a->getAccountName().toUpper() < b->getAccountName().toUpper();
        } else {
            return a->getAccountName().toUpper() < b->getAccountName().toUpper();
        }
    }

private:
    AccountsStandardItemModel *model;
    AccountsStore *store;
};


#endif // ACCOUNTSSTANDARDITEMMODEL_H

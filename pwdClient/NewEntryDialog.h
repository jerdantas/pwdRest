//
// Created by dantas on 8/18/26.
//

#ifndef NEWENTRYDIALOG_H
#define NEWENTRYDIALOG_H

namespace Ui {
    class NewEntryDialog;
}

#include <QDialog>
#include "PasswordClient.h"

namespace Ui {
    class NewEntryDialog;
}

class NewEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewEntryDialog(PasswordClient& pwclient, QWidget *parent = nullptr);
    ~NewEntryDialog() override;

    [[nodiscard]] QString getSite() const;
    [[nodiscard]] QString getUserId() const;
    [[nodiscard]] QString getPassword() const;
    void setSite(const QString& site) const;

private:
    Ui::NewEntryDialog *ui;
    PasswordClient& client;

    void acceptit();
};

#endif // NEWENTRYDIALOG_H
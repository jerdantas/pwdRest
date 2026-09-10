//
// Created by dantas on 8/21/26.
//

#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

#include "Environment.h"
#include "PasswordClient.h"

namespace Ui {
    class LoginDialog;
}

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(PasswordClient& client, Environment &env, QWidget *parent = nullptr);
    ~LoginDialog() override;

private slots:
    void onOwnerIdChanged(const QString &text) const;
    void onLoginClicked();
    void onSignupClicked();

private:
    Ui::LoginDialog *ui;
    PasswordClient& client;
    Environment &env;
};

#endif // LOGINDIALOG_H
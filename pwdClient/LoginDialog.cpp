//
// Created by dantas on 8/21/26.
//

#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>

LoginDialog::LoginDialog(PasswordClient& pwclient, QWidget *parent)
    : QDialog(parent), ui(new Ui::LoginDialog), client(pwclient) {
    ui->setupUi(this);

    connect(ui->ownerIdEdit, &QLineEdit::textChanged, this, &LoginDialog::onOwnerIdChanged);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(ui->signupButton, &QPushButton::clicked, this, &LoginDialog::onSignupClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    onOwnerIdChanged(ui->ownerIdEdit->text());
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::onOwnerIdChanged(const QString &text) {
    bool isEmpty = text.trimmed().isEmpty();
    ui->signupButton->setEnabled(isEmpty);
    ui->loginButton->setEnabled(!isEmpty);
}

void LoginDialog::onLoginClicked() {
    QString ownerId = ui->ownerIdEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (client.login(ownerId.toStdString(), password.toStdString())) {
        QString pwdPath = QDir::homePath() + "/.local/pwd-client";
        QDir().mkpath(pwdPath);
        QFile file(pwdPath + "/.env");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << ownerId << "\n" << password << "\n";
            file.close();
        }
        accept();
    } else {
        QMessageBox::critical(this, "Error", "Login failed. Please check your credentials.");
    }
}

void LoginDialog::onSignupClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Sign Up");
    QFormLayout form(&dialog);

    auto* ownerIdEdit = new QLineEdit(&dialog);
    auto* usernameEdit = new QLineEdit(&dialog);
    auto* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);

    form.addRow("Owner ID:", ownerIdEdit);
    form.addRow("Owner Name (UserName):", usernameEdit);
    form.addRow("Password:", passwordEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString ownerId = ownerIdEdit->text().trimmed();
        QString ownerName = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();

        if (ownerId.isEmpty() || ownerName.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Error", "All fields are required.");
            return;
        }

        if (client.signup(ownerId.toStdString(), ownerName.toStdString(), password.toStdString())) {
            QString pwdPath = QDir::homePath() + "/.local/pwd-client";
            QDir().mkpath(pwdPath);
            QFile file(pwdPath + "/.env");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << ownerId << "\n" << password << "\n";
                file.close();
            }
            accept();
        } else {
            QMessageBox::critical(this, "Error", "Signup failed.");
        }
    }
}
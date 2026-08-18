//
// Created by dantas on 8/18/26.
//

#include "NewEntryDialog.h"
#include "ui_NewEntryDialog.h"

#include <QMessageBox>

NewEntryDialog::NewEntryDialog(PasswordClient& pwclient, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewEntryDialog)
    , client(pwclient)
{
    ui->setupUi(this);
}

NewEntryDialog::~NewEntryDialog()
{
    delete ui;
}

QString NewEntryDialog::getSite() const {
    return ui->siteEdit->text();
}

QString NewEntryDialog::getUserId() const
{
    return ui->userIdEdit->text();
}

QString NewEntryDialog::getPassword() const {
    return ui->passwordEdit->text();
}

void NewEntryDialog::acceptit()
{
    const QString site = ui->siteEdit->text().trimmed();
    const QString user = ui->userIdEdit->text();
    const QString pass = ui->passwordEdit->text();

    if (site.isEmpty()) {
        QMessageBox::warning(this, "Validation", "Site is required.");
        return;
    }

    try {
        client.set(site.toStdString(), user.toStdString(), pass.toStdString());
        QMessageBox::information(this, "Saved",
                                 QString("Password for %1 was saved.").arg(site));
        setResult(QDialog::Accepted);
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Database error", ex.what());
    }
}

void NewEntryDialog::setSite(const QString &site) const {
    if (!site.isEmpty()) {
        ui->siteEdit->setText(site);
    }
}

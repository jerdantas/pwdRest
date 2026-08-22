//
// Created by dantas on 8/18/26.
//

#include <QClipboard>
#include <QMessageBox>
#include <QKeyEvent>
#include <QCompleter>

#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "NewEntryDialog.h"

MainWindow::MainWindow(PasswordClient& pwclient, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , client(pwclient)
    , siteModel(new QStringListModel(this))
{
    ui->setupUi(this);
    setWindowTitle("pw - Password Manager");

    // Get site names from PasswordClient
    QStringList siteNames = client.getSiteNames();

    // Create completer
    auto* completer = new QCompleter(siteNames, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);  // Optional: match anywhere in string

    // Attach completer to QLineEdit
    ui->siteEdit->setCompleter(completer);

    connect(ui->okButton, &QPushButton::clicked, this, &MainWindow::onOk);
    connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(ui->newButton, &QPushButton::clicked, this, &MainWindow::onNew);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::onClose);
    connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::onSiteSelected);
    connect(ui->listWidget, &QListWidget::currentItemChanged, this,
        [this](const QListWidgetItem* current, QListWidgetItem*) {
            if (current) ui->siteEdit->setText(current->text());
        });
    connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onSiteDoubleClicked);

    ui->siteEdit->installEventFilter(this);

    loadPasswords();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOk()
{
    const QString site = ui->siteEdit->text().trimmed();
    if (site.isEmpty()) {
        QMessageBox::information(this, "Info", "Type a site to look up.");
        return;
    }

    try {
        std::string user, pass;
        if (client.get(site.toStdString(), user, pass)) {
            // Show result and offer copy-to-clipboard actions
            QString msg = QString("Access to %1 is:\nuser=%2\npassword=%3")
                              .arg(site,
                                   QString::fromStdString(user),
                                   QString::fromStdString(pass));

            QMessageBox box(this);
            box.setWindowTitle("Result");
            box.setText(msg);
            QPushButton* copyUserBtn = box.addButton("Copy user", QMessageBox::ActionRole);
            QPushButton* copyPassBtn = box.addButton("Copy password", QMessageBox::ActionRole);
            QPushButton* copyBothBtn = box.addButton("Copy both", QMessageBox::ActionRole);
            box.addButton(QMessageBox::Close);

            box.exec();

            QClipboard* cb = QGuiApplication::clipboard();
            if (box.clickedButton() == copyUserBtn) {
                cb->setText(QString::fromStdString(user));
            } else if (box.clickedButton() == copyPassBtn) {
                cb->setText(QString::fromStdString(pass));
            } else if (box.clickedButton() == copyBothBtn) {
                cb->setText(QString("user=%1 password=%2")
                                .arg(QString::fromStdString(user),
                                     QString::fromStdString(pass)));
            }
            // No extra popup after copy to keep it quiet.
        } else {
            // auto sites = client.listSites();
            // QString list;
            // for (const auto& s : sites) list += "  • " + QString::fromStdString(s) + "\n";
            // QString msg = QString("%1 not found, choose from:\n%2").arg(site, list);
            QString msg = QString("%1 not found.").arg(site);
            QMessageBox::information(this, "Not found", msg);
        }
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Database error", ex.what());
    }
}

void MainWindow::loadPasswords() const {
    ui->listWidget->clear();
    auto entries = client.listSites();
    for (const auto& entry : entries) {
        ui->listWidget->addItem(QString::fromStdString(entry));
    }
}

void MainWindow::onNew() {
    NewEntryDialog dialog(client, this);
    dialog.setSite(ui->siteEdit->text());
    if (dialog.exec() == QDialog::Accepted) {
        auto name = dialog.getSite();
        auto userId = dialog.getUserId();
        auto password = dialog.getPassword();
        if (!name.isEmpty() && !userId.isEmpty() && !password.isEmpty()) {
            client.set(name.toStdString(), userId.toStdString(), password.toStdString());
            loadPasswords();
            refreshCompleter();
        } else {
            QMessageBox::warning(this, "Input Error", "Name, userid, and password cannot be empty.");
        }
    }
}

void MainWindow::onDelete() {
    const QString site = ui->siteEdit->text().trimmed();
    if (site.isEmpty()) {
        QMessageBox::information(this, "Info", "Type a site to delete.");
        return;
    }

    try {
        if (client.del(site.toStdString())) {
            QMessageBox::information(this, "Deleted", QString("%1 deleted").arg(site));
            refreshCompleter();
        } else {
            auto sites = client.listSites();
            QString list;
            for (const auto& s : sites) list += "  • " + QString::fromStdString(s) + "\n";
            QString msg = QString("%1 not found, choose from:\n%2").arg(site, list);
            QMessageBox::information(this, "Not found", msg);
        }
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Database error", ex.what());
    }
}

void MainWindow::refreshCompleter() {
    QStringList sites;
    try {
        for (const auto& s : client.listSites()) {
            sites << QString::fromStdString(s);
        }
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Network error", ex.what());
    }
    sites.sort(Qt::CaseInsensitive);
    siteModel->setStringList(sites);
}

void MainWindow::onClose() {
    close();
}

void MainWindow::onSiteSelected(const QListWidgetItem* item) const {
    if (item) {
        ui->siteEdit->setText(item->text());
    }
}

void MainWindow::onSiteDoubleClicked(const QListWidgetItem* item) const {
    if (item) {
        ui->siteEdit->setText(item->text());  // Optional: update siteEdit
        ui->okButton->click();                // Simulate OK button click
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (obj == ui->siteEdit && event->type() == QEvent::KeyPress) {
        auto* keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            ui->okButton->click();   // Simulate OK
            return true;
        } else if (keyEvent->key() == Qt::Key_Escape) {
            ui->closeButton->click(); // Simulate Close
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

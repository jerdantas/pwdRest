//
// Created by dantas on 8/15/26.
//


#include <QApplication>
#include <QMessageBox>
#include <QScreen>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QTextStream>
#include <QDir>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "MainWindow.h"
#include "PasswordClient.h"

static bool ensureCredentials(const std::string& serverUrl) {
    QString pwdPath = QDir::homePath() + "/.local/pwd";
    QDir().mkpath(pwdPath);
    QString envFile = pwdPath + "/.env";

    QFile file(envFile);
    if (file.exists()) {
        return true;
    }

    QDialog dialog;
    dialog.setWindowTitle("Sign Up");
    QFormLayout form(&dialog);

    auto* userIdEdit = new QLineEdit(&dialog);
    auto* usernameEdit = new QLineEdit(&dialog);
    auto* passwordEdit = new QLineEdit(&dialog);
    passwordEdit->setEchoMode(QLineEdit::Password);

    form.addRow("User ID:", userIdEdit);
    form.addRow("Username:", usernameEdit);
    form.addRow("Password:", passwordEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    while (true) {
        if (dialog.exec() != QDialog::Accepted) {
            return false;
        }

        QString userId = userIdEdit->text().trimmed();
        QString username = usernameEdit->text().trimmed();
        QString password = passwordEdit->text();

        if (userId.isEmpty() || username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(nullptr, "Error", "All fields are required.");
            continue;
        }

        httplib::Client cli(serverUrl);
        nlohmann::json j = {
            {"ownerId", userId.toStdString()},
            {"ownerName", username.toStdString()},
            {"ownerPwd", password.toStdString()}
        };

        auto res = cli.Post("/signup", j.dump(), "application/json");
        if (res && res->status == 201) {
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << userId << "\n" << password << "\n";
                return true;
            } else {
                QMessageBox::critical(nullptr, "Error", "Could not save .env file.");
                return false;
            }
        } else {
            QString errorMsg = res ? QString::fromStdString(res->body) : "Network error";
            QMessageBox::critical(nullptr, "Signup Failed", errorMsg);
        }
    }
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QIcon icon(":/icons/pwd-qt.png");
    // qDebug() << icon.isNull();
    app.setWindowIcon(icon);

    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * dpi / 96.0); // 96 = base DPI
    QApplication::setFont(font);

    try {
        // std::string serverUrl = "https://joaodantas.com.br";
        std::string serverUrl = "http://localhost:8080";

        if (!ensureCredentials(serverUrl)) {
            return 0; // User canceled signup
        }

        // Point to the Nginx reverse proxy or directly to the cpp-httplib server
        PasswordClient client(serverUrl);
        MainWindow w(client);
        w.setWindowIcon(icon);
        w.show();
        return app.exec();
    } catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, "Startup error", ex.what());
        return 1;
    }
}

//
// Created by dantas on 8/15/26.
//


#include <QApplication>
#include <QMessageBox>
#include <QScreen>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QDir>
#include <httplib.h>
#include <nlohmann/json.hpp>
#include "MainWindow.h"
#include "PasswordClient.h"
#include "LoginDialog.h"

static bool ensureCredentials(PasswordClient& client) {
    QString pwdPath = QDir::homePath() + "/.local/pwd-client";
    if (!QDir().mkpath(pwdPath)) {
        return false;
    };
    const QString envFile = pwdPath + "/.env";

    if (const QFile file(envFile); file.exists()) {
        Environment env(envFile.toStdString());
        client.setOwner(env.userid(), env.password());
        client.setBaseUrl(env.server());
        return true;
    }

    Environment env(envFile.toStdString());
    LoginDialog dialog(client, env);
    return dialog.exec() == QDialog::Accepted;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QIcon icon(":/icons/pwd-qt.png");
    // qDebug() << icon.isNull();
    QApplication::setWindowIcon(icon);

    QScreen *screen = QGuiApplication::primaryScreen();
    qreal dpi = screen->logicalDotsPerInch();
    QFont font = QApplication::font();
    font.setPointSizeF(font.pointSizeF() * dpi / 96.0); // 96 = base DPI
    QApplication::setFont(font);

    try {
        // std::string serverUrl = "https://joaodantas.com.br/pwserver";
        const std::string serverUrl = "http://localhost:8080";

        PasswordClient client{};

        if (!ensureCredentials(client)) {
            return 0; // User canceled signup
        }

        // Point to the Nginx reverse proxy or directly to the cpp-httplib server
        MainWindow w(client);
        w.setWindowIcon(icon);
        w.setWindowIcon(icon);
        w.show();
        return QApplication::exec();
    } catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, "Startup error", ex.what());
        return 1;
    }
}

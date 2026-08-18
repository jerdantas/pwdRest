#include <QApplication>
#include <QDir>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include "MainWindow.h"
#include "PasswordClient.h"

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
        // Point to the Nginx reverse proxy or directly to the cpp-httplib server
        PasswordClient client("https://joaodantas.com.br");
        MainWindow w(client);
        w.setWindowIcon(icon);
        w.show();
        return app.exec();
    } catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, "Startup error", ex.what());
        return 1;
    }
}

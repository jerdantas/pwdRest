//
// Created by dantas on 8/19/25.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QStringListModel>
#include "PasswordClient.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(PasswordClient& pwclient, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onOk();
    void onDelete();
    void onNew();
    void onClose();
    void onSiteSelected(QListWidgetItem* item);
    void onSiteDoubleClicked(QListWidgetItem* item);

private:
    Ui::MainWindow *ui;
    PasswordClient& client;
    QStringListModel* siteModel;

    void loadPasswords();
    void refreshCompleter();
};
#endif // MAINWINDOW_H

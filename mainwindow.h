#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QtCore>
#include <QtGui>
#include <QUdpSocket>

#define APP_NAME "PaintAppServer"
#define MAX_PEERS 20

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    Ui::MainWindow *ui;

    QUdpSocket *socket;
    quint16 port;
    QHostAddress *Peers[MAX_PEERS];
    int currentPeerNr;
    int PeerID(QHostAddress ip);
    void RemovePeer(QHostAddress ip);
    QString IPList();
    bool DuplicateIP(QHostAddress ip);
    void SendToPeers(QString string, QHostAddress *skipThisIP = NULL);
    void messageBox(QString title, QString message);

protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void ReadyRead();
    void on_btnStart_clicked();
    void on_actionLocal_IP_triggered();
    void on_actionExit_triggered();
};

#endif // MAINWINDOW_H

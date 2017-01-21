#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->editPort->setValidator(new QIntValidator(0, 65534, this));
    socket = NULL;
    for(int i = 0; i < MAX_PEERS; i++)
        Peers[i] = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
    for(int i = 0; i < MAX_PEERS; i++)
        if(Peers[i]) delete Peers[i];
}

int MainWindow::PeerID(QHostAddress ip)
{
    if(currentPeerNr == MAX_PEERS - 1) return -1;
    for(int i = 1; i <= currentPeerNr; i++)
        if(*(Peers[i]) == ip) return i;
    Peers[++currentPeerNr] = new QHostAddress(ip);
    return currentPeerNr;
}

void MainWindow::RemovePeer(QHostAddress ip)
{
    if(currentPeerNr == MAX_PEERS - 1)
    {
        delete Peers[currentPeerNr];
        Peers[currentPeerNr] = NULL;
        currentPeerNr--;
        return;
    }
    for(int i = 1; i <= currentPeerNr; i++)
        if(*(Peers[i]) == ip)
        {
            memmove(&Peers[i], &Peers[i+1], (currentPeerNr - i) * sizeof(Peers[0]));
            currentPeerNr--;
            break;
        }
}

QString MainWindow::IPList()
{
    QString s;
    for(int i = 1; i <= currentPeerNr; i++)
        s += Peers[i]->toString().mid(7) + "\n";
    return s;
}

bool MainWindow::DuplicateIP(QHostAddress ip)
{
    for(int i = 1; i <= currentPeerNr; i++)
        if(*(Peers[i]) == ip) return TRUE;
    return FALSE;
}

void MainWindow::SendToPeers(QString string, QHostAddress *skipThisIP)
{
    QByteArray Data;
    Data.append(string);
    for(int i = 1; i <= currentPeerNr; i++)
    {
        if(skipThisIP && *Peers[i] == *skipThisIP) continue;
        socket->writeDatagram(Data, *Peers[i], port);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, APP_NAME,
                                                                    tr("Are you sure?\n"),
                                                                    QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                    QMessageBox::Yes);
        if (resBtn != QMessageBox::Yes) {
            event->ignore();
        } else {
            if(socket)
            {
                SendToPeers("Server down");
                delete socket;
            }
            QApplication::quit();
        }
}

void MainWindow::messageBox(QString title, QString message)
{
    QMessageBox *msg = new QMessageBox();
    msg->setAttribute(Qt::WA_DeleteOnClose, true);
    msg->setWindowModality(Qt::NonModal);
    msg->setWindowFlags(msg->windowFlags() & ~Qt::WindowContextHelpButtonHint);
    msg->setWindowTitle(title);
    msg->setText(message);
    msg->show();
}

void MainWindow::ReadyRead()
{
    QByteArray datagram;
    QHostAddress senderIP;
    datagram.resize(socket->pendingDatagramSize());
    socket->readDatagram(datagram.data(), datagram.size(), &senderIP);
    if(datagram == "Connection OK"){
        if(!DuplicateIP(senderIP))
        {
            if(currentPeerNr == MAX_PEERS - 1) return;
            SendToPeers(senderIP.toString().mid(7) + " has connected");
            PeerID(senderIP);
            ui->labelListIP->setText(IPList());

            QByteArray Data;
            Data.append("Connected to server");
            socket->writeDatagram(Data, senderIP, port);
        }

    }
    else if(datagram == "Peer has disconnected")
    {
        RemovePeer(senderIP);
        ui->labelListIP->setText(IPList());
        SendToPeers(senderIP.toString().mid(7) + " has disconnected", &senderIP);
    }
    else if(datagram.startsWith("Size") || datagram == "pixmapChangeAck" || datagram == "Clear" || datagram.startsWith("Draw"))
    {
        if(PeerID(senderIP) != -1)
            SendToPeers(QString::number(PeerID(senderIP)) + "@" + datagram, &senderIP);
    }

}

void MainWindow::on_btnStart_clicked()
{
    port = ui->editPort->text().toInt();
    if(socket) delete socket;
    socket = new QUdpSocket(this);
    socket->bind(port);
    connect(socket, SIGNAL(readyRead()), this, SLOT(ReadyRead()));
    currentPeerNr = 0;
    for(int i = 0; i < MAX_PEERS; i++)
        if(Peers[i])
        {
            delete Peers[i];
            Peers[i] = NULL;
        }
    messageBox("PaintAppServer", "Server has started on port " + ui->editPort->text());
    ui->labelListIP->setText(IPList());
}

void MainWindow::on_actionLocal_IP_triggered()
{
    QString addressesString;
    foreach (const QHostAddress &address, QNetworkInterface::allAddresses()) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            addressesString += address.toString() + "\n";
    }

    messageBox("Local IP", addressesString);
}

void MainWindow::on_actionExit_triggered()
{
    closeEvent(new QCloseEvent());
}

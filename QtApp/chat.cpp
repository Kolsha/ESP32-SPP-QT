
#include "chat.h"
#include "remoteselector.h"
#include "ts_proto_client.h"

#include <qbluetoothuuid.h>
#include <qbluetoothserver.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>

#include <QTimer>

#include <QDebug>

// this Uuid is hardcoded in ESP32
static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805f9b34fb");

Chat::Chat(QWidget *parent)
    : QDialog(parent),  currentAdapterIndex(0), ui(new Ui_Chat)
{
    //! [Construct UI]
    ui->setupUi(this);

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));

    //! [Construct UI]

    localAdapters = QBluetoothLocalDevice::allDevices();
    if (localAdapters.count() < 2) {
        ui->localAdapterBox->setVisible(false);
    } else {
        //we ignore more than two adapters
        ui->localAdapterBox->setVisible(true);
        ui->firstAdapter->setText(tr("Default (%1)", "%1 = Bluetooth address").
                                  arg(localAdapters.at(0).address().toString()));
        ui->secondAdapter->setText(localAdapters.at(1).address().toString());
        ui->firstAdapter->setChecked(true);
        connect(ui->firstAdapter, SIGNAL(clicked()), this, SLOT(newAdapterSelected()));
        connect(ui->secondAdapter, SIGNAL(clicked()), this, SLOT(newAdapterSelected()));
        QBluetoothLocalDevice adapter(localAdapters.at(0).address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    }




    localName = QBluetoothLocalDevice().name();


    m_executeTimer = new QTimer(this);
    if(!m_executeTimer){
        return ;
    }
    connect(m_executeTimer, SIGNAL(timeout()), this, SLOT(executeTime()));
    m_executeTimer->setInterval(100);
    m_executeTimer->start();


}

Chat::~Chat()
{
    qDeleteAll(clients);

}

void Chat::executeTime()
{
    if(clients.isEmpty() || messages.isEmpty() || cmds.isEmpty()){
        return ;
    }

    foreach (const cmdInfo &cmd, cmds) {


    }



    messages.clear();
}


void Chat::clientConnected(const QString &name, const QString &addr)
{
    ui->chat->insertPlainText(QString::fromLatin1("Joined chat with %1.\n").arg(name));
}

void Chat::showLatency(const uint32_t latency)
{
    ui->quitButton->setText(QString("Latency: %1").arg(latency));
}

void Chat::newAdapterSelected()
{
    const int newAdapterIndex = adapterFromUserSelection();
    if (currentAdapterIndex != newAdapterIndex) {

        currentAdapterIndex = newAdapterIndex;
        const QBluetoothHostInfo info = localAdapters.at(currentAdapterIndex);
        QBluetoothLocalDevice adapter(info.address());
        adapter.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
        localName = info.name();
    }
}

int Chat::adapterFromUserSelection() const
{
    int result = 0;
    QBluetoothAddress newAdapter = localAdapters.at(0).address();

    if (ui->secondAdapter->isChecked()) {
        newAdapter = localAdapters.at(1).address();
        result = 1;
    }
    return result;
}


void Chat::clientDisconnected(const QString &addr)
{
    tsProtoClient *client = qobject_cast<tsProtoClient *>(sender());
    if (client) {
        Q_ASSERT(!addr.isEmpty());
        clients.remove(addr);
        //clients.removeOne(client);
        client->deleteLater();
    }
}



void Chat::connectClicked()
{
    ui->connectButton->setEnabled(false);

    // scan for services
    const QBluetoothAddress adapter = localAdapters.isEmpty() ?
                                           QBluetoothAddress() :
                                           localAdapters.at(currentAdapterIndex).address();

    RemoteSelector remoteSelector(adapter);
    remoteSelector.startDiscovery(QBluetoothUuid(serviceUuid));
    if (remoteSelector.exec() == QDialog::Accepted) {
        QBluetoothServiceInfo service = remoteSelector.service();

        qDebug() << "Connecting to service 2" << service.serviceName()
                 << "on" << service.device().name();

        // Create client
        qDebug() << "Going to create client";
        tsProtoClient *client = new tsProtoClient(this);
qDebug() << "Connecting...";

        connect(client, SIGNAL(messageReceived(QString,QString)),
                this, SLOT(showMessage(QString,QString)));

        connect(client, SIGNAL(latencyChanged(uint32_t)),
                this, SLOT(showLatency(uint32_t)));

        connect(client, SIGNAL(disconnected(QString)), this, SLOT(clientDisconnected(QString)));

        connect(client, SIGNAL(connected(QString, QString)), this, SLOT(clientConnected(QString, QString)));

        connect(this, SIGNAL(sendMessage(QString)), client, SLOT(sendMessage(QString)));
qDebug() << "Start client";

        client->startClient(service);

        clients.insert(client->getAddr(), client);
    }

    ui->connectButton->setEnabled(true);
}



void Chat::sendClicked()
{
    ui->sendButton->setEnabled(false);
    ui->sendText->setEnabled(false);

    showMessage(localName, ui->sendText->text());
    emit sendMessage(ui->sendText->text());

    ui->sendText->clear();

    ui->sendText->setEnabled(true);
    ui->sendButton->setEnabled(true);
}




void Chat::showMessage(const QString &sender, const QString &message)
{
    messages.insert(sender, message);

    ui->chat->insertPlainText(QString::fromLatin1("%1: %2\n").arg(sender, message));
    ui->chat->ensureCursorVisible();
}


#include <qbluetoothuuid.h>
#include <qbluetoothserver.h>
#include <qbluetoothservicediscoveryagent.h>
#include <qbluetoothdeviceinfo.h>
#include <qbluetoothlocaldevice.h>

#include <QTimer>
#include <QMessageBox>

#include <QLabel>
#include <QDebug>

#include "chat.h"
#include "remoteselector.h"
#include "ts_proto_client.h"

// this Uuid is hardcoded in ESP32
static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805f9b34fb");


//! [Ui methods]

Chat::Chat(QWidget *parent)
    : QDialog(parent),  currentAdapterIndex(0), ui(new Ui_Chat)
{
    //! [Construct UI]
    ui->setupUi(this);

    connect(ui->quitButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(sendClicked()));

     connect(ui->addActionButton, SIGNAL(clicked()), this, SLOT(addActionClicked()));



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

QGroupBox *Chat::createWidgetForDevice(const QString &name)
{
    QGroupBox *res = new QGroupBox(this);
    if(!res)
        return nullptr;

    res->setTitle(name);
    QLineEdit *edit = new QLineEdit(res);
    if(!edit){
        res->deleteLater();
        return nullptr;
    }


    QLabel *lbl = new QLabel(res);
    if(!lbl){
        res->deleteLater();
        edit->deleteLater();
        return nullptr;
    }
    lbl->setText("Latency: 0");




    QGridLayout *gridbox = new QGridLayout;
    if(!gridbox){
        res->deleteLater();
        edit->deleteLater();
        lbl->deleteLater();
        return nullptr;
    }
    gridbox->addWidget(edit, 0, 0);
    gridbox->addWidget(lbl, 1, 0);
    res->setLayout(gridbox);


    return res;
}

void Chat::setInputTextForDevice(const QString &addr, const QString &text)
{
    QMap<QString, QGroupBox *>::iterator it = clients_widget.find(addr);
    if(it != clients_widget.end()){
        QGroupBox *widget = it.value();
        widget->findChild<QLineEdit*>()->setText(text);
    }
}

QString Chat::getInputTextForDevice(const QString &addr)
{
    QMap<QString, QGroupBox *>::iterator it = clients_widget.find(addr);
    if(it != clients_widget.end()){
        QGroupBox *widget = it.value();
        return widget->findChild<QLineEdit*>()->text();
    }

    return QString();
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



Chat::~Chat()
{
    if(m_executeTimer){
        if(m_executeTimer->isActive())
            m_executeTimer->stop();
        delete m_executeTimer;
        m_executeTimer = 0;
    }

    clients.clear();

}

void Chat::executeTime()
{
    if(clients.isEmpty() || messages.isEmpty() || cmds.isEmpty()){
        return ;
    }

    QMutexLocker locker(&msgs_mutex);

    foreach (const cmdInfo &cmd, cmds) {
        bool cond = true;
        for (auto it = cmd.conds.begin(); it != cmd.conds.end(); ++it){
            //qDebug() << it.key() << ": " << it.value() << endl;
            auto msg = messages.find(it.key());
            if(msg == messages.end() || msg.value() != it.value()){
                cond = false;
                break;
            }
        }

        if(cond){
            ui->chat->insertPlainText(QString::fromLatin1("%1 executed\n").arg(cmd.cmd));
            ui->chat->ensureCursorVisible();
        }

    }



    messages.clear();
}


void Chat::clientConnected(const QString &name, const QString &addr)
{
    ui->chat->insertPlainText(QString::fromLatin1("Joined chat with %1.\n").arg(name));
    tsProtoClient *client = qobject_cast<tsProtoClient *>(sender());
    if (client) {
        clients.insert(addr, client);
        QGroupBox * widget = createWidgetForDevice(name);
        if(widget){
            clients_widget.insert(addr, widget);
            ui->deviceLayout->insertWidget(0, widget);
        }
    }
}

void Chat::clientDisconnected(const QString &addr)
{
    tsProtoClient *client = qobject_cast<tsProtoClient *>(sender());
    if (client) {
        Q_ASSERT(!addr.isEmpty());
        clients.remove(addr);
        clients_widget.remove(addr);
        client->deleteLater();
    }
}

void Chat::showMessage(const QString &sender, const QString &message)
{
    QMutexLocker locker(&msgs_mutex);
    messages.insert(sender, message);
    setInputTextForDevice(sender, message);
}

void Chat::showLatency(const QString &addr, const uint32_t latency)
{
    QMap<QString, QGroupBox *>::iterator it = clients_widget.find(addr);
    if(it != clients_widget.end()){
        QGroupBox *widget = it.value();
        widget->findChild<QLabel*>()->setText(QString("Latency: %1").arg(latency));
    }
}










void Chat::connectClicked()
{
    ui->connectButton->setEnabled(false);

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

        connect(client, SIGNAL(latencyChanged(QString, uint32_t)),
                this, SLOT(showLatency(QString, uint32_t)));

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

void Chat::addActionClicked()
{
    if(ui->cmdLineEdit->text().isEmpty()){
        QMessageBox::warning(this, "Warning", "Cmd can't be empty");
        return;
    }

    cmdInfo cmd;
    for (auto it = clients_widget.begin(); it != clients_widget.end(); ++it){

        QString tmp = it.value()->findChild<QLineEdit*>()->text();
        if(tmp.isEmpty())
            continue;
        cmd.conds.insert(it.key(), tmp);

    }
    if(cmd.conds.isEmpty()){
        QMessageBox::information(this, "Warning", "All client cmd is empty");
        return;
    }

    cmd.cmd = ui->cmdLineEdit->text();
    cmds.push_back(cmd);
    ui->cmdLineEdit->clear();
}




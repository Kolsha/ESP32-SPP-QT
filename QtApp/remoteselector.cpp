#include <qbluetoothdeviceinfo.h>
#include <qbluetoothaddress.h>
#include <qbluetoothlocaldevice.h>

#include "remoteselector.h"
#include "ui_remoteselector.h"


QT_USE_NAMESPACE

RemoteSelector::RemoteSelector(QWidget *parent)
:   QDialog(parent), ui(new Ui::RemoteSelector)
{
    ui->setupUi(this);
    m_localAdapter.clear();
}

RemoteSelector::~RemoteSelector()
{
    delete ui;
    delete m_discoveryAgent;
}

void RemoteSelector::startDiscovery()
{
    if(!m_discoveredServices.isEmpty()){
        return ;
    }

    ui->status->setText(tr("Scanning..."));
    ui->rescanButton->setEnabled(false);
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    ui->remoteDevices->clear();

    m_discoveryAgent->setUuidFilter(m_uuid);
    m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

}

void RemoteSelector::stopDiscovery()
{
    if (m_discoveryAgent){
        m_discoveryAgent->stop();
    }
}

QBluetoothServiceInfo RemoteSelector::service() const
{
    return m_service;
}

void RemoteSelector::setUuid(const QBluetoothUuid &uuid)
{
    m_uuid = uuid;
}

void RemoteSelector::setAdapter(const QBluetoothAddress &localAdapter)
{
    if(m_localAdapter == localAdapter){
        return ;
    }

    m_localAdapter = localAdapter;

    if(m_localAdapter.isNull()){
        return ;
    }

    if(m_discoveryAgent){
        delete m_discoveryAgent;
        m_discoveryAgent = 0;
    }

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(localAdapter);

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));
    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));
    connect(m_discoveryAgent, SIGNAL(canceled()), this, SLOT(discoveryFinished()));
}

void RemoteSelector::serviceDiscovered(const QBluetoothServiceInfo &serviceInfo)
{
#if 1
    qDebug() << "Discovered service on"
             << serviceInfo.device().name() << serviceInfo.device().address().toString();
    qDebug() << "\tService name:" << serviceInfo.serviceName();
    qDebug() << "\tDescription:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceDescription).toString();
    qDebug() << "\tProvider:"
             << serviceInfo.attribute(QBluetoothServiceInfo::ServiceProvider).toString();
    qDebug() << "\tL2CAP protocol service multiplexer:"
             << serviceInfo.protocolServiceMultiplexer();
    qDebug() << "\tRFCOMM server channel:" << serviceInfo.serverChannel();

    qDebug() << serviceInfo.serviceClassUuids();
    //qDebug() << serviceInfo.serviceUuid();
    //qDebug() << serviceInfo.attribute(QBluetoothServiceInfo::ServiceId).toUuid();
#endif

    QMapIterator<QListWidgetItem *, QBluetoothServiceInfo> i(m_discoveredServices);
    while (i.hasNext()){
        i.next();
        if (serviceInfo.device().address() == i.value().device().address()){
            return;
        }
    }

    QString remoteName;
    if (serviceInfo.device().name().isEmpty())
        remoteName = serviceInfo.device().address().toString();
    else
        remoteName = serviceInfo.device().name();

    QListWidgetItem *item =
        new QListWidgetItem(QString::fromLatin1("%1 %2").arg(remoteName,
                                                             serviceInfo.serviceName()));

    m_discoveredServices.insert(item, serviceInfo);
    ui->remoteDevices->addItem(item);
}

void RemoteSelector::discoveryFinished()
{
    ui->status->setText(tr("Select the chat service to connect to."));
    ui->rescanButton->setEnabled(true);
}

void RemoteSelector::on_remoteDevices_itemActivated(QListWidgetItem *item)
{
    qDebug() << "got click" << item->text();
    m_service = m_discoveredServices.value(item);
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();

    accept();
}

void RemoteSelector::on_cancelButton_clicked()
{
    reject();
}

void RemoteSelector::on_rescanButton_clicked()
{
    ui->remoteDevices->clear();
    m_discoveredServices.clear();
    m_discoveryAgent->clear();
    startDiscovery();
}

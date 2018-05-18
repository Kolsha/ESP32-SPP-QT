#ifndef REMOTESELECTOR_H
#define REMOTESELECTOR_H

#include <QDialog>

#include <qbluetoothuuid.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothservicediscoveryagent.h>

QT_FORWARD_DECLARE_CLASS(QModelIndex)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui {
    class RemoteSelector;
}
QT_END_NAMESPACE

class RemoteSelector : public QDialog
{
    Q_OBJECT

public:
    explicit RemoteSelector(QWidget *parent = 0);
    ~RemoteSelector();

    void startDiscovery();
    void stopDiscovery();
    QBluetoothServiceInfo service() const;

    void setUuid(const QBluetoothUuid &uuid);
    void setAdapter(const QBluetoothAddress &localAdapter);

private:
    Ui::RemoteSelector *ui;

    QBluetoothUuid m_uuid;
    QBluetoothAddress m_localAdapter;
    QBluetoothServiceDiscoveryAgent *m_discoveryAgent = nullptr;
    QBluetoothServiceInfo m_service;
    QMap<QListWidgetItem *, QBluetoothServiceInfo> m_discoveredServices;

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo &serviceInfo);
    void discoveryFinished();
    void on_remoteDevices_itemActivated(QListWidgetItem *item);
    void on_cancelButton_clicked();

    void on_rescanButton_clicked();
};

#endif // REMOTESELECTOR_H

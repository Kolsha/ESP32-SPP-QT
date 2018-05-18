#include "ui_chat.h"

#include <QDialog>

#include <qbluetoothserviceinfo.h>
#include <qbluetoothsocket.h>
#include <qbluetoothhostinfo.h>

#include <QDebug>

QT_USE_NAMESPACE

class tsProtoClient;

struct cmdInfo
{
    QString cmd;
    QMap<QString, QString> conds; // addr, msg
};


class Chat : public QDialog
{
    Q_OBJECT

public:
    Chat(QWidget *parent = 0);
    ~Chat();

signals:
    void sendMessage(const QString &message);

private slots:

    void executeTime();

    void connectClicked();
    void sendClicked();
    void addActionClicked();

    void showMessage(const QString &sender, const QString &message);


    void clientDisconnected(const QString &addr);
    void clientConnected(const QString &name, const QString &addr);

    void showLatency(const QString &addr, const uint32_t latency);

    void newAdapterSelected();

private:

    QGroupBox *createWidgetForDevice(const QString &name, const QString &addr);

    void setInputTextForDevice(const QString &addr, const QString &text);
    QString getInputTextForDevice(const QString &addr);

    int adapterFromUserSelection() const;
    int currentAdapterIndex;
    Ui_Chat *ui;

    QMap<QString, tsProtoClient*> clients; // addr, client

    QMap<QString, QGroupBox *> clients_widget; // addr, widget

    QMap<QString, QString> messages; // addr, message
    QMutex msgs_mutex;

    QVector<cmdInfo> cmds;

    QList<QBluetoothHostInfo> localAdapters;

    QString localName;

    QTimer *m_executeTimer = 0;
};




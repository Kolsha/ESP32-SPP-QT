
#include "chatclient.h"

#include <qbluetoothsocket.h>
#include <qbytearray.h>

#include <sys/time.h>


#include "ts_proto.h"




void ChatClient::syncTime()
{
    m_timeIsSynchronized = false;

    tsMsg_t msg = {
        .timestamp =  get_ts_time(),
        .cmd = tsProtoCmds_t::timeSyncReq,
        .data = {"Hello Kolsha"},
    };

    if(msg.timestamp.tv_sec == 0){
        return ;
    }

    prepare_msg(&msg);




    char *msg_bytes = reinterpret_cast<char*>(&msg);
    if(msg_bytes == nullptr){
        // error msg
        return ;
    }

    QByteArray ts_msg;
    ts_msg.append(msg_bytes, sizeof(tsMsg_t));

    socket->write(ts_msg);
}

void ChatClient::initTimer()
{
    m_timeIsSynchronized = false;
    m_syncTimer = new QTimer(this);
    if(!m_syncTimer){
        return ;
    }
    connect(m_syncTimer, SIGNAL(timeout()), this, SLOT(syncTime()));
    m_syncTimer->setInterval(1000);
    m_syncTimer->start();
}

void ChatClient::freeTimer()
{
    m_timeIsSynchronized = false;
    if(m_syncTimer){
        while(m_syncTimer->isActive()){
            m_syncTimer->stop();
        }

        delete m_syncTimer;
        m_syncTimer = 0;
    }
}

ChatClient::ChatClient(QObject *parent)
    :   QObject(parent), socket(0)
{
    m_lastMsgTS = {0, 0};
}

ChatClient::~ChatClient()
{
    stopClient();
}


void ChatClient::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (socket)
        return;

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnectToService done";

    connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(m_disconnected()));

    qDebug() << "Signaling done";
}


void ChatClient::stopClient()
{
    delete socket;
    socket = 0;

    freeTimer();
}


void ChatClient::readSocket()
{
    if (!socket)
        return;

    //qDebug() << "readSocket";


    while (socket->bytesAvailable() > 0) {
        emit messageReceived(socket->peerName(),
                             QString("----------------------------------"));
        QByteArray line = socket->readAll();
        for(size_t pos = 0; pos < (line.length() / sizeof(tsMsg_t)); pos++){

            uint8_t * tmp = (uint8_t*)(line.data() + pos * sizeof(tsMsg_t));
            if(*tmp != tsProto_Version){
                qDebug() << "Skip msg, cause version wrong";
                continue;
            }

            tsMsg_t *msg = reinterpret_cast< tsMsg_t *>(tmp);
            if(!msg)
                continue;
            if(msg->sign != sign_msg(msg)){
                qDebug() << "Skip msg: " << msg->data << "cause sign wrong";
                continue;
            }
            if(msg->cmd == timeSyncResponse){
                m_timeIsSynchronized = true;
                m_lastMsgTS = msg->timestamp;
                continue;
            }
            if(!m_timeIsSynchronized){
                continue;
            }

            uint32_t dt = get_ts_delta_time(&(msg->timestamp), &m_lastMsgTS);
            if(dt > MAX_dT){

                qDebug() << "Skip msg: " << (char*)msg->data << "cause dT exceeded " << dt;

                qDebug() << msg->timestamp.tv_sec << '.' << msg->timestamp.tv_usec;
                qDebug() << m_lastMsgTS.tv_sec << '.' << m_lastMsgTS.tv_usec;
                continue;
            }

            m_lastMsgTS = msg->timestamp;

            if(msg->cmd == dataOut){
                emit messageReceived(socket->peerName(),
                                     QString::fromUtf8((char*)msg->data, tsProto_MSG_DATA_LEN));
            }

        }
        //qDebug() << line;

    }
}


void ChatClient::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';
    socket->write(text);
}


void ChatClient::connected()
{
    initTimer();
    emit connected(socket->peerName());
}

void ChatClient::m_disconnected()
{
    freeTimer();
    emit disconnected();
}


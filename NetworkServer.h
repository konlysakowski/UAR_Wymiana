#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QString>
#include <QHostAddress>
#include <QtGlobal>
#include <QDataStream>

class NetworkServer : public QObject {
    Q_OBJECT

public:
    explicit NetworkServer(QObject *parent = nullptr);
    void startListening(quint16 port);
    bool isClientConnected() const;
    void stopListening();
    QTcpSocket* clientSocket() const { return m_clientSocket; }
    QTcpServer* serever() const {return m_server;}

signals:
    void clientConnected();
    void clientDisconnected();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;
};


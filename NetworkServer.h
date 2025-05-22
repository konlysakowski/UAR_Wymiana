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

signals:
    void clientConnected();
    void clientDisconnected();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;
};


#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QTcpServer>
#include <QString>
#include <QHostAddress>

class NetworkServer : public QObject {
    Q_OBJECT

public:
    explicit NetworkServer(QObject *parent = nullptr);
    void startListening(quint16 port);
    bool isClientConnected() const;
    void stopListening();
    void sendValue(double value);

signals:
    void clientConnected();
    void clientDisconnected();
    void valueReceived(double value);

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;

private slots:
    void onReadyRead();


};


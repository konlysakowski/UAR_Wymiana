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
   // bool receiveData(float &value);

signals:
    void clientConnected();
    void clientDisconnected();
    void sterowanieOdebrane(float u);

private slots:
    void onReadyRead();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;



};


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
    void sendValue(float value);
    void sendMessage(float, float);
   // bool receiveData(float &value);

signals:
    void clientConnected();
    void clientDisconnected();
    void sterowanieOdebrane(float u);
    void commandReceived(quint8 command);
    void clientFullyConnected();
    void wartoscRegulowanaOdebrana(float wartosc);
    void paramKpOdebrany(float wartosc);
    void paramTiOdebrany(float wartosc);
    void paramTdOdebrany(float wartosc);
    void typSygnaluOdebrany(int);
    void amplitudaOdebrana(float wartosc);
    void okresOdebrany(int);
    void stalaOdebrana(int);
    void wypelnienieOdebrane(float wartosc);
    void commandReceived(int);


private slots:
    void onReadyRead();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket;
    quint32 m_blockSize = 0;

};


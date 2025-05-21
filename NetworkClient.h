#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QHostAddress>
#include <QDataStream>

class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    void connectToServer(const QString &host, quint16 port);
    bool isConnected() const;
    void disconnectFromHost();
    void sendValue(float value);
    bool receiveData(float &value);
    void sendCommand(quint8 type);
    void sendMessage(float typ, float wartosc);
    void onReadyRead();


signals:
    void connected();
    void disconnected();
    void valueReceived(double value);
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
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *m_socket;

};






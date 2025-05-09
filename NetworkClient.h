#pragma once

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QHostAddress>

class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    void connectToServer(const QString &host, quint16 port);
    bool isConnected() const;
    void disconnectFromHost();
    void sendValue(double value);
    bool receiveData(double &value);

signals:
    void connected();
    void disconnected();
    void valueReceived(double value);

private slots:
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *m_socket;

};






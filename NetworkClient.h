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

signals:
    void connected();
    void disconnected();

private slots:
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *m_socket;

};






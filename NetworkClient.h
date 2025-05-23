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
    QTcpSocket* socket() const { return m_socket; }
signals:
    void connected(const QString&);
    void disconnected(const QString&);

private slots:
    void onConnected();
    void onDisconnected();

private:
    QTcpSocket *m_socket;
};






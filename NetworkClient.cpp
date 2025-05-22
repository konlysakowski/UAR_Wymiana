#include "NetworkClient.h"

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
}

void NetworkClient::connectToServer(const QString &host, quint16 port)
{
    m_socket->connectToHost(host, port);
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::onConnected()
{
    emit connected();
}

void NetworkClient::onDisconnected()
{
    emit disconnected();
}

void NetworkClient::disconnectFromHost()
{
    if (m_socket->isOpen()) {
        m_socket->disconnectFromHost();
    }
}


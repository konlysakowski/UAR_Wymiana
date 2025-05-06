#include "NetworkClient.h"

NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
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

void NetworkClient::sendValue(double value)
{
    if (!isConnected()) return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_6_0);
    stream << value;

    m_socket->write(data);
    m_socket->flush();
}

void NetworkClient::onReadyRead()
{
    QDataStream stream(m_socket);
    stream.setVersion(QDataStream::Qt_6_0);

    while (m_socket->bytesAvailable() >= sizeof(double)) {
        double value;
        stream >> value;
        emit valueReceived(value);
    }
}

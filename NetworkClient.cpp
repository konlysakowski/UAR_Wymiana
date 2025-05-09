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


bool NetworkClient::receiveData(double &value)
{
    if (!m_socket)
        return false;

    if (!m_socket->bytesAvailable()) {
        if (!m_socket->waitForReadyRead(200))  // ❗ czas oczekiwania
            return false;
    }

    if (m_socket->bytesAvailable() >= static_cast<int>(sizeof(double))) {
        QDataStream stream(m_socket);
        stream.setVersion(QDataStream::Qt_6_0);
        stream >> value;
        return true;
    }

    return false;
}

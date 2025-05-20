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

void NetworkClient::sendValue(float value)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << quint8(0x10);
    out << quint32(sizeof(float));
    out << value;

    m_socket->write(block);
    m_socket->flush();
    qDebug() << "[CLIENT] Wyslano u =" << value;
}


bool NetworkClient::receiveData(float &outValue)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return false;

    if (!m_socket->waitForReadyRead(500))
        return false;

    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_6_0);

    if (m_socket->bytesAvailable() < static_cast<int>(sizeof(quint32)))
        return false;

    quint32 blockSize = 0;
    in >> blockSize;

    while (m_socket->bytesAvailable() < blockSize) {
        if (!m_socket->waitForReadyRead(100)) {
            qDebug() << "[CLIENT] Czekanie na dane nie powiodło się.";
            return false;
        }
    }

    in >> outValue;

    qDebug() << "[CLIENT] Odebrano y =" << outValue;
    return true;
}

void NetworkClient::sendCommand(quint8 type) {
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << type;
    out << quint32(0);

    m_socket->write(block);
    m_socket->flush();
    qDebug() << "[CLIENT] Wyslano komendę typ:" << type;
}

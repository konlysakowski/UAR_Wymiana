#include "NetworkServer.h"


NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_clientSocket(nullptr)
{
    connect(m_server, &QTcpServer::newConnection, [this]()
    {
        m_clientSocket = m_server->nextPendingConnection();
        QString adres = m_clientSocket->peerAddress().toString();
        connect(m_clientSocket, &QTcpSocket::disconnected, [this]()
        {
            QString adres = m_clientSocket->peerAddress().toString();
            m_clientSocket->deleteLater();
            m_clientSocket = nullptr;
            emit clientDisconnected(adres);
        });
        emit clientConnected(adres);
    });

}

void NetworkServer::startListening(quint16 port)
{
    m_server->listen(QHostAddress::Any, port);
}

bool NetworkServer::isClientConnected() const
{
    return m_clientSocket && m_clientSocket->state() == QAbstractSocket::ConnectedState;
}

void NetworkServer::stopListening()
{
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
    m_server->close();
}


#include "NetworkServer.h"


NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_clientSocket(nullptr)
{
    connect(m_server, &QTcpServer::newConnection, [this]()
    {
        m_clientSocket = m_server->nextPendingConnection();
        connect(m_clientSocket, &QTcpSocket::disconnected, [this]()
        {
            m_clientSocket->deleteLater();
            m_clientSocket = nullptr;
            emit clientDisconnected();
        });
        connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
        emit clientConnected();
        emit clientFullyConnected();
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

void NetworkServer::sendValue(float value)
{
    if (!m_clientSocket || m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << quint8(0x10);
    out << quint32(sizeof(float));
    out << value;

    m_clientSocket->write(block);
    m_clientSocket->flush();

    qDebug() << "[SERVER] Odesłano y =" << value;
}

// void NetworkServer::onReadyRead() {
//     QDataStream in(m_clientSocket);
//     in.setVersion(QDataStream::Qt_6_0);

//     while (m_clientSocket->bytesAvailable() > 0) {
//         quint8 type;
//         quint32 length;

//         if (m_clientSocket->bytesAvailable() < sizeof(type) + sizeof(length))
//             return;

//         in >> type;
//         in >> length;

//         if (type == 0x10 && length == sizeof(float)) {
//             if (m_clientSocket->bytesAvailable() < static_cast<int>(length))
//                 return;

//             float u;
//             in >> u;
//             emit sterowanieOdebrane(u);
//         } else {
//             emit commandReceived(type);
//             return;
//         }
//     }
// }

void NetworkServer::sendMessage(float typ, float wartosc)
{
    if (!m_clientSocket || m_clientSocket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray wiadomosc;
    QDataStream out(&wiadomosc, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << typ;
    out << wartosc;

    m_clientSocket->write(wiadomosc);
    m_clientSocket->flush();

    qDebug() << "[SERVER] Wyslano typ:" << typ << ", wartosc:" << wartosc;
}

void NetworkServer::onReadyRead()
{
    QDataStream in(m_clientSocket);
    in.setVersion(QDataStream::Qt_6_0);

    while (m_clientSocket->bytesAvailable() >= 2 * sizeof(float))
    {
        float typ;
        float wartosc;
        in >> typ;
        in >> wartosc;

        qDebug() << "[SERVER] Odebrano typ:" << typ << ", wartosc:" << wartosc;

        int ityp = static_cast<int>(typ);
        switch (ityp)
        {
        case 0: emit sterowanieOdebrane(wartosc); break;
        case 10: emit commandReceived(static_cast<int>(wartosc)); break;
        case 2: emit paramKpOdebrany(wartosc); break;
        case 3: emit paramTiOdebrany(wartosc); break;
        case 4: emit paramTdOdebrany(wartosc); break;
        case 5: emit typSygnaluOdebrany(static_cast<int>(wartosc)); break;
        case 6: emit amplitudaOdebrana(wartosc); break;
        case 7: emit okresOdebrany(static_cast<int>(wartosc)); break;
        case 8: emit stalaOdebrana(static_cast<int>(wartosc)); break;
        case 9: emit wypelnienieOdebrane(wartosc); break;
        default:
            qDebug() << "[SERVER] Nieznany typ danych:" << typ;
            break;
        }
    }
}

// bool NetworkServer::receiveData(float &value)
// {
//     if (!m_clientSocket)
//         return false;


//     if (m_clientSocket->bytesAvailable() < static_cast<int>(sizeof(float))) {
//         if (!m_clientSocket->waitForReadyRead(200))  // 200 ms timeout
//             return false;
//     }


//     if (m_clientSocket->bytesAvailable() >= static_cast<int>(sizeof(float))) {
//         QDataStream stream(m_clientSocket);
//         stream.setVersion(QDataStream::Qt_6_0);  // zgodność z klientem!
//         stream >> value;
//         return true;
//     }

//     return false;


// }

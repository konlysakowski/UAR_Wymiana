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
    qDebug() << "[CLIENT] receiveData - start";
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        return false;
    }

    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_6_0);

    quint8 type;
    quint32 length;

    if (m_socket->bytesAvailable() < sizeof(type) + sizeof(length))
    {
        if (!m_socket->waitForReadyRead(500))
        {
            qDebug() << "[CLIENT] receiveData - w ifie od waitForReadyRead";
            return false;
        }
    }

    in >> type;
    in >> length;

    qDebug() << "[CLIENT] Otrzymano typ: " << type << ", długość: " << length;

    if(type != 0x10 || length != sizeof(float))
    {
        qDebug() << "[CLIENT] Nieprawidłowy typ wiadomości.";
        return false;
    }

    while (m_socket->bytesAvailable() < static_cast<int>(length)) {
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


void NetworkClient::sendMessage(float typ, float wartosc)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState)
        return;

    QByteArray wiadomosc;
    QDataStream out(&wiadomosc, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);

    out << typ;
    out << wartosc;

    m_socket->write(wiadomosc);
    m_socket->flush();

    qDebug() << "[CLIENT] Wyslano typ:" << typ << ", wartosc:" << wartosc;
}

void NetworkClient::onReadyRead()
{
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_6_0);

    while (m_socket->bytesAvailable() >= 2 * sizeof(float))
    {
        float typ;
        float wartosc;
        in >> typ;
        in >> wartosc;

        qDebug() << "[CLIENT] Odebrano typ:" << typ << ", wartosc:" << wartosc;

        int ityp = static_cast<int>(typ);
        switch (ityp)
        {
        case 1: emit wartoscRegulowanaOdebrana(wartosc); break;
        case 2: emit paramKpOdebrany(wartosc); break;
        case 3: emit paramTiOdebrany(wartosc); break;
        case 4: emit paramTdOdebrany(wartosc); break;
        case 5: emit typSygnaluOdebrany(static_cast<int>(wartosc)); break;
        case 6: emit amplitudaOdebrana(wartosc); break;
        case 7: emit okresOdebrany(static_cast<int>(wartosc)); break;
        case 8: emit stalaOdebrana(static_cast<int>(wartosc)); break;
        case 9: emit wypelnienieOdebrane(wartosc); break;
        case 10: emit commandReceived(static_cast<int>(wartosc)); break;
        default:
            qDebug() << "[CLIENT] Nieznany typ danych:" << typ;
            break;
        }
    }
}

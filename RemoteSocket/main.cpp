#include <QCoreApplication>
#include <QHostAddress>
#include <QTcpSocket>
#include <QDebug>

std::tuple<QHostAddress, quint16> parseAddressPort(const QString& string)
{
    auto ok = false;
    return std::tuple<QHostAddress, quint16> {
          string.split('=').at(1).split(':').at(0)
        , string.split('=').at(1).split(':').at(1).toUInt(&ok)
    };
}

std::tuple<QHostAddress, quint16> parseAddressPort(const QStringList& arguments, const QString& keyword)
{
    for (const auto& arg : arguments) {
        if (arg.contains(keyword)) {
            return parseAddressPort(arg);
        }
    }
    return {};
}

QString parseKey(const QStringList& arguments)
{
    for (const auto& arg : arguments) {
        if (arg.contains("key")) {
            return arg.split('=').at(1);
        }
    }
    return { "no_keyword" };
}

bool connectToHost(QTcpSocket& socket, const QHostAddress& address, quint16 port)
{
    QObject::connect(&socket, &QTcpSocket::connected, &socket, [address, port]() {
        qDebug() << "Connected to" << address << port;
    });
    QObject::connect(&socket, &QTcpSocket::disconnected, &socket, [address, port]() {
        qDebug() << "Disconnected from" << address << port;
    });

    qDebug() << "About to connect to" << address.toString() << port;
    socket.connectToHost(address, port);
    if (!socket.waitForConnected(5000)) {
        qCritical() << "Connection failed!" << socket.errorString();
        return false;
    }
    return true;
}

void connectSockets(const QString& srsName, QTcpSocket& srs, QTcpSocket& dst)
{
    QObject::connect(&srs, &QTcpSocket::readyRead, &dst, [srsName, &srs, &dst]() {
        qDebug() << "About to resend" << srs.bytesAvailable() << "bytes from" << srsName;
        dst.write(srs.readAll());
    });
}

void sendKeyPacket(QTcpSocket& remoteSocket, const QString& key)
{
    qDebug() << "About to send key packet:" << key;
    remoteSocket.write(key.toUtf8());
}

int main(int argc, char *argv[])
{
    QCoreApplication app { argc, argv };

    const auto [remoteAddress, remotePort] = parseAddressPort(app.arguments(), "remote");
    const auto [localAddress, localPort] = parseAddressPort(app.arguments(), "local");

    QTcpSocket remoteSocket;
    if (!connectToHost(remoteSocket, remoteAddress, remotePort)) {
        return 1;
    }

    QTcpSocket localSocket;
    if (!connectToHost(localSocket, localAddress, localPort)) {
        return 1;
    }

    connectSockets("remote", remoteSocket, localSocket);
    connectSockets("local", localSocket, remoteSocket);

    sendKeyPacket(remoteSocket, parseKey(app.arguments()));

    qInfo() << "Started...";

    return app.exec();
}

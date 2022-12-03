#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

auto ok = false;

QTcpSocket* ClientSocket = nullptr;
QTcpSocket* RemoteSocket = nullptr;

void changeSocket(QTcpSocket* oldSocket, QTcpSocket* newSocket)
{
    if (oldSocket) {
        oldSocket->deleteLater();
    }
    oldSocket = newSocket;
}

void connectSockets(const QString& srsName, QTcpSocket& srs, QTcpSocket& dst)
{
    QObject::connect(&srs, &QTcpSocket::readyRead, &dst, [srsName, &srs, &dst]() {
        qDebug() << "About to resend" << srs.bytesAvailable() << "bytes from" << srsName;
        dst.write(srs.readAll());
    });
}

void catchHelloPacket(QTcpSocket* socket)
{
    const auto context = new QObject { socket };
    QObject::connect(socket, &QTcpSocket::readyRead, context, [context, socket]() {
        const auto firstPacket = socket->readAll();
        if ("remote" == firstPacket) {
            changeSocket(RemoteSocket, socket);
            qInfo() << "remote socket changed";
        }
        else {
            if (RemoteSocket) {
                RemoteSocket->write(firstPacket);
            }
            changeSocket(ClientSocket, socket);
            qInfo() << "client socket changed";
        }

        if (ClientSocket && RemoteSocket) {
            qInfo() << "About to connect sockets...";
            connectSockets("cli", *ClientSocket, *RemoteSocket);
            connectSockets("rms", *RemoteSocket, *ClientSocket);
            delete context;
        }
    });
}

void watchSocketConnection(QTcpSocket* socket)
{
    QObject::connect(socket, &QTcpSocket::disconnected, socket, [socket]() {
        qDebug() << "Client disconnected";
        socket->deleteLater();
    });
}

int main(int argc, char *argv[])
{
    QCoreApplication app { argc, argv };

    QTcpServer server;

    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server]() {
        qInfo() << "New connection";
        const auto socket = server.nextPendingConnection();
        catchHelloPacket(socket);
        watchSocketConnection(socket);
    });

    auto ok = false;
    const auto port = app.arguments().at(1).toInt(&ok);
    if (!server.listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start" << server.errorString();
        return 1;
    }

    qInfo() << "Server started";

    return app.exec();
}

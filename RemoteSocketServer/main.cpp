#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

auto ok = false;

QPair<QTcpSocket*, QTcpSocket*> clients;

void connectSockets(const QString& srsName, QTcpSocket& srs, QTcpSocket& dst)
{
    QObject::connect(&srs, &QTcpSocket::readyRead, &dst, [srsName, &srs, &dst]() {
        qDebug() << "About to resend" << srs.bytesAvailable() << "bytes from" << srsName;
        dst.write(srs.readAll());
    });
}

void catchKeyPacket(QTcpSocket* client)
{
    const auto context = new QObject { client };
    QObject::connect(client, &QTcpSocket::readyRead, context, [context, &client]() {
        const auto keyPacket = client->readAll();
        const auto key = QString { keyPacket };

        if ("no_keyword" == key) {
            qCritical() << "Bad key received:" << key;
            client->deleteLater();
        }
        else {
            if (!clients.first) {
                clients.first = client;
            }
            else if (!clients.second) {
                clients.second = client;
            }
            else {
                qCritical() << "Internal error!";
                client->deleteLater();
            }

            if (clients.first && clients.second) {
                connectSockets(key + "_one", *clients.first, *clients.second);
                connectSockets(key + "_two", *clients.second, *clients.first);
            }
        }

        delete context;
    });
}

void watchClientConnection(QTcpSocket* client)
{
    QObject::connect(client, &QTcpSocket::disconnected, client, []() {
        qDebug() << "Client disconnected";
        clients.first->deleteLater();
        clients.second->deleteLater();
        clients = {};
    });
}

int main(int argc, char *argv[])
{
    QCoreApplication app { argc, argv };

    QTcpServer server;

    QObject::connect(&server, &QTcpServer::newConnection, &server, [&server]() {
        qInfo() << "New connection";
        const auto client = server.nextPendingConnection();
        catchKeyPacket(client);
        watchClientConnection(client);
    });

    auto ok = false;
    const auto port = app.arguments().at(1).toInt(&ok);
    if (!server.listen(QHostAddress::Any, port)) {
        qCritical() << "Server could not start";
        return 1;
    }

    qInfo() << "Server started";

    return app.exec();
}

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

auto ok = false;

QMap<QString, QPair<QTcpSocket*, QTcpSocket*>> clients;

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
            if (!clients.contains(key)) {
                clients.insert(key, {});
            }
            if (!clients[key].first) {
                clients[key].first = client;
            }
            else if (!clients[key].second) {
                clients[key].second = client;
            }
            else {
                qCritical() << "Internal error!";
                client->deleteLater();
            }

            if (clients[key].first && clients[key].second) {
                connectSockets(key + "_one", *clients[key].first, *clients[key].second);
                connectSockets(key + "_two", *clients[key].second, *clients[key].first);
            }
        }

        delete context;
    });
}

QString clientKey(QTcpSocket* client)
{
    for (const auto& value : qAsConst(clients)) {
        if ((client == value.first) || (client == value.second)) {
            return clients.key(value);
        }
    }
    return "no_keyword";
}


void watchClientConnection(QTcpSocket* client)
{
    QObject::connect(client, &QTcpSocket::disconnected, client, [client]() {
        const auto key = clientKey(client);
        qDebug() << "Client" << key << "disconnected";
        clients.value(key).first->deleteLater();
        clients.value(key).second->deleteLater();
        clients.remove(key);
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

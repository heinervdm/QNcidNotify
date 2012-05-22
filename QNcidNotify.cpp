#include "QNcidNotify.h"
#include "QNcidNotify.moc"

#include "config.h"

#include <QtDBus/QDBusInterface>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QIcon>
#include <QtCore/QDate>
#include <QtCore/QTime>
#include <QtCore/QDateTime>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtCore/QEventLoop>

QNcidNotify::QNcidNotify ( QWidget *parent ) : QWidget ( parent ), connected(false)
{
    sock = new QTcpSocket;
    sock->connectToHost ( "192.168.1.1", 3333, QIODevice::ReadOnly );

    log = new QList<LogEntry>();

    trayIcon = new QSystemTrayIcon;
    trayIcon->setIcon(QIcon(":/phone-icon.svg"));
    trayIcon->show();

    connect ( sock, SIGNAL ( readyRead() ), this, SLOT ( readData() ) );
    connect ( this, SIGNAL ( lineRead ( QString ) ), this, SLOT ( parseLine ( QString ) ) );
}


QNcidNotify::~QNcidNotify()
{
    if ( sock->isOpen() ) sock->disconnectFromHost();
}

void QNcidNotify::readData()
{
    int size = 1024;
    char data[size];
    sock->readLine ( data, size );
    emit lineRead ( QString ( data ) );
}

void QNcidNotify::parseLine ( QString line )
{
    line = line.simplified();
    qDebug() << "parseLine():" << line;
    if ( line.startsWith ( "200" ) ) {
        connected = true;
    } else if ( line.startsWith ( "300" ) ) {
        // end of call log
    } else if ( line.startsWith ( "CIDLOG:" ) ) {
        parseCID(line);
    } else if ( line.startsWith ( "CIDINFO:" ) ) {
        // ringing
    } else if ( line.startsWith ( "CID:" ) ) {
        parseCID(line);
    } else {
        // not handeled
    }
}

// CID: *DATE*18052012*TIME*1743*LINE*026322046391*NMBR*017691314331*MESG*NONE*NAME*NO NAME*
// CIDLOG: *DATE*11052012*TIME*1331*LINE*026322046391*NMBR*Anonym*MESG*NONE*NAME*NO NAME*
void QNcidNotify::parseCID(QString line)
{
    LogEntry entry;
    QDate date = QDate(0,0,0);
    QTime time = QTime(0,0);
    QStringList f = line.split("*");
    for (int i=1; i<f.length()-1; i+=2) {
        if (f[i].compare("DATE",Qt::CaseInsensitive) == 0) {
            date.setDate(f[i+1].right(4).toInt(),f[i+1].mid(2,2).toInt(),f[i+1].left(2).toInt());
        }
        else if (f[i].compare("TIME",Qt::CaseInsensitive) == 0) {
            time = QTime::fromString(f[i+1],"hhmm");
        }
        else if (f[i].compare("LINE",Qt::CaseInsensitive) == 0) {
            entry.phoneLine = f[i+1];
        }
        else if (f[i].compare("NMBR",Qt::CaseInsensitive) == 0) {
            entry.callerId = f[i+1];
        }
        else if (f[i].compare("NAME",Qt::CaseInsensitive) == 0) {
            entry.name = f[i+1];
        }
        else if (f[i].compare("MESG",Qt::CaseInsensitive) == 0) {
            entry.msg = f[i+1];
        }
        else {
            qDebug() << "parseCID(): Unkown part:" << f[i+1];
        }
    }
    entry.date = QDateTime(date, time);
    log->append(entry);

    qDebug() << "parseCID(): Number:" << entry.callerId << "Time:" << entry.date.toString();

    if (line.startsWith("CID:",Qt::CaseInsensitive)) showNotification(line);
}

QString QNcidNotify::logEntryToString(const QNcidNotify::LogEntry entry)
{
    QString msg;
    msg << tr("Number: ") << entry.callerId << endl() << tr("Time: ") << entry.date.toString();

    return msg;
}

QString QNcidNotify::getCallerInfo(QString number)
{
    QNetworkAccessManager *networkMgr = new QNetworkAccessManager(this);
    QNetworkReply *reply = networkMgr->get( QNetworkRequest( QUrl( "http://www.google.com" ) ) );
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
    loop.exec();
    return reply->readAll();
}


void QNcidNotify::showNotification(QString msg, int timeout)
{
    qDebug() << "showNotification():" << msg;
    // method uint org.freedesktop.Notifications.Notify(QString app_name, uint replaces_id, QString app_icon, QString summary, QString body, QStringList actions, QVariantMap hints, int timeout)
    QDBusInterface notify("org.freedesktop.Notifications",
                          "/org/freedesktop/Notifications",
                          "org.freedesktop.Notifications",
                          QDBusConnection::sessionBus());

    QVariantList args;
    args << QString("QNcidNotify");
    args << QVariant(QVariant::UInt);
    args << QString("phone-icon");
    args << QString(tr("Incomming Call"));
    args << msg;
    args << QStringList();
    args << QVariantMap();
    args << timeout;
    notify.callWithArgumentList(QDBus::Block, "Notify", args);
}
/*
static void QNcidNotify::showNotification(char *text)
{
	DBusGConnection* dbus_conn;
	DBusGProxy *dbus_proxy;

	dbus_conn = dbus_g_bus_get(DBUS_BUS_SESSION, NULL);
	dbus_proxy = dbus_g_proxy_new_for_name(dbus_conn,
					"org.freedesktop.Notifications",
					"/org/freedesktop/Notifications",
					"org.freedesktop.Notifications");

	if (dbus_proxy != NULL) {
		GArray *actions = g_array_sized_new(TRUE, FALSE,
						    sizeof(gchar *), 0);
		GHashTable *hints = g_hash_table_new_full(g_str_hash,
						g_str_equal, g_free, g_free);

		dbus_g_proxy_call_no_reply(dbus_proxy, "Notify",
					G_TYPE_STRING, PACKAGE,
					G_TYPE_UINT, 0,
					G_TYPE_STRING, "quassel",
					G_TYPE_STRING, "Eingehender Anruf",
					G_TYPE_STRING, text,
					G_TYPE_STRV,
					(gchar **)g_array_free(actions, FALSE),
					dbus_g_type_get_map("GHashTable",
							    G_TYPE_STRING,
							    G_TYPE_VALUE),
					   hints,
					G_TYPE_INT, -1,
					G_TYPE_INVALID);

		g_hash_table_destroy(hints);
	}
	g_object_unref(dbus_proxy);
	dbus_g_connection_unref(dbus_conn);
}
*/

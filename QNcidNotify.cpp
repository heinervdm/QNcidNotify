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
#include <QtCore/QEventLoop>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

QNcidNotify::QNcidNotify ( QWidget *parent ) : QWidget ( parent ), connected(false)
{
    ncidHostIP = "192.168.1.1";
    ncidHostPort = 3333;
    lookupUrl = "";

    sock = new QTcpSocket;
    sock->connectToHost (ncidHostIP, ncidHostPort, QIODevice::ReadOnly );

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

/**
 * reads one line from the ncid server
 */
void QNcidNotify::readData()
{
    int size = 1024;
    char data[size];
    sock->readLine ( data, size );
    emit lineRead ( QString ( data ) );
}

/**
 * Checks for type of line recived from ncid server and hands
 * it over to the line specific parsing function.
 * 
 * @param line The line to parse
 */
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


/**
 * Parses CID and CIDLOG lines with the following format:
 * CID: *DATE*18052012*TIME*1743*LINE*0123456789*NMBR*0987654321*MESG*NONE*NAME*NO NAME*
 * CIDLOG: *DATE*11052012*TIME*1331*LINE*0123456789*NMBR*Anonym*MESG*NONE*NAME*NO NAME*
 * 
 * @param line The line to parse
 */
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

    if (line.startsWith("CID:",Qt::CaseInsensitive)) showNotification(getCallerInfo(entry));
}

/**
 * Converts the LogEntry struct into a string
 * 
 * @param entry The log entry
 * @return the generated string
 */
QString QNcidNotify::logEntryToString(const QNcidNotify::LogEntry entry)
{
    QString msg;
    msg << tr("Number: ") << entry.callerId << endl() << tr("Time: ") << entry.date.toString();

    return msg;
}

/**
 * Generate the Notification text the the log entry
 *  * If the notification text for the number is already cached
 *    in the sqlite database, the cached version is used.
 *  * If no cached version is found but an url to look the text found 
 *    is given in the configuration, the text is recived from this webpage.
 *  * If no cache exists and no url is given the notification is generated
 *    by lotEntryToString()
 * 
 * @param entry the log entry
 * @return the notification text
 */
QString QNcidNotify::getCallerInfo(LogEntry entry)
{
    QString callerInfo;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");
    if (!db.open()) return NULL;
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS numbercache (number TEXT PRIMARY KEY, notification TEXT NOT NULL)");
    query.prepare("SELECT notification FROM numbercache WHERE number=?;");
    query.addBindValue(entry.callerId);
    query.exec();
    if (query.first()) {
        callerInfo = query.value(0).toString();
    }
    else if (!lookupUrl.isEmpty()) {
        QNetworkAccessManager *networkMgr = new QNetworkAccessManager(this);
        QNetworkReply *reply = networkMgr->get( QNetworkRequest( QUrl( lookupUrl + entry.callerId) ) );
        QEventLoop loop;
        QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
        loop.exec();
        callerInfo = QString(reply->readAll().data());
        if (callerInfo.isEmpty()) callerInfo = logEntryToString(entry);
        query.prepare("INSERT INTO numbercache (number, notification) VALUES(?,?)");
        query.addBindValue(entry.callerId);
        query.addBindValue(callerInfo);
        query.exec();
    }
    else {
        callerInfo = logEntryToString(entry);
    }
    db.close();
    return callerInfo;
}

/**
 * Shows a notification using the freedesktop.org DBus notification API
 * 
 * @param msg the message to show
 * @param timeout time to display the notification
 *                (if -1 the timeout is given by the window system)
 */
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

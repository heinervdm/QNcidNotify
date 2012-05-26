/*
    QNcidNotify - A Qt ncid (Network Caller ID) application
    Copyright (C) 2012  Thomas Zimmermann <bugs@vdm-design.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "QNcidNotify.h"
#include "QNcidNotify.moc"

#include "config.h"
#include "QNcidOptionsDialog.h"
#include "QNcidLogDialog.h"

#include <QtDBus/QDBusInterface>

#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QIcon>
#include <QtGui/QAction>
#include <QtGui/QMessageBox>

#include <QtCore/QDate>
#include <QtCore/QTime>
#include <QtCore/QDateTime>
#include <QtCore/QEventLoop>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>

QNcidNotify::QNcidNotify() : connected(false)
{
    settings = new QSettings;
    loadConfiguration();

    trayIcon = new QSystemTrayIcon;
    trayIcon->setIcon(QIcon(":/phone-icon.svg"));
    trayIcon->show();

    context = new QMenu(tr("QNcidNotify"));

    QAction *optAct = new QAction(tr("&Options"), this);
    optAct->setShortcuts(QKeySequence::Open);
    optAct->setStatusTip(tr("Shows the configuration dialog"));
    connect(optAct, SIGNAL(triggered()), this, SLOT(optAct()));
    context->addAction(optAct);

    QAction *logAct = new QAction(tr("Call &Log"), this);
    logAct->setShortcuts(QKeySequence::Open);
    logAct->setStatusTip(tr("Shows the call log"));
    connect(logAct, SIGNAL(triggered()), this, SLOT(logAct()));
    context->addAction(logAct);

    QAction *exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(exitAct()));
    context->addAction(exitAct);

    trayIcon->setContextMenu(context);

    connectToNcidServer();
}

QNcidNotify::~QNcidNotify()
{
    context->deleteLater();
    if (sock->isOpen()) {
        sock->close();
    }
}

/**
 * Sets internal connection status to the Ncid server
 */
void QNcidNotify::ncidServerConnected(bool c)
{
    connected = c;
}

/**
 * Load the application configuration from QSettings
 */
void QNcidNotify::loadConfiguration()
{
    ncidHostIP = settings->value("ncidserver/ip","192.168.1.1").toString();
    ncidHostPort = settings->value("ncidserver/port",3333).toInt();
    lookupUrl = settings->value("lookup/url","").toString();
    lookupState = settings->value("lookup/state", false).toBool();
    logDb = settings->value("lookup/db","test.db").toString();
}

/**
 * Opens the options dialog, called by the options button of the context menu
 */
void QNcidNotify::optAct()
{
    QNcidOptionsDialog *o = new QNcidOptionsDialog(settings);
    int ret = o->exec();
    if (ret == QDialog::Accepted) {
        QString oldServer = ncidHostIP;
        int oldPort = ncidHostPort;
        settings->setValue("ncidserver/ip", o->server->displayText());
        settings->setValue("ncidserver/port", o->port->value());
        settings->setValue("lookup/url", o->lookupurl->displayText());
        settings->setValue("lookup/state", (o->lookup->checkState() == Qt::Checked));
        settings->sync();
        loadConfiguration();
        if (oldPort != ncidHostPort || !oldServer.compare(ncidHostIP)) {
            connectToNcidServer();
        }
    }
    o->close();
    o->deleteLater();
}

/**
 * Opens the call log dialog, called by to Log button of the context menu
 */
void QNcidNotify::logAct()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(logDb);
    if (!db.open()) {
        QMessageBox::warning(0, tr("Database Connection Error"), tr("Can not connect to database %1.").arg(logDb));
    }
    else {
        QNcidLogDialog *o = new QNcidLogDialog(db);
        o->exec();
        o->close();
        o->deleteLater();
        db.close();
    }
}

/**
 * Exits the Application, called by the Exit Button of the context menu
 */
void QNcidNotify::exitAct()
{
    emit quit();
}

/**
 * Opens the connection to the Ncid server
 */
void QNcidNotify::connectToNcidServer()
{
//     if (sock) {
//         sock->close();
//         sock->deleteLater();
//         connected = false;
//     }

    sock = new QNcidSocket;
    sock->connectToHost (ncidHostIP, ncidHostPort, QIODevice::ReadOnly );
    connect(sock, SIGNAL(newLogEntry(QNcidSocket::LogEntry)), this, SLOT(logCall(QNcidSocket::LogEntry)));
    connect(sock, SIGNAL(incommingCall(QNcidSocket::LogEntry)), this, SLOT(newCall(QNcidSocket::LogEntry)));

    if (!sock->waitForConnected(1000)) {
        QMessageBox::warning(0, tr("Ncid Server Connection Error"), tr("Cannot not connect to ncid server (%1:%2)").arg(ncidHostIP).arg(ncidHostPort));
    }
}

/**
 * Show notification about new incomming call
 * 
 * @param entry the call information
 */
void QNcidNotify::newCall(const QNcidSocket::LogEntry entry)
{
    showNotification(getCallerInfo(entry));
}

/**
 * Stores a call in the log database, if it is not already stored
 * 
 * @param entry the call information
 */
void QNcidNotify::logCall(const QNcidSocket::LogEntry entry)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(logDb);
    if (!db.open()) return;
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS calllog (id INTEGER PRIMARY KEY AUTOINCREMENT, number TEXT NOT NULL, time TEXT NOT NULL, line TEXT, message TEXT, name TEXT)");
    query.prepare("SELECT id FROM calllog WHERE number=? AND time=?;");
    query.addBindValue(entry.callerId);
    query.addBindValue(entry.date.toString(Qt::ISODate));
    query.exec();
    if (!query.first()) {
        query.prepare("INSERT INTO calllog (number, time,line, message, name) VALUES(?,?,?,?,?)");
        query.addBindValue(entry.callerId);
        query.addBindValue(entry.date.toString(Qt::ISODate));
        query.addBindValue(entry.phoneLine);
        query.addBindValue(entry.msg);
        query.addBindValue(entry.name);
        query.exec();
    }
    db.close();
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
QString QNcidNotify::getCallerInfo(const QNcidSocket::LogEntry entry)
{
    QString callerInfo;
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(logDb);
    if (!db.open()) return NULL;
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS numbercache (number TEXT PRIMARY KEY, notification TEXT NOT NULL)");
    query.prepare("SELECT notification FROM numbercache WHERE number=?;");
    query.addBindValue(entry.callerId);
    query.exec();
    if (query.first()) {
        callerInfo = query.value(0).toString();
    }
    else if (lookupState && !lookupUrl.isEmpty()) {
        QNetworkAccessManager *networkMgr = new QNetworkAccessManager(this);
        QNetworkReply *reply = networkMgr->get( QNetworkRequest( QUrl( lookupUrl + entry.callerId) ) );
        QEventLoop loop;
        QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
        loop.exec();
        callerInfo = QString(reply->readAll().data());
        if (callerInfo.isEmpty()) callerInfo = QNcidSocket::logEntryToString(entry);
        query.prepare("INSERT INTO numbercache (number, notification) VALUES(?,?)");
        query.addBindValue(entry.callerId);
        query.addBindValue(callerInfo);
        query.exec();
    }
    else {
        callerInfo = QNcidSocket::logEntryToString(entry);
        query.prepare("INSERT INTO numbercache (number, notification) VALUES(?,?)");
        query.addBindValue(entry.callerId);
        query.addBindValue(callerInfo);
        query.exec();
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

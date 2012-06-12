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

#ifndef QNcidNotify_H
#define QNcidNotify_H

#include "QNcidSocket.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>

#include <QtNetwork/QTcpSocket>

#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMenu>

#include <QtSql/QSqlDatabase>

class QNcidNotify : public QObject
{
    Q_OBJECT
public:
    QNcidNotify ();
    virtual ~QNcidNotify();

private:
    QSettings *settings;
    QMenu *context;
    QSystemTrayIcon *trayIcon;
    QNcidSocket *sock;
    QString lookupUrl, ncidHostIP, logDb;
    int ncidHostPort;
    bool lookupState, connected;
    QSqlDatabase db;

    QString getCallerInfo(const QNcidSocket::LogEntry entry);
    void connectToNcidServer();

private slots:
    void logCall(const QNcidSocket::LogEntry);
    void showNotification(const QString msg, int timeout = -1);
    void loadConfiguration();
    void newCall(const QNcidSocket::LogEntry);
    void ncidServerConnected(bool);
    void optAct();
    void exitAct();
    void logAct();

signals:
    void quit();
};

#endif // QNcidNotify_H

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

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QSettings>

#include <QtNetwork/QTcpSocket>

#include <QtGui/QSystemTrayIcon>
#include <QtGui/QMenu>

class QNcidNotify : public QObject
{
    Q_OBJECT
public:
    QNcidNotify ();
    virtual ~QNcidNotify();
private:
    struct LogEntry {
        QDateTime date;
        QString callerId;
        QString phoneLine;
        QString msg;
        QString name;
    };
    QSettings *settings;
    QMenu *context;
    QList<LogEntry> *log;
    QSystemTrayIcon *trayIcon;
    QWidget *m_widget;
    QTcpSocket *sock;
    QString lookupUrl;
    QString ncidHostIP;
    int ncidHostPort;
    void parseCID(const QString line);
    bool connected, lookupState;
    static QString logEntryToString(const LogEntry entry);
    QString getCallerInfo(const LogEntry entry);
private slots:
    void readData();
    void parseLine(QString line);
    void showNotification(const QString msg, int timeout = -1);
    void optAct();
    void loadConfiguration();
    void exitAct();
signals:
    void lineRead(QString line);
    void quit();
};

#endif // QNcidNotify_H

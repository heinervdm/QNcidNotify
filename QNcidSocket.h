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


#ifndef QNCIDSOCKET_H
#define QNCIDSOCKET_H

#include <QtNetwork/QTcpSocket>

#include <QtCore/QString>
#include <QtCore/QDateTime>

class QNcidSocket : public QTcpSocket
{
    Q_OBJECT
public:
    struct LogEntry {
        QDateTime date;
        QString callerId;
        QString phoneLine;
        QString msg;
        QString name;
    };

    static QString logEntryToString(const QNcidSocket::LogEntry);

private:
    QString ncidHostIP;
    int ncidHostPort;
    QTcpSocket *sock;

    void parseCID(const QString);
    void parseLine(QString);

private slots:
    void readData();

signals:
    void connected(bool);
    void newLogEntry(const QNcidSocket::LogEntry);
    void incommingCall(const QNcidSocket::LogEntry);
};

#endif // QNCIDSOCKET_H

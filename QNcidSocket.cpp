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


#include "QNcidSocket.h"
#include "QNcidSocket.moc"

#include <QtCore/QStringList>

QNcidSocket::QNcidSocket(QObject* parent): QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(readData()));
}

/**
 * reads one line from the ncid server
 */
void QNcidSocket::readData()
{
    int size = 1024;
    char data[size];
    this->readLine (data, size);
    parseLine(QString(data));
}

/**
 * Converts the LogEntry struct into a string
 *
 * @param entry The log entry
 * @return the generated string
 */
QString QNcidSocket::logEntryToString(const QNcidSocket::LogEntry entry)
{
    QString msg;
    msg = tr("Number: ") + entry.callerId + "<br>" + tr("Time: ") + entry.date.toString();

    return msg;
}

/**
 * Checks for type of line recived from ncid server and hands
 * it over to the line specific parsing function.
 *
 * @param line The line to parse
 */
void QNcidSocket::parseLine ( QString line )
{
    line = line.simplified();
    qDebug() << "parseLine():" << line;
    if ( line.startsWith ( "200" ) ) {
        emit connected(true);
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
void QNcidSocket::parseCID(QString line)
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
    emit newLogEntry(entry);

    qDebug() << "parseCID(): Number:" << entry.callerId << "Time:" << entry.date.toString();

    if (line.startsWith("CID:",Qt::CaseInsensitive)) emit incommingCall(entry);
}

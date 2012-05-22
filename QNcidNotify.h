#ifndef QNcidNotify_H
#define QNcidNotify_H

#include <QtGui/QWidget>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtGui/QSystemTrayIcon>

class QNcidNotify : public QWidget
{
    Q_OBJECT
public:
    QNcidNotify ( QWidget *parent = 0 );
    virtual ~QNcidNotify();
private:
    struct LogEntry {
        QDateTime date;
        QString callerId;
        QString phoneLine;
        QString msg;
        QString name;
    };
    QList<LogEntry> *log;
    QSystemTrayIcon *trayIcon;
    QWidget *m_widget;
    QTcpSocket *sock;
    void parseCID(const QString line);
    bool connected;
    static QString logEntryToString(const LogEntry entry);
    QString getCallerInfo(QString number);
private slots:
    void readData();
    void parseLine(QString line);
    void showNotification(const QString msg, int timeout = -1);
signals:
    void lineRead(QString line);
};

#endif // QNcidNotify_H

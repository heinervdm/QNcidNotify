#ifndef QNcidNotify_H
#define QNcidNotify_H

#include <QtGui/QWidget>
#include <QtNetwork/QTcpSocket>
#include <QtCore/QString>

#include <KDE/Plasma/PopupApplet>

class QNcidNotify : public Plasma::PopupApplet
{
    Q_OBJECT
public:
    QNcidNotify ( QObject *, const QVariantList & );
    virtual ~QNcidNotify();
    QWidget *widget () {
        return m_widget;
    }
public slots:
      void init();
private:
    QWidget *m_widget;
    QTcpSocket *sock;
private slots:
    void readData();
    void parseLine(QString line);
signals:
    void lineRead(QString line);
};

#endif // QNcidNotify_H

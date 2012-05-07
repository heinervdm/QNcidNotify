#include "QNcidNotify.h"
#include "QNcidNotify.moc"

#include <QtDBus/QDBusConnection>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QIcon>
#include <KDE/KIcon>

QNcidNotify::QNcidNotify ( QObject *parent, const QVariantList &args ) : Plasma::PopupApplet ( parent, args )
{
    sock = new QTcpSocket;
    sock->connectToHost ( "192.168.1.1", 3333, QIODevice::ReadOnly );
}

void QNcidNotify::init()
{
    setPopupIcon(QIcon(KIcon("package_telephone").pixmap(22,22)));

    m_widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setSizeConstraint ( QLayout::SetNoConstraint );
    m_widget->setLayout ( layout );
    m_widget->setMinimumSize ( 200, 100 );

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
    if ( line.startsWith ( "200" ) ) {
        // connected
    } else if ( line.startsWith ( "300" ) ) {
        // end of call log
    } else if ( line.startsWith ( "CIDLOG:" ) ) {
        // incomming call log
    } else if ( line.startsWith ( "CIDINFO:" ) ) {
        // ring ring
    } else if ( line.startsWith ( "CID:" ) ) {
        QVBoxLayout *layout = new QVBoxLayout;
        layout->setSizeConstraint ( QLayout::SetNoConstraint );
        m_widget->setLayout ( layout );
        m_widget->setMinimumSize ( 200, 100 );
        QLabel *msg = new QLabel ( line );
        layout->addWidget ( msg );
        showPopup ( 5000 );
    } else {
        // not handeled
    }
}

K_EXPORT_PLASMA_APPLET ( qncidnotify, QNcidNotify )

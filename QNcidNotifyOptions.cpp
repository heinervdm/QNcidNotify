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

#include "QNcidNotifyOptions.h"
#include "QNcidNotifyOptions.moc"

#include <QtGui/QWidget>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGridLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QGroupBox>

QNcidNotifyOptions::QNcidNotifyOptions(QSettings *settings, QWidget* parent, Qt::WindowFlags f) : QDialog(parent)
{
    if (!settings) settings = new QSettings;

    QVBoxLayout *vl = new QVBoxLayout;
    setLayout(vl);

    QGroupBox *ncidGroupBox = new QGroupBox(tr("Ncid Server Configuration"));
    vl->addWidget(ncidGroupBox);
    QGridLayout *l = new QGridLayout;
    ncidGroupBox->setLayout(l);

    QLabel *serverLabel = new QLabel(tr("IP Address:"));
    l->addWidget(serverLabel, 1, 1);
    server = new QLineEdit(settings->value("ncidserver/ip","192.168.1.1").toString());
    l->addWidget(server, 1, 2);

    QLabel *portLabel = new QLabel(tr("Port:"));
    l->addWidget(portLabel, 2, 1);
    port = new QSpinBox();
    port->setRange(1, 65536);
    port->setValue(settings->value("ncidserver/port",3333).toInt());
    l->addWidget(port, 2, 2);

    QGroupBox *lookupGroupBox = new QGroupBox(tr("Caller ID Lookup Configuration"));
    vl->addWidget(lookupGroupBox);
    l = new QGridLayout;
    lookupGroupBox->setLayout(l);

    QLabel *lookLable = new QLabel(tr("Do Online Lookup:"));
    l->addWidget(lookLable, 1, 1);
    lookup = new QCheckBox;
    if (settings->value("lookup/state", false).toBool()) {
        lookup->setCheckState(Qt::Checked);
    } else {
        lookup->setCheckState(Qt::Unchecked);
    }

    QLabel *urlLabel = new QLabel(tr("URL:"));
    l->addWidget(serverLabel, 2, 1);
    lookupurl = new QLineEdit(settings->value("lookup/url","").toString());
    l->addWidget(lookupurl, 2, 2);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    vl->addWidget(buttonBox);
}



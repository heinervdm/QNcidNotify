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

#ifndef QNCIDOPTIONSDIALOG_H
#define QNCIDOPTIONSDIALOG_H

#include <QtGui/QDialog>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QCheckBox>

#include <QtCore/QSettings>

class QNcidOptionsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QNcidOptionsDialog(QSettings *settings = 0, QWidget* parent = 0, Qt::WindowFlags f = 0);
    QLineEdit *server, *lookupurl;
    QSpinBox *port;
    QCheckBox *lookup;
signals:
    void configurationChanged();
};

#endif // QNCIDOPTIONSDIALOG_H

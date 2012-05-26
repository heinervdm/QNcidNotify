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


#include "QNcidLogDialog.h"
#include "QNcidLogDialog.moc"

#include <QtSql/QSqlTableModel>

#include <QtGui/QTableView>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QVBoxLayout>

QNcidLogDialog::QNcidLogDialog(QSqlDatabase db, QWidget* parent, Qt::WindowFlags f): QDialog(parent, f)
{
    QVBoxLayout *vl = new QVBoxLayout;
    setLayout(vl);

    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->setTable("calllog");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(0, Qt::Horizontal, tr("Number"));
    model->setHeaderData(1, Qt::Horizontal, tr("Time"));

    QTableView *view = new QTableView;
    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    view->hideColumn(0); // don't show the ID
    view->hideColumn(2); // don't show the line
    view->hideColumn(3); // don't show the message
    view->hideColumn(4); // don't show the name
    view->show();

    vl->addWidget(view);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    vl->addWidget(buttonBox);
}

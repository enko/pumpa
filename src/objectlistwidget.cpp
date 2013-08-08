/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "objectlistwidget.h"
#include "pumpa_defines.h"
#include "activitywidget.h"

#include <QDebug>

//------------------------------------------------------------------------------

ObjectListWidget::ObjectListWidget(QWidget* parent) :
  ASWidget(parent)
{}

//------------------------------------------------------------------------------

QASAbstractObjectList* ObjectListWidget::initList(QString endpoint, 
                                                  QObject* parent) {
  m_asMode = QAS_OBJECTLIST;
  return QASObjectList::initObjectList(endpoint, parent);
}

//------------------------------------------------------------------------------

void ObjectListWidget::update() {
  ASWidget::update();
  fetchOlder();
}

//------------------------------------------------------------------------------

ObjectWidgetWithSignals*
ObjectListWidget::createWidget(QASAbstractObject* aObj) {
  QASObject* obj = qobject_cast<QASObject*>(aObj);
  if (!obj) {
    qDebug() << "ERROR ObjectListWidget::createWidget passed non-object";
    return NULL;
  }

  ObjectWidget* ow = new ObjectWidget(obj, this);
  connect(ow, SIGNAL(showContext(QASObject*)),
          this, SIGNAL(showContext(QASObject*)));
  return ow;
}

//------------------------------------------------------------------------------

QASObjectList* ObjectListWidget::objectList() const {
  return qobject_cast<QASObjectList*>(m_list);
}

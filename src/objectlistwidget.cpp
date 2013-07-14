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
  ASWidget(parent),
  m_asMode(QAS_NULL)
{}

//------------------------------------------------------------------------------

void ObjectListWidget::setEndpoint(QString endpoint, int asMode) {
  clear();
  m_asMode = QAS_OBJECTLIST;
  
  if (asMode != -1)
    m_asMode |= asMode;

  m_list = QASObjectList::initObjectList(endpoint,
                                               parent()->parent()->parent());
  connect(m_list, SIGNAL(changed()),
          this, SLOT(update()), Qt::UniqueConnection);
  connect(m_list, SIGNAL(request(QString, int)),
          this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void ObjectListWidget::fetchNewer() {
  emit request(m_list->prevLink(), m_asMode | QAS_NEWER);
}

//------------------------------------------------------------------------------

void ObjectListWidget::fetchOlder() {
  QString nextLink = m_list->nextLink();
  if (!nextLink.isEmpty())
    emit request(nextLink, m_asMode | QAS_OLDER);
}

//------------------------------------------------------------------------------

void ObjectListWidget::update() {
  ASWidget::update();
  fetchOlder();
}

//------------------------------------------------------------------------------

ObjectWidgetWithSignals*
ObjectListWidget::createWidget(QASAbstractObject* aObj, bool& countAsNew) {
  QASObject* obj = qobject_cast<QASObject*>(aObj);
  if (!obj) {
    qDebug() << "ERROR ObjectListWidget::createWidget passed non-object";
    return NULL;
  }

  ObjectWidget* ow = new ObjectWidget(obj, this);
  connect(obj, SIGNAL(request(QString, int)), 
          this, SIGNAL(request(QString, int)), Qt::UniqueConnection);

  countAsNew = true;
  return ow;
}

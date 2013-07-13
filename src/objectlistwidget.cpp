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
  m_objectList(NULL)
{}

//------------------------------------------------------------------------------

void ObjectListWidget::setEndpoint(QString endpoint) {
  clear();

  m_objectList = QASObjectList::initObjectList(endpoint,
                                               parent()->parent()->parent());
  connect(m_objectList, SIGNAL(changed(bool)),
          this, SLOT(update(bool)), Qt::UniqueConnection);
  connect(m_objectList, SIGNAL(request(QString, int)),
          this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void ObjectListWidget::fetchNewer() {
  emit request(m_objectList->prevLink(), QAS_OBJECTLIST | QAS_NEWER);
}

//------------------------------------------------------------------------------

void ObjectListWidget::fetchOlder() {
  QString nextLink = m_objectList->nextLink();
  if (!nextLink.isEmpty())
    emit request(nextLink, QAS_OBJECTLIST | QAS_OLDER);
}

//------------------------------------------------------------------------------

void ObjectListWidget::update(bool older) {
  /* 
     We assume m_objectList contains all objects, but new ones might
     have been added. Go through from top (newest) to bottom. Add any
     non-existing to top (going down from there).
  */

  int li = older ? m_itemLayout->count() : 0;
  int newCount = 0;

  for (size_t i=0; i<m_objectList->size(); i++) {
    QASObject* obj = m_objectList->at(i);
    if (obj->isDeleted())
      continue;

    if (m_object_set.contains(obj))
      continue;
    m_object_set.insert(obj);

    ObjectWidget* ow = new ObjectWidget(obj, this);
    ObjectWidgetWithSignals::connectSignals(ow, this);
    connect(obj, SIGNAL(request(QString, int)), 
            this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
    
    m_itemLayout->insertWidget(li++, ow);
    newCount++;
  }

  if (newCount && !isVisible() && !m_firstTime && !older)
    emit highlightMe();
  m_firstTime = false;

  fetchOlder();
}


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

#include "collectionwidget.h"
#include "pumpa_defines.h"
#include "activitywidget.h"

#include <QDebug>

//------------------------------------------------------------------------------

CollectionWidget::CollectionWidget(QWidget* parent) :
  ASWidget(parent),
  m_collection(NULL)
{}

//------------------------------------------------------------------------------

void CollectionWidget::setEndpoint(QString endpoint) {
  clear();

  m_collection = QASCollection::initCollection(endpoint,
                                               parent()->parent()->parent());
  connect(m_collection, SIGNAL(changed()), this, SLOT(update()),
          Qt::UniqueConnection);
  connect(m_collection, SIGNAL(request(QString, int)),
          this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void CollectionWidget::fetchNewer() {
  emit request(m_collection->prevLink(), QAS_COLLECTION | QAS_NEWER);
}

//------------------------------------------------------------------------------

void CollectionWidget::fetchOlder() {
  QString nextLink = m_collection->nextLink();
  if (!nextLink.isEmpty())
    emit request(nextLink, QAS_COLLECTION | QAS_OLDER);
}

//------------------------------------------------------------------------------

QASActivity* CollectionWidget::activityAt(int idx) {
  QLayoutItem* item = m_itemLayout->itemAt(idx);

  if (dynamic_cast<QWidgetItem*>(item)) {
    ActivityWidget* aw = qobject_cast<ActivityWidget*>(item->widget());
    if (aw)
      return aw->activity();
  }

  return NULL;
}

//------------------------------------------------------------------------------

void CollectionWidget::update() {
  /* 
     We assume m_collection contains all activities, but new ones
     might have been added either (or both) to the top or end. Go
     through from top (newest) to bottom. If the activity doesn't
     exist add it, if it does increment the counter (go further down
     both in the collection and widget list).
  */

  int li = 0; 
  int newCount = 0;

  for (size_t i=0; i<m_collection->size(); i++) {
    QASActivity* cAct = m_collection->at(i);

    QASObject* obj = cAct->object();
    if (obj->isDeleted())
      continue;

    QASActivity* wAct = activityAt(li);
    if (wAct == cAct) {
      li++;
      continue;
    }

    if (m_activity_set.contains(cAct)) {
      qDebug() << "THIS CAN'T BE HAPPENING";
      continue;
    }
    m_activity_set.insert(cAct);

    QString verb = cAct->verb();
    
    ActivityWidget* aw = new ActivityWidget(cAct, this);
    ObjectWidgetWithSignals::connectSignals(aw, this);
    connect(aw, SIGNAL(showContext(QASObject*)),
            this, SIGNAL(showContext(QASObject*)));

    if (obj)
      connect(obj, SIGNAL(request(QString, int)), 
              this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
    
    m_itemLayout->insertWidget(li++, aw);

    // m_shown_objects.insert(obj);

    if (!cAct->actor()->isYou())
      newCount++;
  }

  if (newCount && !isVisible() && !m_firstTime)
    emit highlightMe();
  m_firstTime = false;
}


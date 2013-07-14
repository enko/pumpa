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
  ASWidget(parent)
{}

//------------------------------------------------------------------------------

void CollectionWidget::setEndpoint(QString endpoint) {
  clear();

  m_list = QASCollection::initCollection(endpoint,
                                         parent()->parent()->parent());
  connect(m_list, SIGNAL(changed()), this, SLOT(update()),
          Qt::UniqueConnection);
  connect(m_list, SIGNAL(request(QString, int)),
          this, SIGNAL(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void CollectionWidget::fetchNewer() {
  emit request(m_list->prevLink(), QAS_COLLECTION | QAS_NEWER);
}

//------------------------------------------------------------------------------

void CollectionWidget::fetchOlder() {
  QString nextLink = m_list->nextLink();
  if (!nextLink.isEmpty())
    emit request(nextLink, QAS_COLLECTION | QAS_OLDER);
}

//------------------------------------------------------------------------------

ObjectWidgetWithSignals*
CollectionWidget::createWidget(QASAbstractObject* aObj, bool& countAsNew) {
  QASActivity* act = qobject_cast<QASActivity*>(aObj);
  if (!act) {
    qDebug() << "ERROR CollectionWidget::createWidget passed non-activity";
    return NULL;
  }

  ActivityWidget* aw = new ActivityWidget(act, this);
  connect(aw, SIGNAL(showContext(QASObject*)),
          this, SIGNAL(showContext(QASObject*)));

  QASObject* obj = act->object();
  if (obj)
    connect(obj, SIGNAL(request(QString, int)), 
            this, SIGNAL(request(QString, int)), Qt::UniqueConnection);

  countAsNew = !act->actor()->isYou();
  return aw;
}

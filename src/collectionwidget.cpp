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
#include "newactivitywidget.h"

#include <QDebug>

//------------------------------------------------------------------------------

CollectionWidget::CollectionWidget(QWidget* parent, bool shortDisplay) :
  QScrollArea(parent),
  m_firstTime(true),
  m_shortDisplay(shortDisplay),
  m_collection(NULL)
{
  m_itemLayout = new QVBoxLayout;
  m_itemLayout->setSpacing(10);
  m_itemLayout->addStretch();

  m_listContainer = new QWidget;
  m_listContainer->setLayout(m_itemLayout);
  m_listContainer->setSizePolicy(QSizePolicy::Ignored,
                                 QSizePolicy::Ignored);

  setWidget(m_listContainer);
  setWidgetResizable(true);
}

//------------------------------------------------------------------------------

void CollectionWidget::setEndpoint(QString endpoint) {
  QLayoutItem* item;
  while ((item = m_itemLayout->takeAt(0)) != 0) {
    if (dynamic_cast<QWidgetItem*>(item)) {
      QWidget* w = item->widget();
      delete w;
    }
    delete item;
  }

  m_firstTime = true;
  m_collection = QASCollection::initCollection(endpoint,
                                               parent()->parent()->parent());
  connect(m_collection, SIGNAL(changed(bool)), this, SLOT(update(bool)),
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

void CollectionWidget::refreshTimeLabels() {
  for (int i=0; i<m_itemLayout->count(); i++) {
    QLayoutItem* const li = m_itemLayout->itemAt(i);
    if (dynamic_cast<QWidgetItem*>(li)) {
      ActivityWidget* aw = qobject_cast<ActivityWidget*>(li->widget());
      if (aw)
        aw->refreshTimeLabels();
    }
  }
}

//------------------------------------------------------------------------------

void CollectionWidget::update(bool older) {
  /* 
     We assume m_collection contains all objects, but new ones might
     have been added. Go through from top (newest) to bottom. Add any
     non-existing to top (going down from there).
  */

  int li = older ? m_itemLayout->count() : 0;
  int newCount = 0;

  for (size_t i=0; i<m_collection->size(); i++) {
    QASActivity* activity = m_collection->at(i);

    if (m_activity_set.contains(activity))
      continue;
    m_activity_set.insert(activity);

    QASObject* obj = activity->object();
    QString verb = activity->verb();
    
    // bool full = verb == "post" ||
    //   (verb == "share" && !m_shown_objects.contains(obj));
    // bool full = false;

    NewActivityWidget* aw = new NewActivityWidget(activity, this);
    connect(aw, SIGNAL(linkHovered(const QString&)),
            this,  SIGNAL(linkHovered(const QString&)));
    
    m_itemLayout->insertWidget(li++, aw);

    // if (!full) {
    //   ShortActivityWidget* aw = new ShortActivityWidget(activity, this);
    //   connect(aw, SIGNAL(linkHovered(const QString&)),
    //           this,  SIGNAL(linkHovered(const QString&)));
      
    //   m_itemLayout->insertWidget(li++, aw);
    // } else {
    //   ActivityWidget* aw = new ActivityWidget(activity, this);
    //   connect(aw, SIGNAL(newReply(QASObject*)),
    //           this, SIGNAL(newReply(QASObject*)));
    //   connect(aw, SIGNAL(linkHovered(const QString&)),
    //           this,  SIGNAL(linkHovered(const QString&)));
    //   connect(aw, SIGNAL(like(QASObject*)), this, SIGNAL(like(QASObject*)));
    //   connect(aw, SIGNAL(share(QASObject*)), this, SIGNAL(share(QASObject*)));

    //   aw->updateText();

    //   m_itemLayout->insertWidget(li++, aw);
    // }
    
    m_shown_objects.insert(obj);

    if (!activity->actor()->isYou())
      newCount++;
  }

  if (newCount && !isVisible() && !m_firstTime)
    emit highlightMe();
  m_firstTime = false;
}

//------------------------------------------------------------------------------

void CollectionWidget::keyPressEvent(QKeyEvent* event) {
  int key = event->key();

  if (key == Qt::Key_Home || key == Qt::Key_End) {
    bool home = key==Qt::Key_Home;
    QScrollBar* sb = verticalScrollBar();
    sb->setValue(home ? sb->minimum() : sb->maximum());
  } else {
    QScrollArea::keyPressEvent(event);
  }
}

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

#include <QDebug>
// #include <QPalette>

//------------------------------------------------------------------------------

CollectionWidget::CollectionWidget(QWidget* parent) :
  QScrollArea(parent)
{

  // loadSignalMapper = new QSignalMapper(this);
  // connect(loadSignalMapper, SIGNAL(mapped(int)), this, SLOT(loadOlder(int)));
  
  m_itemLayout = new QVBoxLayout;
  m_itemLayout->setSpacing(10);
  m_itemLayout->addStretch();

  m_listContainer = new QWidget;
  m_listContainer->setLayout(m_itemLayout);

  setWidget(m_listContainer);
  setWidgetResizable(true);
}

//------------------------------------------------------------------------------

void CollectionWidget::addCollection(const QASCollection& coll) {
  int li = 0; // index into internal m_list

  for (size_t i=0; i<coll.size(); i++) {
    QASActivity* activity = coll.at(i);
    QString activity_id = activity->id();

    if (m_activity_map.contains(activity_id))
      continue; // do nothing, assume some signal will be emitted to
                // keep widget updated

    m_activity_map.insert(activity_id, activity);

    ActivityWidget* aw = new ActivityWidget(activity, this);

    m_itemLayout->insertWidget(li++, aw);

    // if (noteObj->hasReplies()) {
    //   const QASObjectList* ol = noteObj->replies();
    //   for (size_t j=0; j<ol->size(); j++) {
    //     QASObject* replyObj = ol->at(ol->size()-j-1);
    //     const QASActor* author = replyObj->author();

    //     ObjectWidget* ow = new ObjectWidget(this);
    //     ow->setText(QString("<p style=\"margin-left: 40px\">%1<br/>"
    //                         "%2 at <a href=\"%4\">%3</a></p>").
    //                 arg(replyObj->content()).
    //                 arg(author->displayName()).
    //                 arg(relativeFuzzyTime(replyObj->published())).
    //                 arg(replyObj->url()));
    //     m_itemLayout->insertWidget(li++, ow);
    //   }
    // }
  }
}

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

QString relativeFuzzyTime(QDateTime sTime) {
  QString dateStr = sTime.toString("ddd d MMMM yyyy");

  int secs = sTime.secsTo(QDateTime::currentDateTime().toUTC());
  if (secs < 0)
    secs = 0;
  int mins = qRound((float)secs/60);
  int hours = qRound((float)secs/60/60);
    
  if (secs < 60) { 
    dateStr = QString("a few seconds ago");
  } else if (mins < 60) {
    dateStr = QString("%1 minute%2 ago").arg(mins).arg(mins==1?"":"s");
  } else if (hours < 24) {
    dateStr = QString("%1 hour%2 ago").arg(hours).arg(hours==1?"":"s");
  }
  return dateStr;
}

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QWidget* parent, Qt::WindowFlags f) :
  QLabel(parent, f)
{
  setWordWrap(true);

  setOpenExternalLinks(true);
  setTextInteractionFlags(Qt::TextSelectableByMouse |
                          Qt::LinksAccessibleByMouse);
  setScaledContents(false);

  setLineWidth(2);
  setMargin(0);
  setFocusPolicy(Qt::NoFocus);

  // QPalette pal;
  // pal.setColor(QPalette::AlternateBase, Qt::white);
  // setPalette(pal);
}

//------------------------------------------------------------------------------

void ObjectWidget::mousePressEvent(QMouseEvent* e) {
  QLabel::mousePressEvent(e);
  e->ignore();
}


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
      
    QASObject* noteObj = activity->object();
    QASActor* actor = activity->actor();

    ObjectWidget* ow = new ObjectWidget(this);
    ow->setText(QString("<p>%1 at <a href=\"%3\">%2</a><br/>%4</p>").
                arg(actor->displayName()).
                arg(relativeFuzzyTime(activity->published())).
                arg(noteObj->url()).
                arg(noteObj->content()));
    ow->setBackgroundRole(QPalette::Base);
    m_itemLayout->insertWidget(li++, ow);

    if (noteObj->hasReplies()) {
      const QASObjectList* ol = noteObj->replies();
      for (size_t j=0; j<ol->size(); j++) {
        QASObject* replyObj = ol->at(ol->size()-j-1);
        const QASActor* author = replyObj->author();

        ObjectWidget* ow = new ObjectWidget(this);
        ow->setText(QString("<p style=\"margin-left: 40px\">%1<br/>"
                            "%2 at <a href=\"%4\">%3</a></p>").
                    arg(replyObj->content()).
                    arg(author->displayName()).
                    arg(relativeFuzzyTime(replyObj->published())).
                    arg(replyObj->url()));
        m_itemLayout->insertWidget(li++, ow);
      }
    }
  }
}

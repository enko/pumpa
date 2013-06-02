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

#include "activitywidget.h"

#include <QDebug>

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

ActivityWidget::ActivityWidget(QASActivity* a, QWidget* parent) :
  QFrame(parent),  m_hasMoreButton(NULL), m_activity(a)
{
  QASObject* noteObj = m_activity->object();

  m_objectWidget = new ObjectWidget(noteObj, parent);
  m_actorWidget = new ActorWidget(m_activity->actor(), parent);

  updateText();

  m_favourButton = new QToolButton();
  m_favourButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_favourButton->setFocusPolicy(Qt::NoFocus);
  updateFavourButton();
  m_favourButton->setEnabled(false);
  connect(m_favourButton, SIGNAL(clicked()), this, SLOT(favourite()));

  m_shareButton = new QToolButton();
  m_shareButton->setText(QChar(0x267A));
  m_shareButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_shareButton->setFocusPolicy(Qt::NoFocus);
  m_shareButton->setEnabled(false);
  connect(m_shareButton, SIGNAL(clicked()), this, SLOT(repeat()));

  m_commentButton = new QToolButton();
  m_commentButton->setText("comment");
  m_commentButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_commentButton->setFocusPolicy(Qt::NoFocus);
  // m_commentButton->setEnabled(false);
  connect(m_commentButton, SIGNAL(clicked()), this, SLOT(reply()));

  m_buttonLayout = new QHBoxLayout;
  m_buttonLayout->addStretch();
  m_buttonLayout->addWidget(m_favourButton, 0, Qt::AlignTop);
  // m_buttonLayout->addWidget(m_shareButton, 0, Qt::AlignTop);
  m_buttonLayout->addWidget(m_commentButton, 0, Qt::AlignTop);

  m_rightLayout = new QVBoxLayout;
  m_rightLayout->setContentsMargins(0, 0, 0, 0);
  m_rightLayout->addWidget(m_objectWidget);
  m_rightLayout->addLayout(m_buttonLayout);

  connect(noteObj, SIGNAL(changed()), this, SLOT(onObjectChanged()));

  QASObjectList* ol = noteObj->replies();
  Q_ASSERT(ol != NULL);
  connect(ol, SIGNAL(changed()), this, SLOT(onObjectChanged()));
    
  if (noteObj->hasReplies())
    addObjectList(ol);

  // m_rightFrame = new QFrame(this);
  // m_rightFrame->setLayout(m_rightLayout);

  QHBoxLayout* m_acrossLayout = new QHBoxLayout;
  m_acrossLayout->setSpacing(10);
  m_acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  // m_acrossLayout->addWidget(m_rightFrame, 0, Qt::AlignTop);
  m_acrossLayout->addLayout(m_rightLayout, 0); 

  setLayout(m_acrossLayout);

  // connect(msg, SIGNAL(hasUpdated()), this, SLOT(onMessageHasUpdated()));
}

//------------------------------------------------------------------------------

// void ActivityWidget::mousePressEvent(QMouseEvent* e) {
//   emit clickedStatus(msg->getId());
//   QFrame::mousePressEvent(e);
// }

//------------------------------------------------------------------------------

void ActivityWidget::updateFavourButton(bool wait) {
  QString text = false ? QChar(0x2605) : QChar(0x2606);
  if (wait)
    text = "...";
  m_favourButton->setText(text);
}

//------------------------------------------------------------------------------

void ActivityWidget::updateText() {
  const QASObject* noteObj = m_activity->object();
  const QASActor* actor = m_activity->actor();
  const QASActor* author = noteObj->author();
  
  if (author == NULL)
    author = actor;

  m_objectWidget->setText(QString("<p>%1 at <a href=\"%3\">%2</a><br/>%4</p>").
                          arg(author->displayName()).
                          arg(relativeFuzzyTime(noteObj->published())).
                          arg(noteObj->url()).
                          arg(noteObj->content()));
}

//------------------------------------------------------------------------------

void ActivityWidget::favourite() {
  updateFavourButton(true);
  // FIXME make favouritisin' request here
}

//------------------------------------------------------------------------------

void ActivityWidget::repeat() {
  // FIXME
}

//------------------------------------------------------------------------------

void ActivityWidget::reply() {
  emit newReply(m_activity->object());
}
 
//------------------------------------------------------------------------------

void ActivityWidget::onObjectChanged() {
  updateText();

  const QASObject* noteObj = m_activity->object();
  if (noteObj->hasReplies()) {
    QASObjectList* ol = noteObj->replies();
    addObjectList(ol);
  }
}

//------------------------------------------------------------------------------

void ActivityWidget::addObjectList(QASObjectList* ol) {
  int li = 2; // index where to insert next widget in the layout

  if (ol->hasMore() && (qulonglong)m_repliesList.size() < ol->totalItems()) {
    addHasMoreButton(ol, li++);
  } else if (m_hasMoreButton != NULL) {
    m_rightLayout->removeWidget(m_hasMoreButton);
    delete m_hasMoreButton;
    m_hasMoreButton = NULL;
  }

  /*
    For now we sort by time, or more accurately by whatever number the
    QASObject::sortInt() returns. Higher number is newer, goes further
    down the list.
  
    Comments' lists returned by the pump API are with newest at the
    top, so we start from the end, and can assume that the next one is
    always newer.
  */

  int i = 0; // index into m_repliesList
  for (size_t j=0; j<ol->size(); j++) {
    QASObject* replyObj = ol->at(ol->size()-j-1);
    QString replyId = replyObj->id();
    qint64 sortInt = replyObj->sortInt();

    // if (m_repliesSet.contains(replyId))
    //   continue;
    // m_repliesMap.insert(replyId, replyObj);

    while (i < m_repliesList.size() &&
           m_repliesList[i]->id() != replyId &&
           m_repliesList[i]->sortInt() < sortInt)
      i++;

    if (i < m_repliesList.size() && m_repliesList[i]->id() == replyId)
      continue;

    QASActor* author = replyObj->author();

    ActorWidget* aw = new ActorWidget(author, this, true);
    ObjectWidget* ow = new ObjectWidget(replyObj, this);
    
    ow->setText(QString("%1<br/>%2 at <a href=\"%4\">%3</a>").
                arg(replyObj->content()).
                arg(author->displayName()).
                arg(relativeFuzzyTime(replyObj->published())).
                arg(replyObj->url()));
      
    QHBoxLayout* replyLayout = new QHBoxLayout;
    replyLayout->setContentsMargins(0, 0, 0, 0);
    replyLayout->addWidget(aw, 0, Qt::AlignTop);
    replyLayout->addWidget(ow, 0, Qt::AlignTop);
    
    m_rightLayout->insertLayout(li + i, replyLayout);
    m_repliesList.insert(i, replyObj);
  }
}

//------------------------------------------------------------------------------

void ActivityWidget::addHasMoreButton(QASObjectList* ol, int li) {
  QString buttonText = QString("Show all %1 replies").
    arg(ol->totalItems());
  if (m_hasMoreButton == NULL) {
    m_hasMoreButton = new QPushButton(this);
    m_hasMoreButton->setFocusPolicy(Qt::NoFocus);
    m_rightLayout->insertWidget(li, m_hasMoreButton);
    connect(m_hasMoreButton, SIGNAL(clicked()), 
            this, SLOT(onHasMoreClicked()));
  }

  m_hasMoreButton->setText(buttonText);
}

//------------------------------------------------------------------------------

void ActivityWidget::onHasMoreClicked() {
  emit request(m_activity->object()->replies()->url(), QAS_FETCH_REPLIES);
}

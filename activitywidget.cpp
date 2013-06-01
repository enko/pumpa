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
  QFrame(parent), m_activity(a)
{
  m_objectWidget = new ObjectWidget(parent);
  m_objectWidget->setBackgroundRole(QPalette::Base);

  m_actorWidget = new ActorWidget(m_activity->actor(), parent);

  updateText();

  m_favourButton = new QToolButton();
  m_favourButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_favourButton->setFocusPolicy(Qt::NoFocus);
  updateFavourButton();
  connect(m_favourButton, SIGNAL(clicked()), this, SLOT(favourite()));

  m_repeatButton = new QToolButton();
  m_repeatButton->setText(QChar(0x267A));
  m_repeatButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_repeatButton->setFocusPolicy(Qt::NoFocus);
  connect(m_repeatButton, SIGNAL(clicked()), this, SLOT(repeat()));

  m_repeatButton = new QToolButton();
  m_repeatButton->setText("comment");
  m_repeatButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_repeatButton->setFocusPolicy(Qt::NoFocus);
  connect(m_repeatButton, SIGNAL(clicked()), this, SLOT(reply()));

  m_buttonLayout = new QHBoxLayout;
  m_buttonLayout->addStretch();
  m_buttonLayout->addWidget(m_favourButton, 0, Qt::AlignTop);
  m_buttonLayout->addWidget(m_repeatButton, 0, Qt::AlignTop);
  m_buttonLayout->addWidget(m_repeatButton, 0, Qt::AlignTop);

  m_rightLayout = new QVBoxLayout;
  m_rightLayout->addWidget(m_objectWidget);
  m_rightLayout->addLayout(m_buttonLayout);
  m_rightLayout->setContentsMargins(0, 0, 0, 0);

  QHBoxLayout* m_acrossLayout = new QHBoxLayout;
  m_acrossLayout->setSpacing(10);
  m_acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  m_acrossLayout->addLayout(m_rightLayout, 0); //, Qt::AlignTop);

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
  m_objectWidget->setText(QString("<p>%1 at <a href=\"%3\">%2</a><br/>%4</p>").
                          arg(actor->displayName()).
                          arg(relativeFuzzyTime(m_activity->published())).
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
  // FIXME
}
 

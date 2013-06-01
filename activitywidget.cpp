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

  updateText();

  favourButton = new QToolButton();
  favourButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  favourButton->setFocusPolicy(Qt::NoFocus);
  updateFavourButton();
  connect(favourButton, SIGNAL(clicked()), this, SLOT(favourite()));

  repeatButton = new QToolButton();
  repeatButton->setText(QChar(0x267A));
  repeatButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  repeatButton->setFocusPolicy(Qt::NoFocus);
  connect(repeatButton, SIGNAL(clicked()), this, SLOT(repeat()));

  replyButton = new QToolButton();
  replyButton->setText("comment");
  replyButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  replyButton->setFocusPolicy(Qt::NoFocus);
  connect(replyButton, SIGNAL(clicked()), this, SLOT(reply()));
  buttonLayout = new QHBoxLayout;

  buttonLayout->addStretch();
  buttonLayout->addWidget(favourButton, 0, Qt::AlignTop);
  buttonLayout->addWidget(repeatButton, 0, Qt::AlignTop);
  buttonLayout->addWidget(replyButton, 0, Qt::AlignTop);

  statusLayout = new QVBoxLayout;
  statusLayout->addWidget(m_objectWidget);
  statusLayout->addLayout(buttonLayout);

  statusLayout->setContentsMargins(0, 0, 0, 0);
  setLayout(statusLayout);

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
  favourButton->setText(text);
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
 

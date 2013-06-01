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

#ifndef ACTIVITYWIDGET_H
#define ACTIVITYWIDGET_H

#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QWidget>
#include <QMouseEvent>

#include "qactivitystreams.h"
#include "objectwidget.h"

#define MAX_WORD_LENGTH       40

//------------------------------------------------------------------------------

class ActivityWidget : public QFrame {
  Q_OBJECT

public:
  ActivityWidget(QASActivity* a, QWidget* parent=0);

  QString getId() const { return m_activity->id(); }

// protected:
//   void mousePressEvent(QMouseEvent* e);

//signals:
  // void replySignal(const QString&, message_id_t=-1);

  // void clickedStatus(message_id_t);
  // void requestReload();
  
public slots:
  void favourite();
  void repeat();
  void reply();

  //private slots:
  // void onStatusReady(StatusMessage*);
  // void onMessageHasUpdated();
  // void onUrlReady(const QString&, const QString&);

private:
  void updateFavourButton(bool wait = false);
  void updateText();

  ObjectWidget* m_objectWidget;
  // ActorWidget* m_actorWidget;

  QToolButton* favourButton;
  QToolButton* repeatButton;
  QToolButton* replyButton;

  QHBoxLayout* buttonLayout;
  QVBoxLayout* statusLayout;
  
  QASActivity* m_activity;
};

#endif 

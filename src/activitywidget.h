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
#include <QPushButton>

#include "qactivitystreams.h"
#include "objectwidget.h"
#include "actorwidget.h"
#include "richtextlabel.h"

#define MAX_WORD_LENGTH       40

//------------------------------------------------------------------------------

class ActivityWidget : public QFrame {
  Q_OBJECT

public:
  ActivityWidget(QASActivity* a, QWidget* parent=0);

  QString getId() const { return m_activity->id(); }
  
public slots:
  void favourite();
  void repeat();
  void reply();

  void onObjectChanged();
  void onHasMoreClicked();

signals:
  void request(QString, int);
  void newReply(QASObject*);
  void linkHovered(const QString&);

private:
  QASActor* effectiveAuthor();

  void addHasMoreButton(QASObjectList* ol, int li);
  void updateFavourButton(bool wait = false);
  void updateText();
  void addObjectList(QASObjectList* ol);

  RichTextLabel* m_infoLabel;
  ObjectWidget* m_objectWidget;
  ActorWidget* m_actorWidget;

  QToolButton* m_favourButton;
  QToolButton* m_shareButton;
  QToolButton* m_commentButton;

  QHBoxLayout* m_buttonLayout;
  QVBoxLayout* m_rightLayout;
  QHBoxLayout* m_acrossLayout;
  
  QFrame* m_rightFrame;

  QPushButton* m_hasMoreButton;

  QASActivity* m_activity;

  QList<QASObject*> m_repliesList;
};

#endif 

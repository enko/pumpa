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

#ifndef _NEWACTIVITYWIDGET_H_
#define _NEWACTIVITYWIDGET_H_

#include <QHBoxLayout>
#include <QFrame>
#include <QWidget>

#include "qactivitystreams.h"
// #include "actorwidget.h"
#include "richtextlabel.h"
#include "shortactivitywidget.h"
#include "objectwidget.h"

//------------------------------------------------------------------------------

class NewActivityWidget : public AbstractActivityWidget {
  Q_OBJECT

public:
  NewActivityWidget(QASActivity* a, QWidget* parent=0);

  void refreshTimeLabels();

public slots:
  virtual void onObjectChanged();

signals:
  void linkHovered(const QString&);
  void newReply(QASObject*);
  void like(QASObject*);
  void share(QASObject*);

private:
  void updateText();
  QString recipientsToString(QASObjectList* rec);

  RichTextLabel* m_textLabel;
  ActorWidget* m_actorWidget;
  ObjectWidget* m_objectWidget;
};

#endif /* _NEWACTIVITYWIDGET_H_ */

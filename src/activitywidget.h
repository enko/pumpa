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

#ifndef _ACTIVITYWIDGET_H_
#define _ACTIVITYWIDGET_H_

#include <QFrame>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>

#include "richtextlabel.h"
#include "qactivitystreams.h"
#include "objectwidget.h"
#include "objectwidgetwithsignals.h"

//------------------------------------------------------------------------------

class ActivityWidget : public ObjectWidgetWithSignals {
  Q_OBJECT

public:
  ActivityWidget(QASActivity* a, QWidget* parent=0);
  virtual ~ActivityWidget();

  virtual void changeObject(QASAbstractObject* obj);

  virtual QString getId() const { return m_activity->id(); }
  QASActivity* activity() const { return m_activity; }

  virtual QASAbstractObject* asObject() const { return activity(); }

  virtual void refreshTimeLabels();

public slots:
  virtual void onObjectChanged();

signals:
  void showContext(QASObject*);

private:
  void updateText();
  QString recipientsToString(QASObjectList* rec);
  // ObjectWidget* makeObjectWidgetAndConnect(QASObject* obj, bool shortObject);

  RichTextLabel* m_textLabel;
  ActorWidget* m_actorWidget;
  ObjectWidget* m_objectWidget;

  QASActivity* m_activity;
};

#endif /* _ACTIVITYWIDGET_H_ */

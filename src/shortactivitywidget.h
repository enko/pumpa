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

#ifndef _SHORTACTIVITYWIDGET_H_
#define _SHORTACTIVITYWIDGET_H_

#include <QHBoxLayout>
#include <QFrame>
#include <QWidget>

#include "qactivitystreams.h"
#include "actorwidget.h"
#include "richtextlabel.h"

//------------------------------------------------------------------------------

class AbstractActivityWidget : public QFrame {
  Q_OBJECT

public:
  AbstractActivityWidget(QASActivity* a, QWidget* parent=0);

  virtual QString getId() const { return m_activity->id(); }

protected:
  QASActivity* m_activity;
};

//------------------------------------------------------------------------------

class ShortActivityWidget : public AbstractActivityWidget {
  Q_OBJECT

public:
  ShortActivityWidget(QASActivity* a, QWidget* parent=0);

public slots:
  virtual void onObjectChanged();

signals:
  void linkHovered(const QString&);

private:
  void updateText();

  RichTextLabel* m_textLabel;
  ActorWidget* m_actorWidget;
  QHBoxLayout* m_acrossLayout;
};

#endif /* _SHORTACTIVITYWIDGET_H_ */

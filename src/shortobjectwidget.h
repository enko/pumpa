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

#ifndef _SHORTOBJECTWIDGET_H_
#define _SHORTOBJECTWIDGET_H_

#include <QFrame>
#include <QWidget>

#include "qactivitystreams.h"
#include "richtextlabel.h"
#include "actorwidget.h"

//------------------------------------------------------------------------------

class ShortObjectWidget : public QFrame {
  Q_OBJECT

public:
  ShortObjectWidget(QASObject* obj, QWidget* parent = 0);

  QASObject* object() const { return m_object; }

  static QString objectExcerpt(QASObject* obj);

  void refreshTimeLabels() {};

signals:
  void moreClicked();

private slots:
  void onChanged();

private:
  void updateText();
  void updateAvatar();

  RichTextLabel* m_textLabel;
  ActorWidget* m_actorWidget;
  QASObject* m_object;
  QASActor* m_actor;
};

#endif /* _SHORTOBJECTWIDGET_H_ */

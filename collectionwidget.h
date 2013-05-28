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

#ifndef _COLLECTIONWIDGET_H_
#define _COLLECTIONWIDGET_H_

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

#include "qactivitystreams.h"

//------------------------------------------------------------------------------

class ObjectWidget : public QLabel {
public:
  ObjectWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

protected:
  void mousePressEvent(QMouseEvent* e);
};

//------------------------------------------------------------------------------


class CollectionWidget : public QScrollArea {
  Q_OBJECT

public:
  CollectionWidget(QWidget* parent=0);

  void setCollection(const QASCollection& coll);

private:
  QVBoxLayout* itemLayout;
  QWidget* listContainer;
};

#endif /* _COLLECTIONWIDGET_H_ */
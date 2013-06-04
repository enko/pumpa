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
#include "activitywidget.h"

//------------------------------------------------------------------------------

class CollectionWidget : public QScrollArea {
  Q_OBJECT

public:
  CollectionWidget(QWidget* parent=0);

  void addCollection(const QASCollection& coll);

signals:
  void request(QString, int);
  void newReply(QASObject*);
  void linkHovered(const QString&);

protected:
  void keyPressEvent(QKeyEvent* event);

private:
  QVBoxLayout* m_itemLayout;
  QWidget* m_listContainer;
  // QList<QASActivity*> m_list;
  QMap<QString, QASActivity*> m_activity_map;
  
  QString m_nextUrl;
};

#endif /* _COLLECTIONWIDGET_H_ */

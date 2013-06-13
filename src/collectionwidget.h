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

#include "activitywidget.h"
#include "qactivitystreams.h"

#include <QLabel>
#include <QWidget>
#include <QScrollBar>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>

//------------------------------------------------------------------------------

class CollectionWidget : public QScrollArea {
  Q_OBJECT

public:
  CollectionWidget(QWidget* parent, bool shortDisplay=false);
  void setEndpoint(QString endpoint);
  void fetchNewer();
  void fetchOlder();

signals:
  void highlightMe();  
  void request(QString, int);
  void newReply(QASObject*);
  void linkHovered(const QString&);
  void like(QASObject*);
  void share(QASObject*);

protected:
  void keyPressEvent(QKeyEvent* event);

private slots:
  void update(bool older);

private:
  QVBoxLayout* m_itemLayout;
  QWidget* m_listContainer;
  QSet<QASActivity*> m_activity_set;
  
  bool m_firstTime;
  bool m_shortDisplay;

  QSet<QASObject*> m_shown_objects;

  QASCollection* m_collection;
};

#endif /* _COLLECTIONWIDGET_H_ */

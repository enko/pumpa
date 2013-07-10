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

#include "qactivitystreams.h"

// #include <QLabel>
// #include <QWidget>
// #include <QScrollBar>
// #include <QMouseEvent>
// #include <QScrollArea>
// #include <QVBoxLayout>

#include "aswidget.h"

//------------------------------------------------------------------------------

class CollectionWidget : public ASWidget {
  Q_OBJECT

public:
  CollectionWidget(QWidget* parent);
  void setEndpoint(QString endpoint);
  void fetchNewer();
  void fetchOlder();
  // void refreshTimeLabels();

private slots:
  void update(bool older);

private:
  QSet<QASActivity*> m_activity_set;
  QSet<QASObject*> m_shown_objects;

  QASCollection* m_collection;
};

#endif /* _COLLECTIONWIDGET_H_ */

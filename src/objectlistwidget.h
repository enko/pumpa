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

#ifndef _OBJECTLISTWIDGET_H_
#define _OBJECTLISTWIDGET_H_

#include "qactivitystreams.h"

#include "aswidget.h"

//------------------------------------------------------------------------------

class ObjectListWidget : public ASWidget {
  Q_OBJECT

public:
  ObjectListWidget(QWidget* parent);
  void setEndpoint(QString endpoint);
  void fetchNewer();
  void fetchOlder();

private slots:
  void update(bool older=false);

private:
  QSet<QASObject*> m_object_set;

  QASObjectList* m_objectList;
};

#endif /* _OBJECTLISTWIDGET_H_ */

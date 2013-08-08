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

  QASObjectList* objectList() const;

protected:
  virtual QASAbstractObjectList* initList(QString endpoint, QObject* parent);
  virtual void update();
  virtual ObjectWidgetWithSignals* createWidget(QASAbstractObject* aObj);
};

#endif /* _OBJECTLISTWIDGET_H_ */

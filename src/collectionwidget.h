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
#include "activitywidget.h"
#include "aswidget.h"

//------------------------------------------------------------------------------

class CollectionWidget : public ASWidget {
  Q_OBJECT

public:
  CollectionWidget(QWidget* parent, int widgetLimit=-1, int purgeWait=10);

  // virtual void fetchNewer();

protected slots:
  virtual void update();
  void onLoadOlderClicked();

protected:
  void updateLoadOlderButton(bool wait=false);
  virtual QASAbstractObjectList* initList(QString endpoint, QObject* parent);
  virtual ObjectWidgetWithSignals* createWidget(QASAbstractObject* aObj);
  virtual bool countAsNew(QASAbstractObject* aObj);
  virtual void clear();

  QPushButton* m_loadOlderButton;
};

#endif /* _COLLECTIONWIDGET_H_ */

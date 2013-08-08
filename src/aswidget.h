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

#ifndef _ASWIDGET_H_
#define _ASWIDGET_H_

#include "qactivitystreams.h"
#include "objectwidgetwithsignals.h"

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

//------------------------------------------------------------------------------

class ASWidget : public QScrollArea {
  Q_OBJECT

public:
  ASWidget(QWidget* parent, int widgetLimit=-1, int purgeWait=10);
  virtual void refreshTimeLabels();
  virtual void fetchNewer();
  virtual void fetchOlder();
  void setEndpoint(QString endpoint, QObject* parent, int asMode=-1);

  int count() const { return m_object_set.size(); }

signals:
  void highlightMe();  
  void request(QString, int);
  void newReply(QASObject*);
  void linkHovered(const QString&);
  void like(QASObject*);
  void share(QASObject*);
  void showContext(QASObject*);
  void follow(QString, bool);
  void deleteObject(QASObject*);

protected slots:
  virtual void update();

protected:
  virtual QASAbstractObjectList* initList(QString endpoint, QObject* parent);

  QASAbstractObject* objectAt(int idx);
  ObjectWidgetWithSignals* widgetAt(int idx);
  virtual ObjectWidgetWithSignals* createWidget(QASAbstractObject* aObj);
  virtual bool countAsNew(QASAbstractObject*) { return true; }

  void keyPressEvent(QKeyEvent* event);
  virtual void clear();

  void refreshObject(QASAbstractObject* obj);

  QVBoxLayout* m_itemLayout;
  QWidget* m_listContainer;
  bool m_firstTime;

  QSet<QASAbstractObject*> m_object_set;
  QASAbstractObjectList* m_list;

  int m_asMode;

  bool m_reuseWidgets;
  int m_purgeWait;
  int m_purgeCounter;
  int m_widgetLimit;
};

#endif /* _ASWIDGET_H_ */

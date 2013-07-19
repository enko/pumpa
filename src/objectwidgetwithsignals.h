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

#ifndef _OBJECTWIDGETWITHSIGNALS_H_
#define _OBJECTWIDGETWITHSIGNALS_H_

#include <QFrame>
#include "qactivitystreams.h"

//------------------------------------------------------------------------------

class ObjectWidgetWithSignals : public QFrame {
  Q_OBJECT

public:
  ObjectWidgetWithSignals(QWidget* parent = 0);

  virtual QASAbstractObject* asObject() const = 0;

  static void connectSignals(ObjectWidgetWithSignals* ow, QWidget* w);

  virtual void refreshTimeLabels() = 0;
  
signals:
  void linkHovered(const QString&);
  void like(QASObject*);
  void share(QASObject*);
  void newReply(QASObject*);
  void follow(QString, bool);
  void deleteObject(QASObject*);
};

#endif /* _OBJECTWIDGETWITHSIGNALS_H_ */

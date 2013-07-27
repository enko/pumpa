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

#include "objectwidgetwithsignals.h"

//------------------------------------------------------------------------------

ObjectWidgetWithSignals::ObjectWidgetWithSignals(QWidget* parent) :
  QFrame(parent) {}

//------------------------------------------------------------------------------

void ObjectWidgetWithSignals::connectSignals(ObjectWidgetWithSignals* ow, 
                                             QWidget* w) 
{
  connect(ow, SIGNAL(linkHovered(const QString&)),
          w, SIGNAL(linkHovered(const QString&)));
  connect(ow, SIGNAL(like(QASObject*)),
          w, SIGNAL(like(QASObject*)));
  connect(ow, SIGNAL(share(QASObject*)),
          w, SIGNAL(share(QASObject*)));
  connect(ow, SIGNAL(newReply(QASObject*)),
          w, SIGNAL(newReply(QASObject*)));
  connect(ow, SIGNAL(follow(QString, bool)),
          w, SIGNAL(follow(QString, bool)));
  connect(ow, SIGNAL(deleteObject(QASObject*)),
          w, SIGNAL(deleteObject(QASObject*)));
  connect(ow, SIGNAL(request(QString, int)),
          w, SIGNAL(request(QString, int)));
}

//------------------------------------------------------------------------------

void ObjectWidgetWithSignals::disconnectSignals(ObjectWidgetWithSignals* ow, 
                                                QWidget* w) 
{
  disconnect(ow, SIGNAL(linkHovered(const QString&)),
             w, SIGNAL(linkHovered(const QString&)));
  disconnect(ow, SIGNAL(like(QASObject*)),
             w, SIGNAL(like(QASObject*)));
  disconnect(ow, SIGNAL(share(QASObject*)),
             w, SIGNAL(share(QASObject*)));
  disconnect(ow, SIGNAL(newReply(QASObject*)),
             w, SIGNAL(newReply(QASObject*)));
  disconnect(ow, SIGNAL(follow(QString, bool)),
             w, SIGNAL(follow(QString, bool)));
  disconnect(ow, SIGNAL(deleteObject(QASObject*)),
             w, SIGNAL(deleteObject(QASObject*)));
  disconnect(ow, SIGNAL(request(QString, int)),
             w, SIGNAL(request(QString, int)));
}

//------------------------------------------------------------------------------

void ObjectWidgetWithSignals::refreshObject(QASAbstractObject* obj) {
  if (!obj)
    return;
  
  QDateTime now = QDateTime::currentDateTime();
  QDateTime lr = obj->lastRefreshed();

  if (lr.isNull() || lr.secsTo(now) > 10) {
    emit request(obj->apiLink(), obj->asType());
    obj->lastRefreshed(now);
  }
}

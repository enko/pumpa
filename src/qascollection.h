/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _QASCOLLECTION_H_
#define _QASCOLLECTION_H_

#include "qasactivity.h"
#include "qasabstractobject.h"
#include "qasabstractobjectlist.h"

//------------------------------------------------------------------------------

class QASCollection : public QASAbstractObjectList {
  Q_OBJECT

protected:
  QASCollection(QString url, QObject* parent);

public:
  static void clearCache();

  static QASCollection* initCollection(QString url, QObject* parent);
  static QASCollection* getCollection(QVariantMap json, QObject* parent,
                                      int id);

  QASActivity* at(size_t i) const {
    return qobject_cast<QASActivity*>(QASAbstractObjectList::at(i));
  }

private:
  virtual QASAbstractObject* getAbstractObject(QVariantMap json,
                                               QObject* parent);

  static QMap<QString, QASCollection*> s_collections;
};

#endif /* _QASCOLLECTION_H_ */

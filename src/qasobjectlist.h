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

#ifndef _QASOBJECTLIST_H_
#define _QASOBJECTLIST_H_

#include "qasabstractobject.h"
#include "qasabstractobjectlist.h"
#include "qasobject.h"

//------------------------------------------------------------------------------

class QASObjectList : public QASAbstractObjectList {
  Q_OBJECT

protected:
  QASObjectList(QString url, QObject* parent);

public:
  virtual void update(QVariantMap json, bool older);

  static void clearCache();

  static QASObjectList* initObjectList(QString url, QObject* parent);

  static QASObjectList* getObjectList(QVariantMap json, QObject* parent, 
                                      int id=0);
  static QASObjectList* getObjectList(QVariantList json, QObject* parent, 
                                      int id=0);

  QASObject* at(size_t i) const {
    return qobject_cast<QASObject*>(QASAbstractObjectList::at(i));
  }

  void isReplies(bool b) { m_isReplies = b; }

protected:
  virtual QASAbstractObject* getAbstractObject(QVariantMap json,
                                               QObject* parent);

private:
  static QMap<QString, QASObjectList*> s_objectLists;
  bool m_isReplies;
};

#endif /* _QASOBJECTLIST_H_ */

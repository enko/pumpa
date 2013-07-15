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

#ifndef _QASABSTRACTOBJECTLIST_H_
#define _QASABSTRACTOBJECTLIST_H_

#include "qasabstractobject.h"

#include <QSet>

//------------------------------------------------------------------------------

class QASAbstractObjectList : public QASAbstractObject {
  Q_OBJECT

protected:
  QASAbstractObjectList(int asType, QString url, QObject* parent);

public:
  virtual void update(QVariantMap json, bool older);

  QString prevLink() const { 
    return m_prevLink.isEmpty() ? m_url : m_prevLink; 
  }
  QString nextLink() const { return m_nextLink; }

  size_t size() const { return m_items.size(); }

  QASAbstractObject* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

  qulonglong totalItems() const { return m_totalItems; }
  QString url() const { return m_url; }
  QString urlOrProxy() const;
  virtual QString apiLink() const { return urlOrProxy(); }
  bool hasMore() const { return m_hasMore; }

  void addObject(QASAbstractObject*);
  void removeObject(QASAbstractObject*);
  bool contains(QASAbstractObject* obj) const {
    return m_item_set.contains(obj);
  }

protected:
  virtual QASAbstractObject* getAbstractObject(QVariantMap json,
                                               QObject* parent) = 0;

  QString m_displayName;
  QString m_url;
  qulonglong m_totalItems;
  QString m_proxyUrl;

  bool m_hasMore;

  QList<QASAbstractObject*> m_items;
  QSet<QASAbstractObject*> m_item_set;

  QString m_prevLink, m_nextLink;

  bool m_firstTime;
};

#endif /* _QASABSTRACTOBJECTLIST_H_ */

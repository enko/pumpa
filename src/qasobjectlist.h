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
#include "qasobject.h"

#include <QSet>

//------------------------------------------------------------------------------

class QASObjectList : public QASAbstractObject {
  Q_OBJECT

protected:
  QASObjectList(QString url, QObject* parent);

public:
  static void clearCache();

  static QASObjectList* initObjectList(QString url, QObject* parent);

  static QASObjectList* getObjectList(QVariantMap json, QObject* parent, 
                                      int id=0);
  static QASObjectList* getObjectList(QVariantList json, QObject* parent, 
                                      int id=0);
  void update(QVariantMap json, bool older);

  void addObject(QASObject* obj);

  size_t size() const { return m_items.size(); }
  qulonglong totalItems() const { return m_totalItems; }
  bool hasMore() const { return m_hasMore; }
  QString url() const { return m_url; }
  QString urlOrProxy() const;
  virtual QString apiLink() const { return urlOrProxy(); }
  // virtual void refresh();

  virtual QASObject* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

  bool contains(QASObject* obj) const {
    return m_items.contains(obj);
  }

  QString nextLink() const { return m_nextLink; }
  QString prevLink() const { 
    return m_prevLink.isEmpty() ? m_url : m_prevLink; 
  }

signals:
  void changed(bool);

protected:
  QString m_url;
  QString m_proxyUrl;
  qulonglong m_totalItems;
  QList<QASObject*> m_items;
  QSet<QASObject*> m_item_set;
  bool m_hasMore;
  QString m_nextLink;
  QString m_prevLink;

private:
  static QMap<QString, QASObjectList*> s_objectLists;
};

#endif /* _QASOBJECTLIST_H_ */

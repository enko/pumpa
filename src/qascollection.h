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

#include "qasabstractobject.h"
#include "qasactivity.h"

#include <QSet>

//------------------------------------------------------------------------------

class QASCollection : public QASAbstractObject {
  Q_OBJECT

protected:
  QASCollection(QString url, QObject* parent);

public:
  static void clearCache();

  static QASCollection* initCollection(QString url, QObject* parent);
  static QASCollection* getCollection(QVariantMap json, QObject* parent,
                                      int id);
  void update(QVariantMap json, bool older);

  QString prevLink() const { 
    return m_prevLink.isEmpty() ? m_url : m_prevLink; 
  }
  QString nextLink() const { return m_nextLink; }

  size_t size() const { return m_items.size(); }

  QASActivity* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

signals:
  void changed(bool);

private:
  QString m_displayName;
  QString m_url;
  qulonglong m_totalItems;
  QList<QASActivity*> m_items;
  QSet<QASActivity*> m_item_set;

  QString m_prevLink, m_nextLink;

  static QMap<QString, QASCollection*> s_collections;
};

#endif /* _QASCOLLECTION_H_ */

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

#include "qascollection.h"

#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QMap<QString, QASCollection*> QASCollection::s_collections;

void QASCollection::clearCache() { deleteMap<QASCollection*>(s_collections); }

//------------------------------------------------------------------------------

QASCollection::QASCollection(QString url, QObject* parent) :
  QASAbstractObjectList(QAS_COLLECTION, url, parent)
{
#ifdef DEBUG_QAS
  qDebug() << "new Collection" << m_url;
#endif
}

//------------------------------------------------------------------------------

QASAbstractObject* QASCollection::getAbstractObject(QVariantMap json,
                                                    QObject* parent) {
  return QASActivity::getActivity(json, parent);
}

//------------------------------------------------------------------------------

QASCollection* QASCollection::getCollection(QVariantMap json, QObject* parent,
                                            int id) {
  QString url = json["url"].toString();
  if (url.isEmpty())
     url = json["id"].toString();
  // if (url.isEmpty())
  //   return NULL;

  QASCollection* coll = s_collections.contains(url) ? s_collections[url] :
    new QASCollection(url, parent);
  s_collections.insert(url, coll);

  coll->update(json, id & QAS_OLDER);
  return coll;
}

//------------------------------------------------------------------------------

QASCollection* QASCollection::initCollection(QString url, QObject* parent) {
  if (s_collections.contains(url))
    return s_collections[url];
  
  QASCollection* coll = new QASCollection(url, parent);
  s_collections.insert(url, coll);

  return coll;
}

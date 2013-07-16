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

#include "qasobjectlist.h"

#include "qasactor.h"
#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QMap<QString, QASObjectList*> QASObjectList::s_objectLists;
void QASObjectList::clearCache() { deleteMap<QASObjectList*>(s_objectLists); }

//------------------------------------------------------------------------------

QASObjectList::QASObjectList(QString url, QObject* parent) :
  QASAbstractObjectList(QAS_OBJECTLIST, url, parent),
  m_isReplies(false)
{
#ifdef DEBUG_QAS
  qDebug() << "new ObjectList" << m_url;
#endif
}

//------------------------------------------------------------------------------

QASAbstractObject* QASObjectList::getAbstractObject(QVariantMap json,
                                                    QObject* parent) {
  if (json["objectType"].toString() == "person")
    return QASActor::getActor(json, parent);
  return QASObject::getObject(json, parent);
}


//------------------------------------------------------------------------------

QASObjectList* QASObjectList::initObjectList(QString url, QObject* parent) {
  if (s_objectLists.contains(url))
    return s_objectLists[url];
  
  QASObjectList* ol = new QASObjectList(url, parent);
  s_objectLists.insert(url, ol);

  return ol;
}

//------------------------------------------------------------------------------

QASObjectList* QASObjectList::getObjectList(QVariantMap json, QObject* parent,
                                            int id) {
  QString url = json["url"].toString();
  // if (url.isEmpty())
  //   return NULL;

  QASObjectList* ol = s_objectLists.contains(url) ? s_objectLists[url] :
    new QASObjectList(url, parent);
  if (!url.isEmpty())
    s_objectLists.insert(url, ol);

  ol->update(json, id & QAS_OLDER);
  return ol;
}

//------------------------------------------------------------------------------

QASObjectList* QASObjectList::getObjectList(QVariantList json, QObject* parent,
                                            int id) {
  QVariantMap jmap;
  jmap["totalItems"] = json.size();
  jmap["items"] = json;

  return getObjectList(jmap, parent, id);
}

//------------------------------------------------------------------------------

void QASObjectList::update(QVariantMap json, bool older) {
  if (m_isReplies && json.contains("items")) {
    m_item_set.clear();
    m_items.clear();
  }
  QASAbstractObjectList::update(json, older);
}

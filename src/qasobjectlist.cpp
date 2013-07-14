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

//------------------------------------------------------------------------------

QMap<QString, QASObjectList*> QASObjectList::s_objectLists;
void QASObjectList::clearCache() { deleteMap<QASObjectList*>(s_objectLists); }

//------------------------------------------------------------------------------

QASObjectList::QASObjectList(QString url, QObject* parent) :
  QASAbstractObjectList(QAS_OBJECTLIST, url, parent),
  m_hasMore(false)
{
#if DEBUG >= 1
  qDebug() << "new ObjectList" << m_url;
#endif
}

//------------------------------------------------------------------------------

// void QASObjectList::update(QVariantMap json, bool older) {
// #if DEBUG >= 1
//   qDebug() << "updating ObjectList" << m_url;
// #endif
//   bool ch = false;

//   updateVar(json, m_totalItems, "totalItems", ch);
//   updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

//   m_nextLink = "";
//   updateVar(json, m_nextLink, "links", "next", "href", ch);
//   updateVar(json, m_prevLink, "links", "prev", "href", ch);


//   if (json.contains("items")) {
//     // m_items.clear();
//     QVariantList items_json = json["items"].toList();
//     for (int i=0; i<items_json.count(); i++) {
//       QVariantMap item = items_json[i].toMap();
//       QASObject* obj;
//       if (item["objectType"].toString() == "person")
//         obj = QASActor::getActor(item, parent());
//       else
//         obj = QASObject::getObject(item, parent());

//       if (m_item_set.contains(obj))
//         continue;

//       m_items.append(obj);
//       m_item_set.insert(obj);

//       connectSignals(obj, false, true);
//       ch = true;
//     }
//   }
//   // set to false if number of items < total, and if we have already
//   // fetched it - that seems to have a displayName element
//   // ^^ FFFUUUGLY HACK !!!
//   // bool old_hasMore = m_hasMore;
//   m_hasMore = !json.contains("displayName") && size() < m_totalItems;
//   // if (!old_hasMore && m_hasMore)
//   //   qDebug() << "[DEBUG]: set hasMore" << m_url << m_proxyUrl << urlOrProxy();

//   if (ch)
//     emit changed(older);
// }

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

void QASObjectList::addObject(QASObject* obj) {
  if (m_items.contains(obj))
    return;
  m_items.append(obj);
  m_totalItems++;
  emit changed(false);
}



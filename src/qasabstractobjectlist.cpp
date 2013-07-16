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

#include "qasabstractobjectlist.h"

#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QASAbstractObjectList::QASAbstractObjectList(int asType, QString url,
                                             QObject* parent) :
  QASAbstractObject(asType, parent),
  m_url(url),
  m_totalItems(0),
  m_hasMore(false),
  m_firstTime(true)
{}

//------------------------------------------------------------------------------

void QASAbstractObjectList::update(QVariantMap json, bool older) {
#ifdef DEBUG_QAS
  qDebug() << "updating AbstractObjectList" << m_url;
#endif

  bool ch = false;
  bool dummy = false;

  updateVar(json, m_displayName, "displayName", ch);
  updateVar(json, m_totalItems, "totalItems", ch, true);
  updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

  // In pump.io the next link goes "next" in the UI, i.e. to older
  // stuff, so:
  // next => older
  // prev => newer
  //
  // If we are loading older stuff, update only the next link. If it
  // is empty it means we have reached the oldest stuff.
  //
  // If we are loading newer stuff, update only the prev link, except
  // if it is empty, then don't touch it. (That means there's no newer
  // stuff, but there's bound to be more later :-)
  //
  // And a special case is when we load it the first time, then both
  // next and prev links should be updated.
  
  if (older || m_firstTime) {
    m_nextLink = ""; // it's left as empty if it doesn't exist in the
                     // json
    updateVar(json, m_nextLink, "links", "next", "href", dummy);
  } 
  if (!older || m_firstTime) {
    // updateVar doesn't touch it if it is empty in the json
    updateVar(json, m_prevLink, "links", "prev", "href", dummy);
  }

  // We assume that collections come in as newest first, so we add
  // items starting from the top going downwards. Or if older=true
  // starting from the end and going downwards (appending).

  // Start adding from the top or bottom, depending on value of older.
  int mi = older ? m_items.size() : 0;

  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASAbstractObject* obj = getAbstractObject(items_json.at(i).toMap(),
                                               parent());
    if (mi < m_items.size()) {
      QASAbstractObject* mObj = m_items[mi];
      if (mObj == obj) {
        mi++;
        continue;
      }
    }

    if (m_item_set.contains(obj))
      continue;

    m_items.insert(mi++, obj);
    m_item_set.insert(obj);
    connectSignals(obj, false, true);

    ch = true;
  }

  // In theory, there should be more to be fetched if size <
  // totalItems.  Sometimes those missing items still do not appear in
  // the fetched list, and we will have a perpetual "load more"
  // button. It turns out that the fetched replies lists has a
  // displayName, while the short one has not... this is a very ugly
  // hack indeed :)
  m_hasMore = !json.contains("displayName") && size() < m_totalItems;

  m_firstTime = false;
  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QString QASAbstractObjectList::urlOrProxy() const {
  return m_proxyUrl.isEmpty() ? m_url : m_proxyUrl; 
}

//------------------------------------------------------------------------------

void QASAbstractObjectList::addObject(QASAbstractObject* obj) {
  if (m_item_set.contains(obj))
    return;

#ifdef DEBUG_QAS
  qDebug() << "addObject" << obj->apiLink();
#endif

  m_items.append(obj);
  m_item_set.insert(obj);

  // m_totalItems++;
  emit changed();
}

//------------------------------------------------------------------------------

void QASAbstractObjectList::removeObject(QASAbstractObject* obj) {
  m_items.removeAll(obj);
  m_item_set.remove(obj);

#ifdef DEBUG_QAS
  qDebug() << "removeObject" << obj->apiLink();
#endif

  // m_totalItems--;
  emit changed();
}

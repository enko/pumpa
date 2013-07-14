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

//------------------------------------------------------------------------------

QASAbstractObjectList::QASAbstractObjectList(int asType, QString url,
                                             QObject* parent) :
  QASAbstractObject(asType, parent),
  m_url(url),
  m_totalItems(0)
{}

//------------------------------------------------------------------------------

void QASAbstractObjectList::update(QVariantMap json, bool older) {
  bool ch = false;

  updateVar(json, m_displayName, "displayName", ch);
  updateVar(json, m_totalItems, "totalItems", ch);
  updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

  m_nextLink = "";
  updateVar(json, m_prevLink, "links", "prev", "href", ch);
  updateVar(json, m_nextLink, "links", "next", "href", ch);

  // We assume that collections come in as newest first, so we add
  // items starting from the top going downwards. Or if older=true
  // starting from the end and going downwards (appending).

  // Start adding from the top or bottom, depending on value of older.
  int mi = older ? m_items.size() : 0;

  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASAbstractObject* obj = getAbstractObject(items_json.at(i).toMap(),
                                               parent());
    if (m_item_set.contains(obj))
      continue;

    m_items.insert(mi++, obj);
    m_item_set.insert(obj);
    connectSignals(obj, false, true);

    ch = true;
  }

  if (ch)
    emit changed(older);
}

//------------------------------------------------------------------------------

QString QASAbstractObjectList::urlOrProxy() const {
  return m_proxyUrl.isEmpty() ? m_url : m_proxyUrl; 
}

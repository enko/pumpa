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

#include "qasactorlist.h"

#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QMap<QString, QASActorList*> QASActorList::s_actorLists;
void QASActorList::clearCache() { deleteMap<QASActorList*>(s_actorLists); }

//------------------------------------------------------------------------------

QASActorList::QASActorList(QString url, QObject* parent) :
  QASObjectList(url, parent)
{
  m_asType = QAS_OBJECTLIST;
#ifdef DEBUG_QAS
  qDebug() << "new ActorList" << m_url;
#endif
}

//------------------------------------------------------------------------------

QASActorList* QASActorList::getActorList(QVariantMap json, QObject* parent,
                                         int id) {
  QString url = json["url"].toString();
  if (url.isEmpty())
    return NULL;

  QASActorList* ol = s_actorLists.contains(url) ? s_actorLists[url] :
    new QASActorList(url, parent);
  s_actorLists.insert(url, ol);

  ol->update(json, id & QAS_OLDER);
  return ol;
}

//------------------------------------------------------------------------------

QASActor* QASActorList::at(size_t i) const {
  if (i >= size())
    return NULL;
  return QASObjectList::at(i)->asActor();
}

//------------------------------------------------------------------------------

QString QASActorList::actorNames() const {
  QString text;
  for (size_t i=0; i<size(); i++) {
    QASActor* a = at(i);
    text += QString("<a href=\"%1\">%2</a>")
      .arg(a->url())
      .arg(a->displayNameOrYou());
    if (i != size()-1)
      text += ", ";
  }
  return text;
}


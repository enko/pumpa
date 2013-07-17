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

#include "qasobject.h"

#include "qasactor.h"
#include "qasobjectlist.h"
#include "qasactorlist.h"
#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QMap<QString, QASObject*> QASObject::s_objects;

void QASObject::clearCache() { deleteMap<QASObject*>(s_objects); }

//------------------------------------------------------------------------------

QASObject::QASObject(QString id, QObject* parent) :
  QASAbstractObject(QAS_OBJECT, parent),
  m_id(id),
  m_liked(false),
  m_shared(false),
  m_inReplyTo(NULL),
  m_author(NULL),
  m_replies(NULL),
  m_likes(NULL),
  m_shares(NULL)
{
#ifdef DEBUG_QAS
  qDebug() << "new Object" << m_id;
#endif
}

//------------------------------------------------------------------------------

void QASObject::update(QVariantMap json, bool ignoreLike) {
#ifdef DEBUG_QAS
  qDebug() << "updating Object" << m_id;
#endif
  bool ch = false;
  bool wasDeleted = isDeleted();

  updateVar(json, m_objectType, "objectType", ch);
  updateVar(json, m_url, "url", ch);
  updateVar(json, m_content, "content", ch);
  if (!ignoreLike)
    updateVar(json, m_liked, "liked", ch);
  updateVar(json, m_displayName, "displayName", ch);
  updateVar(json, m_shared, "pump_io", "shared", ch);

  if (m_objectType == "image" && json.contains("image")) {
    updateUrlOrProxy(json["image"].toMap(), m_imageUrl, ch);

    if (json.contains("fullImage"))
      updateUrlOrProxy(json["fullImage"].toMap(), m_fullImageUrl, ch);
  }

  updateVar(json, m_published, "published", ch);
  updateVar(json, m_updated, "updated", ch);
  updateVar(json, m_deleted, "deleted", ch);

  updateVar(json, m_apiLink, "links", "self", "href", ch);  
  updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

  if (json.contains("inReplyTo")) {
    m_inReplyTo = QASObject::getObject(json["inReplyTo"].toMap(), parent());
    connectSignals(m_inReplyTo, true, true);
  }

  if (json.contains("author")) {
    m_author = QASActor::getActor(json["author"].toMap(), parent());
    connectSignals(m_author);
  }

  if (json.contains("replies")) {
    QVariantMap repliesMap = json["replies"].toMap();

    // don't replace a list with an empty one...
    if (repliesMap["items"].toList().size()) {
      m_replies = QASObjectList::getObjectList(repliesMap, parent());
      m_replies->isReplies(true);
      connectSignals(m_replies);
    }
  }

  if (json.contains("likes")) {
    m_likes = QASActorList::getActorList(json["likes"].toMap(), parent());
    connectSignals(m_likes);
  }

  if (json.contains("shares")) {
    m_shares = QASActorList::getActorList(json["shares"].toMap(), parent());
    connectSignals(m_shares);
  }

  if (isDeleted()) {
    m_content = "";
    m_displayName = "";
    if (!wasDeleted)
      ch = true;
  }

  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QASObject* QASObject::getObject(QVariantMap json, QObject* parent,
                                bool ignoreLike) {
  QString id = json["id"].toString();
  Q_ASSERT_X(!id.isEmpty(), "getObject", serializeJsonC(json));

  if (json["objectType"] == "person")
    return QASActor::getActor(json, parent);

  QASObject* obj = s_objects.contains(id) ?  s_objects[id] :
    new QASObject(id, parent);
  s_objects.insert(id, obj);

  obj->update(json, ignoreLike);
  return obj;
}

//------------------------------------------------------------------------------

void QASObject::addReply(QASObject* obj) {
  if (!m_replies) {
    m_replies = QASObjectList::initObjectList(id() + "/replies", parent());
    m_replies->isReplies(true);
    connectSignals(m_replies);
  }
  m_replies->addObject(obj);
#ifdef DEBUG_QAS
  qDebug() << "addReply" << obj->id() << "to" << id();
#endif
}

//------------------------------------------------------------------------------

void QASObject::toggleLiked() { 
  m_liked = !m_liked; 
  emit changed();
}

//------------------------------------------------------------------------------

size_t QASObject::numLikes() const {
  return m_likes ? m_likes->size() : 0;
}

//------------------------------------------------------------------------------

void QASObject::addLike(QASActor* actor, bool like) {
  if (!m_likes)
    return;
  if (like)
    m_likes->addActor(actor);
  else
    m_likes->removeActor(actor);
}

//------------------------------------------------------------------------------

void QASObject::addShare(QASActor* actor) {
  if (!m_shares)
    return;
  m_shares->addActor(actor);
}

//------------------------------------------------------------------------------

size_t QASObject::numShares() const {
  return m_shares ? m_shares->size() : 0;
}

//------------------------------------------------------------------------------

size_t QASObject::numReplies() const {
  return m_replies ? m_replies->size() : 0;
}

//------------------------------------------------------------------------------

QString QASObject::apiLink() const {
  return 
    !m_proxyUrl.isEmpty() ? m_proxyUrl :
    !m_apiLink.isEmpty() ? m_apiLink :
    m_id;
}

//------------------------------------------------------------------------------

QVariantMap QASObject::toJson() const {
  QVariantMap obj;

  obj["id"] = m_id;

  addVar(obj, m_content, "content");
  addVar(obj, m_objectType, "objectType");
  addVar(obj, m_url, "url");
  addVar(obj, m_displayName, "displayName");
  // addVar(obj, m_, "");

  return obj;
}

//------------------------------------------------------------------------------

QASActor* QASObject::asActor() {
  return qobject_cast<QASActor*>(this);
}


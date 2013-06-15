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

#include "qactivitystreams.h"
#include "pumpa_defines.h"
#include "json.h"

#include <QDebug>
#include <QStringList>
#include <QVariantList>

#define DEBUG 0

//------------------------------------------------------------------------------

QMap<QString, QASActor*> QASActor::s_actors;
QMap<QString, QASObject*> QASObject::s_objects;
QMap<QString, QASActivity*> QASActivity::s_activities;
QMap<QString, QASObjectList*> QASObjectList::s_objectLists;
QMap<QString, QASActorList*> QASActorList::s_actorLists;
QMap<QString, QASCollection*> QASCollection::s_collections;

//------------------------------------------------------------------------------

qint64 sortIntByDateTime(QDateTime dt) {
  return dt.toMSecsSinceEpoch();
}

//------------------------------------------------------------------------------

bool timeNewer(QDateTime thisT, QDateTime thatT) {
  return sortIntByDateTime(thisT) > sortIntByDateTime(thatT);
}

//------------------------------------------------------------------------------

QDateTime parseTime(QString timeStr) {
  // 2013-05-28T16:43:06Z 
  // 55 minutes ago kl. 20:39 -> 19:44

  QDateTime dt = QDateTime::fromString(timeStr, Qt::ISODate); 
                                       // "yyyy-MM-ddThh:mm:ssZ");
  dt.setTimeSpec(Qt::UTC);

  return dt;
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QString& var, QString name, bool& changed) {
  QString oldVar = var;
  if (obj.contains(name))
    var = obj[name].toString();
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, bool& var, QString name, bool& changed) {
  bool oldVar = var;
  if (obj.contains(name))
    var = obj[name].toBool();
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, qulonglong& var, QString name, bool& changed) {
  qulonglong oldVar = var;
  if (obj.contains(name))
    var = obj[name].toULongLong();
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QDateTime& var, QString name, bool& changed) {
  QDateTime oldVar = var;
  if (obj.contains(name))
    var = parseTime(obj[name].toString());
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QString& var, QString name1, QString name2,
               bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, changed);
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, bool& var, QString name1, QString name2,
               bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, changed);
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QString& var, QString name1,
               QString name2, QString name3, bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, name3, changed);
}

//------------------------------------------------------------------------------

void addVar(QVariantMap& obj, QString var, QString name) {
  if (var.isEmpty())
    return;
  obj[name] = var;
}

//------------------------------------------------------------------------------

void updateUrlOrProxy(QVariantMap obj, QString& var, bool& changed) {
  QString oldVar = var;
  bool dummy;
  updateVar(obj, var, "url", dummy);
  updateVar(obj, var, "pump_io", "proxyURL", dummy);
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

QASAbstractObject::QASAbstractObject(QObject* parent) : QObject(parent) {}

//------------------------------------------------------------------------------

void QASAbstractObject::connectSignals(QASAbstractObject* obj,
                                       bool changed, bool req) {
  if (!obj)
    return;

  if (changed)
    connect(obj, SIGNAL(changed()),
            this, SIGNAL(changed()), Qt::UniqueConnection);
  if (req)
    connect(obj, SIGNAL(request(QString, int)),
            parent(), SLOT(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

QASActor::QASActor(QString id, QObject* parent) :
  QASObject(id, parent),
  m_isYou(false)
{
#if DEBUG >= 1
  qDebug() << "new Actor" << m_id;
#endif
}

//------------------------------------------------------------------------------

void QASActor::update(QVariantMap json) {
#if DEBUG >= 1
  qDebug() << "updating Actor" << m_id;
#endif
  bool ch = false;

  updateVar(json, m_url, "url", ch); 
  updateVar(json, m_displayName, "displayName", ch);

  if (json.contains("image")) {
    QVariantMap im = json["image"].toMap();
    updateUrlOrProxy(im, m_imageUrl, ch);
  }

  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QASActor* QASActor::getActor(QVariantMap json, QObject* parent) {
  QString id = json["id"].toString();
  Q_ASSERT_X(!id.isEmpty(), "getActor", serializeJsonC(json));

  QASActor* act = s_actors.contains(id) ?  s_actors[id] :
    new QASActor(id, parent);
  s_actors.insert(id, act);

  act->update(json);
  return act;
}

//------------------------------------------------------------------------------

QASObject::QASObject(QString id, QObject* parent) :
  QASAbstractObject(parent),
  m_id(id),
  m_liked(false),
  m_shared(false),
  m_inReplyTo(NULL),
  m_author(NULL),
  m_replies(NULL),
  m_likes(NULL),
  m_shares(NULL)
{
#if DEBUG >= 1
  qDebug() << "new Object" << m_id;
#endif
}

//------------------------------------------------------------------------------

void QASObject::update(QVariantMap json) {
#if DEBUG >= 1
  qDebug() << "updating Object" << m_id;
#endif
  bool ch = false;

  updateVar(json, m_objectType, "objectType", ch);
  updateVar(json, m_url, "url", ch);
  updateVar(json, m_content, "content", ch);
  updateVar(json, m_liked, "liked", ch);
  updateVar(json, m_displayName, "displayName", ch);
  updateVar(json, m_shared, "pump_io", "shared", ch);

  if (m_objectType == "image") {
    QVariantMap imageObj = json["image"].toMap();
    updateUrlOrProxy(imageObj, m_imageUrl, ch);
  }

  updateVar(json, m_published, "published", ch);
  updateVar(json, m_updated, "updated", ch);

  updateVar(json, m_apiLink, "links", "self", "href", ch);  
  updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

  if (json.contains("inReplyTo")) {
    m_inReplyTo = QASObject::getObject(json["inReplyTo"].toMap(), parent());
    connectSignals(m_inReplyTo, true, false);
  }

  if (json.contains("author")) {
    m_author = QASActor::getActor(json["author"].toMap(), parent());
    connectSignals(m_author);
  }

  if (json.contains("replies")) {
    m_replies = QASObjectList::getObjectList(json["replies"].toMap(), parent());
    connectSignals(m_replies);
  }

  if (json.contains("likes")) {
    m_likes = QASActorList::getActorList(json["likes"].toMap(), parent());
    connectSignals(m_likes);
  }

  if (json.contains("shares")) {
    m_shares = QASActorList::getActorList(json["shares"].toMap(), parent());
    connectSignals(m_shares);
  }

  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QASObject* QASObject::getObject(QVariantMap json, QObject* parent) {
  QString id = json["id"].toString();
  Q_ASSERT_X(!id.isEmpty(), "getObject", serializeJsonC(json));

  QASObject* obj = s_objects.contains(id) ?  s_objects[id] :
    new QASObject(id, parent);
  s_objects.insert(id, obj);

  obj->update(json);
  return obj;
}

//------------------------------------------------------------------------------

void QASObject::refresh() {
  static QDateTime last_refreshed;
  QDateTime now = QDateTime::currentDateTime();
  
  if (last_refreshed.isNull() || last_refreshed.secsTo(now) > 1)
    emit request(apiLink(), QAS_OBJECT);

  last_refreshed = now;
}

//------------------------------------------------------------------------------

void QASObject::addReply(QASObject* obj) {
  if (!m_replies)
    return;
  m_replies->addObject(obj);
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

QASActivity::QASActivity(QString id, QObject* parent) : 
  QASAbstractObject(parent),
  m_id(id),
  m_object(NULL),
  m_actor(NULL)
{
#if DEBUG >= 1
  qDebug() << "new Activity" << m_id;
#endif
}

//------------------------------------------------------------------------------

void QASActivity::update(QVariantMap json) {
#if DEBUG >= 1
  qDebug() << "updating Activity" << m_id;
#endif
#if DEBUG >= 3
  qDebug() << debugDumpJson(json, "QASActivity");
#endif
  bool ch = false;

  updateVar(json, m_verb, "verb", ch);
  updateVar(json, m_url, "url", ch);
  updateVar(json, m_content, "content", ch);
  
  if (json.contains("actor")) {
    m_actor = QASActor::getActor(json["actor"].toMap(), parent());
    connectSignals(m_actor);
  }

  if (json.contains("object")) {
    m_object = QASObject::getObject(json["object"].toMap(), parent());
    connectSignals(m_object);
    if (!m_object->author())
      m_object->setAuthor(m_actor);
  }

  updateVar(json, m_published, "published", ch);
  updateVar(json, m_updated, "updated", ch);
  updateVar(json, m_generatorName, "generator", "displayName", ch);

  if (m_verb == "post" && m_object && m_object->inReplyTo())
    m_object->inReplyTo()->addReply(m_object);

  if ((m_verb == "favorite" || m_verb == "like" ||
       m_verb == "unfavorite" || m_verb == "unlike") && m_object && m_actor) 
    m_object->addLike(m_actor, !m_verb.startsWith("un"));

  if (m_verb == "share" && m_object && m_actor) 
    m_object->addShare(m_actor);

  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QASActivity* QASActivity::getActivity(QVariantMap json, QObject* parent) {
  QString id = json["id"].toString();
  Q_ASSERT_X(!id.isEmpty(), "getActivity", serializeJsonC(json));

  QASActivity* act = s_activities.contains(id) ? s_activities[id] :
    new QASActivity(id, parent);
  s_activities.insert(id, act);

  act->update(json);
  return act;
}

//------------------------------------------------------------------------------

QASObjectList::QASObjectList(QString url, QObject* parent) :
  QASAbstractObject(parent),
  m_url(url),
  m_totalItems(0),
  m_hasMore(false)
{
#if DEBUG >= 1
  qDebug() << "new ObjectList" << m_url;
#endif
}

//------------------------------------------------------------------------------

void QASObjectList::update(QVariantMap json, bool) {
#if DEBUG >= 1
  qDebug() << "updating ObjectList" << m_url;
#endif
  bool ch = false;

  updateVar(json, m_totalItems, "totalItems", ch);
  updateVar(json, m_proxyUrl, "pump_io", "proxyURL", ch);

  if (json.contains("items")) {
    m_items.clear();
    QVariantList items_json = json["items"].toList();
    for (int i=0; i<items_json.count(); i++) {
      QVariantMap item = items_json[i].toMap();
      QASObject* obj;
      if (item["objectType"].toString() == "person")
        obj = QASActor::getActor(item, parent());
      else
        obj = QASObject::getObject(item, parent());
      
      m_items.append(obj);
      connectSignals(obj, false, true);
      ch = true;
    }
  }
  // set to false if number of items < total, and if we have already
  // fetched it - that seems to have a displayName element
  // ^^ FFFUUUGLY HACK !!!
  // bool old_hasMore = m_hasMore;
  m_hasMore = !json.contains("displayName") && size() < m_totalItems;
  // if (!old_hasMore && m_hasMore)
  //   qDebug() << "[DEBUG]: set hasMore" << m_url << m_proxyUrl << urlOrProxy();

  if (ch)
    emit changed();
}

//------------------------------------------------------------------------------

QASObjectList* QASObjectList::getObjectList(QVariantMap json, QObject* parent,
                                            int id) {
  QString url = json["url"].toString();
  if (url.isEmpty())
    return NULL;

  QASObjectList* ol = s_objectLists.contains(url) ? s_objectLists[url] :
    new QASObjectList(url, parent);
  s_objectLists.insert(url, ol);

  ol->update(json, id & QAS_OLDER);
  return ol;
}

//------------------------------------------------------------------------------

void QASObjectList::addObject(QASObject* obj) {
  m_items.append(obj);
  m_totalItems++;
  emit changed();
}

//------------------------------------------------------------------------------

QString QASObjectList::urlOrProxy() const {
  return m_proxyUrl.isEmpty() ? m_url : m_proxyUrl; 
}

//------------------------------------------------------------------------------

//FIXME, refresh() could be in QASAbstractObject
void QASObjectList::refresh() {
  static QDateTime last_refreshed;
  QDateTime now = QDateTime::currentDateTime();
  
  if (last_refreshed.isNull() || last_refreshed.secsTo(now) > 1)
    emit request(urlOrProxy(), QAS_OBJECTLIST);

  last_refreshed = now;
}

//------------------------------------------------------------------------------

QASActorList::QASActorList(QString url, QObject* parent) :
  QASObjectList(url, parent)
{
#if DEBUG >= 1
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
  return qobject_cast<QASActor*>(m_items[i]);
}

//------------------------------------------------------------------------------

void QASActorList::addActor(QASActor* actor) {
  if (m_items.contains(actor))
    return;

  m_items.append(actor);
  m_totalItems++;
  emit changed();
}

//------------------------------------------------------------------------------

void QASActorList::removeActor(QASActor* actor) {
  m_items.removeAll(actor);
  m_totalItems--;
  emit changed();
}

//------------------------------------------------------------------------------

//FIXME, refresh() could be in QASAbstractObject
void QASActorList::refresh() {
  static QDateTime last_refreshed;
  QDateTime now = QDateTime::currentDateTime();
  
  if (last_refreshed.isNull() || last_refreshed.secsTo(now) > 1)
    emit request(urlOrProxy(), QAS_ACTORLIST);

  last_refreshed = now;
}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QString url, QObject* parent) :
  QASAbstractObject(parent),
  m_url(url),
  m_totalItems(0)
{
#if DEBUG >= 1
  qDebug() << "new Collection" << m_url;
#endif
}

//------------------------------------------------------------------------------

void QASCollection::update(QVariantMap json, bool older) {
  bool ch = false;

  updateVar(json, m_displayName, "displayName", ch);
  updateVar(json, m_totalItems, "totalItems", ch);

  updateVar(json, m_prevLink, "links", "prev", "href", ch);
  updateVar(json, m_nextLink, "links", "next", "href", ch);

  // We assume that collections come in as newest first, so we add
  // items starting from the top going downwards. Or if older=true
  // starting from the end and going downwards (appending).

  // Start adding from the top or bottom, depending on value of older.
  int mi = older ? m_items.size() : 0;


  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASActivity* act = QASActivity::getActivity(items_json.at(i).toMap(),
                                                parent());
    if (m_item_set.contains(act))
      continue;

    m_items.insert(mi++, act);
    m_item_set.insert(act);
    connectSignals(act);

    ch = true;
  }

  if (ch)
    emit changed(older);
}

//------------------------------------------------------------------------------

QASCollection* QASCollection::getCollection(QVariantMap json, QObject* parent,
                                            int id) {
  QString url = json["url"].toString();
  if (url.isEmpty())
    return NULL;

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

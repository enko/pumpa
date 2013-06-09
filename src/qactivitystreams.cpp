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

void updateVar(QVariantMap obj, QString& var, QString name) {
  if (obj.contains(name))
    var = obj[name].toString();
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, bool& var, QString name) {
  if (obj.contains(name))
    var = obj[name].toBool();
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, qulonglong& var, QString name) {
  if (obj.contains(name))
    var = obj[name].toULongLong();
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QDateTime& var, QString name) {
  if (obj.contains(name))
    var = parseTime(obj[name].toString());
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QString& var, QString name1, QString name2) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2);
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, bool& var, QString name1, QString name2) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2);
}

//------------------------------------------------------------------------------

void updateVar(QVariantMap obj, QString& var, QString name1,
               QString name2, QString name3) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, name3);
}

//------------------------------------------------------------------------------

void addVar(QVariantMap& obj, QString var, QString name) {
  if (var.isEmpty())
    return;
  obj[name] = var;
}

//------------------------------------------------------------------------------

QString getProxyUrl(QVariantMap obj) {
  if (obj.contains("pump_io")) 
    return obj["pump_io"].toMap()["proxyURL"].toString();
  return "";
}

//------------------------------------------------------------------------------

void updateUrlOrProxy(QVariantMap obj, QString& var) {
  QString proxyUrl = getProxyUrl(obj);
  if (!proxyUrl.isEmpty())
    var = proxyUrl;
  else
    updateVar(obj, var, "url");
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
  
  updateVar(json, m_url, "url"); 
  updateVar(json, m_displayName, "displayName");

  if (json.contains("image")) {
    QVariantMap im = json["image"].toMap();
    updateUrlOrProxy(im, m_imageUrl);
  }

  Q_ASSERT_X(!m_id.isEmpty(), "QASActor", serializeJsonC(json));
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
  QObject(parent),
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

  bool old_liked = m_liked;
  QDateTime old_updated = m_updated;
  size_t num_replies = numReplies();
  size_t num_likes = numLikes();
  size_t num_shares = numShares();

  m_objectType = json["objectType"].toString();

  updateVar(json, m_url, "url");
  updateVar(json, m_content, "content");
  updateVar(json, m_liked, "liked");
  updateVar(json, m_displayName, "displayName");
  updateVar(json, m_shared, "pump_io", "shared");

  if (m_objectType == "image") {
    QVariantMap imageObj = json["image"].toMap();
    updateUrlOrProxy(imageObj, m_imageUrl);
  }

  updateVar(json, m_published, "published");
  updateVar(json, m_updated, "updated");

  updateVar(json, m_apiLink, "links", "self", "href");  updateVar(json, m_proxyUrl, "pump_io", "proxyURL");

  Q_ASSERT_X(!m_id.isEmpty(), "QASObject", serializeJsonC(json));

  // this is guard against almost empty "shell" objects from emptying
  // the real objects
  if (!json.contains("url"))
    return;

  m_replies = QASObjectList::getObjectList(json["replies"].toMap(), parent());

  m_likes = QASActorList::getActorList(json["likes"].toMap(), parent());
  m_shares = QASActorList::getActorList(json["shares"].toMap(), parent());

  if (json.contains("inReplyTo"))
    m_inReplyTo = QASObject::getObject(json["inReplyTo"].toMap(), parent());

  if (json.contains("author"))
    m_author = QASActor::getActor(json["author"].toMap(), parent());

  if (old_liked != m_liked || old_updated != m_updated ||
      num_replies != numReplies() || num_likes != numLikes() ||
      num_shares != numShares())
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

// void QASObject::setLike(bool like) {
//   m_liked = like;
//   emit changed();
// }

//------------------------------------------------------------------------------

size_t QASObject::numLikes() const {
  // return m_likes ? m_likes->size() : 0;
  return m_likes ? m_likes->totalItems() : 0;
}

//------------------------------------------------------------------------------

size_t QASObject::numShares() const {
  return m_shares ? m_shares->totalItems() : 0;
}

//------------------------------------------------------------------------------

size_t QASObject::numReplies() const {
  return m_replies ? m_replies->size() : 0;
}

//------------------------------------------------------------------------------

QString QASObject::apiLink() const {
  return m_proxyUrl.isEmpty() ? m_apiLink : m_proxyUrl;
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
  QObject(parent),
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

  updateVar(json, m_verb, "verb");
  updateVar(json, m_url, "url");
  updateVar(json, m_content, "content");
  
  Q_ASSERT_X(!m_id.isEmpty(), "QASActivity::update", serializeJsonC(json));

  if (json.contains("object"))
    m_object = QASObject::getObject(json["object"].toMap(), parent());
  if (json.contains("actor"))
    m_actor = QASActor::getActor(json["actor"].toMap(), parent());

  updateVar(json, m_published, "published");
  updateVar(json, m_updated, "updated");

  if (json.contains("generator"))
    m_generatorName = json["generator"].toMap()["displayName"].toString();

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
  QObject(parent),
  m_url(url),
  m_totalItems(0),
  m_hasMore(false)
{
#if DEBUG >= 1
  qDebug() << "new ObjectList" << m_url;
#endif
}

//------------------------------------------------------------------------------

void QASObjectList::update(QVariantMap json) {
#if DEBUG >= 1
  qDebug() << "updating ObjectList" << m_url;
#endif
#if DEBUG >= 3
  qDebug() << debugDumpJson(json, "QASObjectList");
#endif

  qulonglong old_totalItems = m_totalItems;
  int old_item_count = m_items.size();

  updateVar(json, m_url, "url");
  updateVar(json, m_totalItems, "totalItems");
  m_proxyUrl = getProxyUrl(json);

  m_items.clear();
  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++)
    addObject(items_json.at(i).toMap());

  // set to false if number of items < total, and if we have already
  // fetched it - that seems to have a displayName element
  // ^^ FFFUUUGLY HACK !!!
  m_hasMore = !json.contains("displayName") && size() < m_totalItems;

  if (old_totalItems != m_totalItems || old_item_count != m_items.size())
    emit changed();
}

//------------------------------------------------------------------------------

void QASObjectList::addObject(QVariantMap json) {
  QASObject* act;
  if (json["objectType"].toString() == "person")
    act = QASActor::getActor(json, parent());
  else
    act = QASObject::getObject(json, parent());
  m_items.append(act);
}

//------------------------------------------------------------------------------

QASObjectList* QASObjectList::getObjectList(QVariantMap json, QObject* parent) {
  QString url = json["url"].toString();
  if (url.isEmpty()) {
    // qDebug() << "Curious object list" << debugDumpJson(json);
    return NULL;
  }
  // Q_ASSERT_X(!url.isEmpty(), "getObjectList", serializeJsonC(json));

  QASObjectList* ol = s_objectLists.contains(url) ? s_objectLists[url] :
    new QASObjectList(url, parent);
  s_objectLists.insert(url, ol);

  ol->update(json);
  return ol;
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

QASActorList* QASActorList::getActorList(QVariantMap json, QObject* parent) {
  QString url = json["url"].toString();
  if (url.isEmpty())
    return NULL;

  QASActorList* ol = s_actorLists.contains(url) ? s_actorLists[url] :
    new QASActorList(url, parent);
  s_actorLists.insert(url, ol);

  ol->update(json);
  return ol;
}

//------------------------------------------------------------------------------

QASActor* QASActorList::at(size_t i) const {
  if (i >= size())
    return NULL;
  return qobject_cast<QASActor*>(m_items[i]);
}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QObject* parent) : QObject(parent),
                                                m_totalItems(0) {}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QVariantMap json, QObject* parent) :
  QObject(parent) 
{
  updateVar(json, m_displayName, "displayName");
  updateVar(json, m_url, "url");
  updateVar(json, m_totalItems, "totalItems");

  m_nextUrl = json["links"].toMap()["next"].toMap()["href"].toString();

  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASActivity* act = QASActivity::getActivity(items_json.at(i).toMap(),
                                                parent);
    m_items.append(act);
  }
}


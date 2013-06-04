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

QString getProxyUrl(QVariantMap obj) {
  if (obj.contains("pump_io")) 
    return obj["pump_io"].toMap()["proxyURL"].toString();
  return "";
}

//------------------------------------------------------------------------------

QString getUrlOrProxy(QVariantMap obj) {
  QString proxyUrl = getProxyUrl(obj);
  return proxyUrl.isEmpty() ? obj["url"].toString() : proxyUrl;
}

//------------------------------------------------------------------------------

QASActor::QASActor(QString id, QObject* parent) :
  QObject(parent),
  m_id(id) 
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

  m_preferredUsername = json["preferredUsername"].toString();
  m_url = json["url"].toString();
  m_displayName = json["displayName"].toString();

  if (json.contains("image")) {
    QVariantMap im = json["image"].toMap();
    m_imageUrl = getUrlOrProxy(im);
  }

  Q_ASSERT_X(!m_id.isEmpty(), "QASActor", serializeJsonC(json));

        //   "links": {
      //     "self": {
      //       "href": "https://microca.st/api/user/encycl/profile"
      //     },
      //     "activity-inbox": {
      //       "href": "https://microca.st/api/user/encycl/inbox"
      //     },
      //     "activity-outbox": {
      //       "href": "https://microca.st/api/user/encycl/feed"
      //     }
      //   },
      //   "objectType": "person",
      //   "followers": {
      //     "url": "https://microca.st/api/user/encycl/followers",
      //     "author": {
      //       "id": "acct:encycl@microca.st",
      //       "objectType": "person"
      //     },
      //     "links": {
      //       "self": {
      //         "href": "https://microca.st/api/user/encycl/followers"
      //       }
      //     },
      //     "displayName": "Followers",
      //     "members": {
      //       "url": "https://microca.st/api/user/encycl/followers"
      //     },
      //     "objectType": "collection",
      //     "id": "https://microca.st/api/user/encycl/followers",
      //     "pump_io": {
      //       "proxyURL": "https://io.saz.im/api/proxy/35H_pp91SW657XmH3m3tPw"
      //     }
      //   },
      //   "following": {
      //     "url": "https://microca.st/api/user/encycl/following",
      //     "pump_io": {
      //       "proxyURL": "https://io.saz.im/api/proxy/809G1WAHSzGe68K5Ls0T8g"
      //     }
      //   },
      //   "favorites": {
      //     "url": "https://microca.st/api/user/encycl/favorites"
      //   },
      //   "lists": {
      //     "url": "https://microca.st/api/user/encycl/lists/person",
      //     "displayName": "Collections of persons for encycl",
      //     "objectTypes": [
      //       "collection"
      //     ],
      //     "links": {
      //       "first": {
      //         "href": "https://microca.st/api/user/encycl/lists/person"
      //       },
      //       "self": {
      //         "href": "https://microca.st/api/user/encycl/lists/person"
      //       },
      //       "prev": {
      //         "href": "https://microca.st/api/user/encycl/lists/person?since=https%3A%2F%2Fmicroca.st%2Fapi%2Fcollection%2FujrvzbkZTaWfqNkwMxy0hw"
      //       }
      //     },
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
  m_author(NULL),
  m_replies(NULL)
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
  bool num_replies = m_replies ? m_replies->size() : 0;

  m_content = json["content"].toString();
  m_objectType = json["objectType"].toString();
  m_url = json["url"].toString();
  m_liked = json["liked"].toBool();

  if (m_objectType == "image") {
    QVariantMap imageObj = json["image"].toMap();
    m_imageUrl = getUrlOrProxy(imageObj);
  }

  m_published = parseTime(json["published"].toString());
  m_updated = parseTime(json["updated"].toString());

  Q_ASSERT_X(!m_id.isEmpty(), "QASObject", serializeJsonC(json));

  m_replies = QASObjectList::getObjectList(json["replies"].toMap(), parent());

  if (json.contains("inReplyTo"))
    m_inReplyToId = json["inReplyTo"].toMap()["id"].toString();

  if (json.contains("author"))
    m_author = QASActor::getActor(json["author"].toMap(), parent());

  if (old_liked != m_liked || old_updated != m_updated ||
      num_replies != numReplies())
    emit changed();

      //   "links": {
      //     "self": {
      //       "href": "http://frodo:8000/api/note/jMgmxKHfSuaLM1eqsvFKaw"
      //     }
      //   },
      //   "likes": {
      //     "url": "http://frodo:8000/api/note/jMgmxKHfSuaLM1eqsvFKaw/likes",
      //     "totalItems": 0
      //   },
      //   "replies": {
      //     "url": "http://frodo:8000/api/note/jMgmxKHfSuaLM1eqsvFKaw/replies",
      //     "totalItems": 0
      //   },
      //   "shares": {
      //     "url": "http://frodo:8000/api/note/jMgmxKHfSuaLM1eqsvFKaw/shares",
      //     "totalItems": 0
      //   },
      //   "pump_io": {
      //     "shared": false
      //   }
      // },
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

size_t QASObject::numReplies() const {
  return m_replies ? m_replies->size() : 0;
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

  m_verb = json["verb"].toString();
  m_url = json["url"].toString();
  m_content = json["content"].toString();
  
  Q_ASSERT_X(!m_id.isEmpty(), "QASActivity::update", serializeJsonC(json));

  m_object = QASObject::getObject(json["object"].toMap(), parent());
  m_actor = QASActor::getActor(json["actor"].toMap(), parent());

  m_published = parseTime(json["published"].toString());
  m_updated = parseTime(json["updated"].toString());

  // Stuff not handled yet:

  // "generator": {
  //   "objectType": "application",
  //   "id": "http://frodo:8000/api/application/oGi8xnNvS1GXZHjiCn9CFQ",
  //   "updated": "2013-05-25T21:06:06Z",
  //   "published": "2013-05-25T21:06:06Z",
  //   "links": { "self": {"href": "http://frodo:8000/api/application/oGi8xnNvS1GXZHjiCn9CFQ"} },
  //   "likes": { "url": "http://frodo:8000/api/application/oGi8xnNvS1GXZHjiCn9CFQ/likes"},
  //   "replies": {"url": "http://frodo:8000/api/application/oGi8xnNvS1GXZHjiCn9CFQ/replies" },
  //   "shares": { "url": "http://frodo:8000/api/application/oGi8xnNvS1GXZHjiCn9CFQ/shares" }
  // },
  // "cc": [
  //   {
  //     "author": {
  //       "preferredUsername": "sazius",
  //       "url": "http://frodo:8000/sazius",
  //       "displayName": "sazius",
  //       "id": "http://frodo:8000/api/user/sazius/profile",
  //       "links": {
  //         "self": {
  //           "href": "http://frodo:8000/api/user/sazius/profile"
  //         },
  //         "activity-inbox": {
  //           "href": "http://frodo:8000/api/user/sazius/inbox"
  //         },
  //         "activity-outbox": {
  //           "href": "http://frodo:8000/api/user/sazius/feed"
  //         }
  //       },
  //       "objectType": "person",
  //       "followers": {
  //         "url": "http://frodo:8000/api/user/sazius/followers"
  //       },
  //       "following": {
  //         "url": "http://frodo:8000/api/user/sazius/following"
  //       },
  //       "favorites": {
  //         "url": "http://frodo:8000/api/user/sazius/favorites"
  //       },
  //       "lists": {
  //         "url": "http://frodo:8000/api/user/sazius/lists/person"
  //       }
  //     },
  //     "id": "http://frodo:8000/api/user/sazius/followers",
  //     "links": {
  //       "self": {
  //         "href": "http://frodo:8000/api/user/sazius/followers"
  //       }
  //     },
  //     "url": "http://frodo:8000/sazius/followers",
  //     "displayName": "Followers",
  //     "members": {
  //       "url": "http://frodo:8000/api/user/sazius/followers"
  //     },
  //     "objectType": "collection"
  //   }
  // ],
  // "links": {
  //   "self": {
  //     "href": "http://frodo:8000/api/activity/H1rhziiJRiSkihKckkHJ3A"
  //   }
  // },
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

  m_url = json["url"].toString();
  m_proxyUrl = getProxyUrl(json);
  m_totalItems = json["totalItems"].toULongLong();

  m_items.clear();
  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASObject* act = QASObject::getObject(items_json.at(i).toMap(),
                                          parent());
    m_items.append(act);
  }

  // set to false if number of items < total, and if we have already
  // fetched it - that seems to have a displayName element
  // ^^ FFFUUUGLY HACK !!!
  m_hasMore = !json.contains("displayName") && size() < m_totalItems;

  if (old_totalItems != m_totalItems || old_item_count != m_items.size())
    emit changed();
}

//------------------------------------------------------------------------------

QASObjectList* QASObjectList::getObjectList(QVariantMap json, QObject* parent) {
  QString url = json["url"].toString();
  if (url.isEmpty()) {
    qDebug() << "Curious object list" << debugDumpJson(json);
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

QASCollection::QASCollection(QObject* parent) : QObject(parent),
                                                m_totalItems(0) {}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QVariantMap json, QObject* parent) :
  QObject(parent) 
{
  m_displayName = json["displayName"].toString();
  m_url = json["url"].toString();
  m_totalItems = (int)json["totalItems"].toDouble();

  QVariantList items_json = json["items"].toList();
  for (int i=0; i<items_json.count(); i++) {
    QASActivity* act = QASActivity::getActivity(items_json.at(i).toMap(),
                                                parent);
    m_items.append(act);
  }

  // Stuff not handled yet:
  //   "objectTypes": [ "activity"  ],
  //   "links": { "first": {"href": ""}, "self": {"href": "http://"}, "prev": {"href": "" }  },
  //   "author": {
  //     "preferredUsername": "sazius",
  //     "url": "http://frodo:8000/sazius",
  //     "displayName": "sazius",
  //     "id": "http://frodo:8000/api/user/sazius/profile",
  //     "links": {
  //       "self": { "href": "http://frodo:8000/api/user/sazius/profile"},
  //       "activity-inbox": {"href": "http://frodo:8000/api/user/sazius/inbox"},
  //       "activity-outbox": {"href": "http://frodo:8000/api/user/sazius/feed"}
  //     },
  //     "objectType": "person",
  //     "followers": {"url": "http://frodo:8000/api/user/sazius/followers"},
  //     "following": { "url": "http://frodo:8000/api/user/sazius/following" },
  //     "favorites": { "url": "http://frodo:8000/api/user/sazius/favorites" },
  //     "lists": { "url": "http://frodo:8000/api/user/sazius/lists/person" }
  //   },
}


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
#include <QJsonArray>

//------------------------------------------------------------------------------

QASObject::QASObject(QObject* parent) : QObject(parent), liked(false) {}

QASObject::QASObject(QJsonObject json, QObject* parent) : QObject(parent) {
  QString content = json["content"].toString();
  QString objectType = json["objectType"].toString();
  QString id = json["id"].toString();
  QString url = json["url"].toString();
  liked = json["liked"].toBool();
  
  qDebug() << "QASObject" << content;
      //   "updated": "2013-05-25T21:06:07Z",
      //   "published": "2013-05-25T21:06:07Z",
      //   "links": {
      //     "self": {
      //       "href": "http:\/\/frodo:8000\/api\/note\/jMgmxKHfSuaLM1eqsvFKaw"
      //     }
      //   },
      //   "likes": {
      //     "url": "http:\/\/frodo:8000\/api\/note\/jMgmxKHfSuaLM1eqsvFKaw\/likes",
      //     "totalItems": 0
      //   },
      //   "replies": {
      //     "url": "http:\/\/frodo:8000\/api\/note\/jMgmxKHfSuaLM1eqsvFKaw\/replies",
      //     "totalItems": 0
      //   },
      //   "shares": {
      //     "url": "http:\/\/frodo:8000\/api\/note\/jMgmxKHfSuaLM1eqsvFKaw\/shares",
      //     "totalItems": 0
      //   },
      //   "pump_io": {
      //     "shared": false
      //   }
      // },
}

//------------------------------------------------------------------------------

QASActivity::QASActivity(QObject* parent) : QObject(parent),
                                            object(NULL) {}

//------------------------------------------------------------------------------

QASActivity::QASActivity(QJsonObject json, QObject* parent) : QObject(parent) {
  QString id = json["id"].toString();
  QString verb = json["verb"].toString();
  QString url = json["url"].toString();
  QString content = json["content"].toString();
  
  object = new QASObject(json["object"].toObject());

  // Stuff not handled yet:

  // "published": "2013-05-25T21:06:07Z",
  // "updated": "2013-05-25T21:06:07Z",

  // "actor": {
  //   "preferredUsername": "sazius",
  //   "url": "http:\/\/frodo:8000\/sazius",
  //   "displayName": "sazius",
  //   "id": "http:\/\/frodo:8000\/api\/user\/sazius\/profile",
  //   "links": {
  //     "self": {"href": "http:\/\/frodo:8000\/api\/user\/sazius\/profile"},
  //     "activity-inbox": {"href": "http:\/\/frodo:8000\/api\/user\/sazius\/inbox"},
  //     "activity-outbox": {"href": "http:\/\/frodo:8000\/api\/user\/sazius\/feed"}
  //   },
  //   "objectType": "person",
  //   "followers": {"url": "http:\/\/frodo:8000\/api\/user\/sazius\/followers"},
  //   "following": {"url": "http:\/\/frodo:8000\/api\/user\/sazius\/following"},
  //   "favorites": {"url": "http:\/\/frodo:8000\/api\/user\/sazius\/favorites"},
  //   "lists": {"url": "http:\/\/frodo:8000\/api\/user\/sazius\/lists\/person"}
  // },
  // "generator": {
  //   "objectType": "application",
  //   "id": "http:\/\/frodo:8000\/api\/application\/oGi8xnNvS1GXZHjiCn9CFQ",
  //   "updated": "2013-05-25T21:06:06Z",
  //   "published": "2013-05-25T21:06:06Z",
  //   "links": { "self": {"href": "http:\/\/frodo:8000\/api\/application\/oGi8xnNvS1GXZHjiCn9CFQ"} },
  //   "likes": { "url": "http:\/\/frodo:8000\/api\/application\/oGi8xnNvS1GXZHjiCn9CFQ\/likes"},
  //   "replies": {"url": "http:\/\/frodo:8000\/api\/application\/oGi8xnNvS1GXZHjiCn9CFQ\/replies" },
  //   "shares": { "url": "http:\/\/frodo:8000\/api\/application\/oGi8xnNvS1GXZHjiCn9CFQ\/shares" }
  // },
  // "cc": [
  //   {
  //     "author": {
  //       "preferredUsername": "sazius",
  //       "url": "http:\/\/frodo:8000\/sazius",
  //       "displayName": "sazius",
  //       "id": "http:\/\/frodo:8000\/api\/user\/sazius\/profile",
  //       "links": {
  //         "self": {
  //           "href": "http:\/\/frodo:8000\/api\/user\/sazius\/profile"
  //         },
  //         "activity-inbox": {
  //           "href": "http:\/\/frodo:8000\/api\/user\/sazius\/inbox"
  //         },
  //         "activity-outbox": {
  //           "href": "http:\/\/frodo:8000\/api\/user\/sazius\/feed"
  //         }
  //       },
  //       "objectType": "person",
  //       "followers": {
  //         "url": "http:\/\/frodo:8000\/api\/user\/sazius\/followers"
  //       },
  //       "following": {
  //         "url": "http:\/\/frodo:8000\/api\/user\/sazius\/following"
  //       },
  //       "favorites": {
  //         "url": "http:\/\/frodo:8000\/api\/user\/sazius\/favorites"
  //       },
  //       "lists": {
  //         "url": "http:\/\/frodo:8000\/api\/user\/sazius\/lists\/person"
  //       }
  //     },
  //     "id": "http:\/\/frodo:8000\/api\/user\/sazius\/followers",
  //     "links": {
  //       "self": {
  //         "href": "http:\/\/frodo:8000\/api\/user\/sazius\/followers"
  //       }
  //     },
  //     "url": "http:\/\/frodo:8000\/sazius\/followers",
  //     "displayName": "Followers",
  //     "members": {
  //       "url": "http:\/\/frodo:8000\/api\/user\/sazius\/followers"
  //     },
  //     "objectType": "collection"
  //   }
  // ],
  // "links": {
  //   "self": {
  //     "href": "http:\/\/frodo:8000\/api\/activity\/H1rhziiJRiSkihKckkHJ3A"
  //   }
  // },
}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QObject* parent) : QObject(parent),
                                                totalItems(0) {}

//------------------------------------------------------------------------------

QASCollection::QASCollection(QJsonObject json, QObject* parent) :
  QObject(parent) 
{
  displayName = json["displayName"].toString();
  url = json["url"].toString();
  totalItems = (int)json["totalItems"].toDouble();

  QJsonArray items_json = json["items"].toArray();
  for (int i=0; i<items_json.count(); i++) {
    QASActivity* act = new QASActivity(items_json.at(i).toObject());
    items.append(act);
  }

  // Stuff not handled yet:
  //   "objectTypes": [ "activity"  ],
  //   "links": { "first": {"href": ""}, "self": {"href": "http:\/\/"}, "prev": {"href": "" }  },
  //   "author": {
  //     "preferredUsername": "sazius",
  //     "url": "http:\/\/frodo:8000\/sazius",
  //     "displayName": "sazius",
  //     "id": "http:\/\/frodo:8000\/api\/user\/sazius\/profile",
  //     "links": {
  //       "self": { "href": "http:\/\/frodo:8000\/api\/user\/sazius\/profile"},
  //       "activity-inbox": {"href": "http:\/\/frodo:8000\/api\/user\/sazius\/inbox"},
  //       "activity-outbox": {"href": "http:\/\/frodo:8000\/api\/user\/sazius\/feed"}
  //     },
  //     "objectType": "person",
  //     "followers": {"url": "http:\/\/frodo:8000\/api\/user\/sazius\/followers"},
  //     "following": { "url": "http:\/\/frodo:8000\/api\/user\/sazius\/following" },
  //     "favorites": { "url": "http:\/\/frodo:8000\/api\/user\/sazius\/favorites" },
  //     "lists": { "url": "http:\/\/frodo:8000\/api\/user\/sazius\/lists\/person" }
  //   },
}


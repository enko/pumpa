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

#ifndef _QACTIVITYSTREAMS_H_
#define _QACTIVITYSTREAMS_H_

#include <QObject>
#include <QDateTime>

#include "json.h"

//------------------------------------------------------------------------------
// Forward declarations

class QASCollection;
class QASObjectList;

//------------------------------------------------------------------------------

#define QAS_RESPONSE_NULL      0
#define QAS_INBOX_MAJOR        1
#define QAS_INBOX_MINOR        2
#define QAS_INBOX_DIRECT_MAJOR 3
#define QAS_INBOX_DIRECT_MINOR 4
#define QAS_NEW_POST           5
#define QAS_REPLIES            6


//------------------------------------------------------------------------------

qint64 sortIntByDateTime(QDateTime dt);

//------------------------------------------------------------------------------

class QASActor : public QObject {
  Q_OBJECT

private:
  QASActor(QString id, QObject* parent);

public:
  static QASActor* getActor(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  QString url() const { return m_url; }
  QString displayName() const { return m_displayName; }
  QString imageUrl() const { return m_imageUrl; }

private:
  QString m_id;
  QString m_preferredUsername;
  QString m_url;
  QString m_displayName;
  QString m_imageUrl;

  static QMap<QString, QASActor*> s_actors;
};

//------------------------------------------------------------------------------

class QASObject : public QObject {
  Q_OBJECT

private:
  QASObject(QString id, QObject* parent);

public:
  static QASObject* getObject(QVariantMap json, QObject* parent);
  static QASObject* getObject(QString id) { 
    return s_objects.contains(id) ? s_objects[id] : NULL;
  }
  void update(QVariantMap json);

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }
  
  QString id() const { return m_id; }
  QString content() const { return m_content; }
  QString type() const { return m_objectType; }
  QString url() const { return m_url; }
  QString imageUrl() const { return m_imageUrl; }
  QString displayName() const { return m_displayName; }

  QDateTime published() const { return m_published; }
  bool liked() const { return m_liked; }
  size_t numReplies() const;
  QASObjectList* replies() const { return m_replies; }

  QASActor* author() const { return m_author; }
  QASObject* inReplyTo() const { return m_inReplyTo; }

signals:
  void changed();

private:
  QString m_id;
  QString m_content;
  bool m_liked;
  QString m_objectType;
  QString m_url;
  QString m_imageUrl;
  QString m_displayName;

  QDateTime m_published;
  QDateTime m_updated;

  QASObject* m_inReplyTo;
  QASActor* m_author;
  QASObjectList* m_replies;

  static QMap<QString, QASObject*> s_objects;
};

//------------------------------------------------------------------------------

class QASActivity : public QObject {
  Q_OBJECT

  QASActivity(QString id, QObject* parent);

public:
  static QASActivity* getActivity(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  QString id() const { return m_id; }
  QString verb() const { return m_verb; }
  QString content() const { return m_content; }
  QString generatorName() const { return m_generatorName; }

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }

  QASObject* object() const { return m_object; }
  QASActor* actor() const { return m_actor; }

  QDateTime published() const { return m_published; }
  QString url() const { return m_url; }

private:
  QString m_id;
  QString m_url;
  QString m_content;
  QString m_verb;
  QString m_generatorName;

  QDateTime m_published;
  QDateTime m_updated;

  QASObject* m_object;
  QASActor* m_actor;

  static QMap<QString, QASActivity*> s_activities;
};

//------------------------------------------------------------------------------

class QASObjectList : public QObject {
  Q_OBJECT
  QASObjectList(QString url, QObject* parent);

public:
  static QASObjectList* getObjectList(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  size_t size() const { return m_items.size(); }
  qulonglong totalItems() const { return m_totalItems; }
  bool hasMore() const { return m_hasMore; }
  QString url() const { return m_url; }
  QString urlOrProxy() const {
    return m_proxyUrl.isEmpty() ? m_url : m_proxyUrl; 
  }

  QASObject* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

signals:
  void changed();

private:
  QString m_url;
  QString m_proxyUrl;
  qulonglong m_totalItems;
  QList<QASObject*> m_items;
  bool m_hasMore;
  
  static QMap<QString, QASObjectList*> s_objectLists;
};

//------------------------------------------------------------------------------

class QASCollection : public QObject {
  Q_OBJECT

public:
  QASCollection(QObject* parent);
  QASCollection(QVariantMap json, QObject* parent);

  size_t size() const { return m_items.size(); }
  QString nextUrl() const { return m_nextUrl; }

  QASActivity* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

private:
  QString m_displayName;
  QString m_url;
  qulonglong m_totalItems;
  QList<QASActivity*> m_items;

  QString m_nextUrl;
};

#endif /* _QACTIVITYSTREAMS_H_ */

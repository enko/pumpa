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

#define QAS_RESPONSE_NULL     0
#define QAS_FETCH_INBOX       1
#define QAS_NEW_POST          2
#define QAS_FETCH_REPLIES     3

//------------------------------------------------------------------------------

qint64 sortIntByDateTime(QDateTime dt);

//------------------------------------------------------------------------------

class QASActor : public QObject {
  Q_OBJECT

private:
  QASActor(QString id="", QObject* parent=0);

public:
  static QASActor* getActor(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

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
  QASObject(QString id="", QObject* parent=0);

public:
  static QASObject* getObject(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }

  QString id() const { return m_id; }
  QString content() const { return m_content; }
  QString url() const { return m_url; }
  QDateTime published() const { return m_published; }
  bool liked() const { return m_liked; }
  bool hasReplies() const;
  QASObjectList* replies() const { return m_replies; }

  QASActor* author() const { return m_author; }

signals:
  void changed();

private:
  QString m_id;
  QString m_content;
  bool m_liked;
  QString m_objectType;
  QString m_url;

  QDateTime m_published;
  QDateTime m_updated;

  QASActor* m_author;
  QASObjectList* m_replies;

  static QMap<QString, QASObject*> s_objects;
};

//------------------------------------------------------------------------------

class QASActivity : public QObject {
  Q_OBJECT

  QASActivity(QString id="", QObject* parent=0);

public:
  static QASActivity* getActivity(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  QString id() const { return m_id; }

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

  QDateTime m_published;
  QDateTime m_updated;

  QASObject* m_object;
  QASActor* m_actor;

  static QMap<QString, QASActivity*> s_activities;
};

//------------------------------------------------------------------------------

class QASObjectList : public QObject {
  Q_OBJECT
  QASObjectList(QString url="", QObject* parent=0);

public:
  static QASObjectList* getObjectList(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  size_t size() const { return m_items.size(); }
  qulonglong totalItems() const { return m_totalItems; }
  bool hasMore() const { return size() < m_totalItems; }
  QString url() const { return m_url; }

  QASObject* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

signals:
  void changed();

private:
  QString m_url;
  qulonglong m_totalItems;
  QList<QASObject*> m_items;
  
  static QMap<QString, QASObjectList*> s_objectLists;
};

//------------------------------------------------------------------------------

class QASCollection : public QObject {
  Q_OBJECT

public:
  QASCollection(QObject* parent=0);
  QASCollection(QVariantMap json, QObject* parent=0);

  size_t size() const { return m_items.size(); }

  QASActivity* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

private:
  QString m_displayName;
  QString m_url;
  size_t m_totalItems;
  QList<QASActivity*> m_items;
};

#endif /* _QACTIVITYSTREAMS_H_ */

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

#include <QSet>
#include <QObject>
#include <QDateTime>
#include <QVariantMap>

//------------------------------------------------------------------------------
// Forward declarations

class QASCollection;
class QASObjectList;
class QASActorList;
class QASActor;

//------------------------------------------------------------------------------

qint64 sortIntByDateTime(QDateTime dt);

//------------------------------------------------------------------------------

class QASAbstractObject : public QObject {
  Q_OBJECT
protected:
  QASAbstractObject(QObject* parent);
  virtual void connectSignals(QASAbstractObject* obj);

signals:
  void changed();
  void request(QString, int);
};

//------------------------------------------------------------------------------

class QASObject : public QASAbstractObject {
  Q_OBJECT

protected:
  QASObject(QString id, QObject* parent);

public:
  static QASObject* getObject(QVariantMap json, QObject* parent);
  static QASObject* getObject(QString id) { 
    return s_objects.contains(id) ? s_objects[id] : NULL;
  }
  virtual void update(QVariantMap json);

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }
  
  QString id() const { return m_id; }
  QString content() const { return m_content; }
  QString type() const { return m_objectType; }
  QString url() const { return m_url; }
  QString imageUrl() const { return m_imageUrl; }
  QString displayName() const { return m_displayName; }
  QString apiLink() const;

  QDateTime published() const { return m_published; }

  void refresh();

  void toggleLiked();
  bool liked() const { return m_liked; }
  size_t numLikes() const;
  void addLike(QASActor* actor, bool like);
  QASActorList* likes() const { return m_likes; }

  bool shared() const { return m_shared; }
  size_t numShares() const;
  QASActorList* shares() const { return m_shares; }

  size_t numReplies() const;
  QASObjectList* replies() const { return m_replies; }
  void addReply(QASObject* obj);

  QASActor* author() const { return m_author; }
  void setAuthor(QASActor* a) { m_author = a; }
  QASObject* inReplyTo() const { return m_inReplyTo; }

  // currently just a minimal variant needed for the API e.g. when
  // favouriting the object
  QVariantMap toJson() const;

protected:
  QString m_id;
  QString m_content;
  bool m_liked;
  bool m_shared;
  QString m_objectType;
  QString m_url;
  QString m_imageUrl;
  QString m_displayName;
  QString m_apiLink;
  QString m_proxyUrl;

  QDateTime m_published;
  QDateTime m_updated;

  QASObject* m_inReplyTo;
  QASActor* m_author;
  QASObjectList* m_replies;
  QASActorList* m_likes;
  QASActorList* m_shares;

private:
  static QMap<QString, QASObject*> s_objects;
};

//------------------------------------------------------------------------------

class QASActor : public QASObject {
  Q_OBJECT

private:
  QASActor(QString id, QObject* parent);

public:
  static QASActor* getActor(QVariantMap json, QObject* parent);
  virtual void update(QVariantMap json);

  QString displayNameOrYou() const { return isYou() ? "You" : displayName(); }
  bool isYou() const { return m_isYou; }
  void setYou() { m_isYou = true; }

private:
  static QMap<QString, QASActor*> s_actors;
  bool m_isYou;
};

//------------------------------------------------------------------------------

class QASActivity : public QASAbstractObject {
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

class QASObjectList : public QASAbstractObject {
  Q_OBJECT

protected:
  QASObjectList(QString url, QObject* parent);

public:
  static QASObjectList* getObjectList(QVariantMap json, QObject* parent, 
                                      int id=0);
  void update(QVariantMap json, bool older);

  void addObject(QASObject* obj);

  size_t size() const { return m_items.size(); }
  qulonglong totalItems() const { return m_totalItems; }
  bool hasMore() const { return m_hasMore; }
  QString url() const { return m_url; }
  QString urlOrProxy() const;
  virtual void refresh();

  virtual QASObject* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

signals:
  void changed();

protected:
  QString m_url;
  QString m_proxyUrl;
  qulonglong m_totalItems;
  QList<QASObject*> m_items;
  bool m_hasMore;

private:
  static QMap<QString, QASObjectList*> s_objectLists;
};

//------------------------------------------------------------------------------

class QASActorList : public QASObjectList {
  Q_OBJECT

protected:
  QASActorList(QString url, QObject* parent);

public:
  static QASActorList* getActorList(QVariantMap json, QObject* parent,
                                    int id=0);

  virtual QASActor* at(size_t i) const;

  void addActor(QASActor* actor);
  void removeActor(QASActor* actor);

  bool onlyYou() const { return size()==1 && at(0)->isYou(); }

  virtual void refresh();

private:
  static QMap<QString, QASActorList*> s_actorLists;
};

//------------------------------------------------------------------------------

class QASCollection : public QASAbstractObject {
  Q_OBJECT

protected:
  QASCollection(QString url, QObject* parent);

public:
  static QASCollection* initCollection(QString url, QObject* parent);
  static QASCollection* getCollection(QVariantMap json, QObject* parent,
                                      int id);
  void update(QVariantMap json, bool older);

  QString prevLink() const { 
    return m_prevLink.isEmpty() ? m_url : m_prevLink; 
  }
  QString nextLink() const { return m_nextLink; }

  size_t size() const { return m_items.size(); }

  QASActivity* at(size_t i) const {
    if (i >= size())
      return NULL;
    return m_items[i];
  }

signals:
  void changed(bool);

private:
  QString m_displayName;
  QString m_url;
  qulonglong m_totalItems;
  QList<QASActivity*> m_items;
  QSet<QASActivity*> m_item_set;

  QString m_prevLink, m_nextLink;

  static QMap<QString, QASCollection*> s_collections;
};

#endif /* _QACTIVITYSTREAMS_H_ */

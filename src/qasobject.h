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

#ifndef _QASOBJECT_H_
#define _QASOBJECT_H_

#include "qasabstractobject.h"

class QASActor;
class QASActorList;
class QASObjectList;

//------------------------------------------------------------------------------

class QASObject : public QASAbstractObject {
  Q_OBJECT

protected:
  QASObject(QString id, QObject* parent);

public:
  static void clearCache();

  static QASObject* getObject(QVariantMap json, QObject* parent);
  static QASObject* getObject(QString id) { 
    return s_objects.contains(id) ? s_objects[id] : NULL;
  }
  virtual void update(QVariantMap json);

  QASActor* asActor();

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }
  
  QString id() const { return m_id; }
  QString content() const { return m_content; }
  QString type() const { return m_objectType; }
  QString url() const { return m_url; }
  QString imageUrl() const { return m_imageUrl; }
  QString fullImageUrl() const { return m_fullImageUrl; }
  QString displayName() const { return m_displayName; }
  virtual QString apiLink() const;

  QDateTime published() const { return m_published; }


  void toggleLiked();
  bool liked() const { return m_liked; }
  size_t numLikes() const;
  void addLike(QASActor* actor, bool like);
  QASActorList* likes() const { return m_likes; }

  bool shared() const { return m_shared; }
  size_t numShares() const;
  void addShare(QASActor* actor);
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

  virtual bool isDeleted() const { return !m_deleted.isNull(); }

protected:
  QString m_id;
  QString m_content;
  bool m_liked;
  bool m_shared;
  QString m_objectType;
  QString m_url;
  QString m_imageUrl;
  QString m_fullImageUrl;
  QString m_displayName;
  QString m_apiLink;
  QString m_proxyUrl;

  QDateTime m_published;
  QDateTime m_updated;
  QDateTime m_deleted;

  QASObject* m_inReplyTo;
  QASActor* m_author;
  QASObjectList* m_replies;
  QASActorList* m_likes;
  QASActorList* m_shares;

  static QMap<QString, QASObject*> s_objects;
};

#endif /* _QASOBJECT_H_ */

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
#include <QJsonObject>
#include <QDateTime>

//------------------------------------------------------------------------------

class QASActor : public QObject {
  Q_OBJECT

public:
  QASActor(QObject* parent=0);
  QASActor(QJsonObject json, QObject* parent=0);

  QString displayName() const { return m_displayName; }

private:
  QString m_preferredUsername;
  QString m_url;
  QString m_displayName;
  QString m_id;
};

//------------------------------------------------------------------------------

class QASObject : public QObject {
  Q_OBJECT

public:
  QASObject(QObject* parent=0);
  QASObject(QJsonObject json, QObject* parent=0);

  QString content() const { return m_content; }
  QString url() const { return m_url; }

private:
  QString m_content;
  bool m_liked;
  QString m_objectType;
  QString m_id;
  QString m_url;
};

//------------------------------------------------------------------------------

class QASActivity : public QObject {
  Q_OBJECT

public:
  QASActivity(QObject* parent=0);
  QASActivity(QJsonObject json, QObject* parent=0);

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
};

//------------------------------------------------------------------------------

class QASCollection : public QObject {
  Q_OBJECT

public:
  QASCollection(QObject* parent=0);
  QASCollection(QJsonObject json, QObject* parent=0);

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

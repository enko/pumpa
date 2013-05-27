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

//------------------------------------------------------------------------------

class QASObject : public QObject {
  Q_OBJECT

public:
  QASObject(QObject* parent=0);
  QASObject(QJsonObject json, QObject* parent=0);

  QString content() const { return m_content; }

private:
  QString m_content;
  bool liked;
};

//------------------------------------------------------------------------------

class QASActivity : public QObject {
  Q_OBJECT

public:
  QASActivity(QObject* parent=0);
  QASActivity(QJsonObject json, QObject* parent=0);

  QASObject* object() const { return m_object; }

private:
  QString id;
  QString url;
  QString content;

  QASObject* m_object;
  QString verb;
};

//------------------------------------------------------------------------------

class QASCollection : public QObject {
  Q_OBJECT

public:
  QASCollection(QObject* parent=0);
  QASCollection(QJsonObject json, QObject* parent=0);

  size_t size() const { return items.size(); }

  QASActivity* at(size_t i) const {
    if (i >= size())
      return NULL;
    return items[i];
  }

private:
  QString displayName;
  QString url;
  size_t totalItems;
  QList<QASActivity*> items;
};

#endif /* _QACTIVITYSTREAMS_H_ */

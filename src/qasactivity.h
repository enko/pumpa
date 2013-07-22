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

#ifndef _QASACTIVITY_H_
#define _QASACTIVITY_H_

#include "qasabstractobject.h"
#include "qasobject.h"
#include "qasactor.h"
#include "qasobjectlist.h"


//------------------------------------------------------------------------------

class QASActivity : public QASAbstractObject {
  Q_OBJECT

  QASActivity(QString id, QObject* parent);

public:
  static void clearCache();

  static QASActivity* getActivity(QVariantMap json, QObject* parent);
  void update(QVariantMap json);

  virtual QString apiLink() const { return id(); }

  QString id() const { return m_id; }
  QString verb() const { return m_verb; }
  QString content() const { return m_content; }
  QString generatorName() const { return m_generatorName; }

  qint64 sortInt() const { return sortIntByDateTime(m_updated); }

  QASObject* object() const { return m_object; }
  QASActor* actor() const { return m_actor; }

  QDateTime published() const { return m_published; }
  QString url() const { return m_url; }

  bool hasTo() const;
  bool hasCc() const;

  QASObjectList* to() const { return m_to; }
  QASObjectList* cc() const { return m_cc; }

  static bool isLikeVerb(QString verb) {
    return (verb == "favorite" || verb == "like" ||
            verb == "unfavorite" || verb == "unlike");
  }

  virtual bool isDeleted() const { 
    return m_verb == "post" && m_object && m_object->isDeleted();
  }

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
  
  QASObjectList* m_to;
  QASObjectList* m_cc;

  static QMap<QString, QASActivity*> s_activities;
};

#endif /* _QASACTIVITY_H_ */

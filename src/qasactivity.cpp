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

#include "qasactivity.h"
#include "util.h"

#include <QDebug>

//------------------------------------------------------------------------------

QMap<QString, QASActivity*> QASActivity::s_activities;

void QASActivity::clearCache() { deleteMap<QASActivity*>(s_activities); }

//------------------------------------------------------------------------------

QASActivity::QASActivity(QString id, QObject* parent) : 
  QASAbstractObject(QAS_ACTIVITY, parent),
  m_id(id),
  m_object(NULL),
  m_actor(NULL),
  m_to(NULL),
  m_cc(NULL)
{
#ifdef DEBUG_QAS
  qDebug() << "new Activity" << m_id;
#endif
}

//------------------------------------------------------------------------------

void QASActivity::update(QVariantMap json) {
#ifdef DEBUG_QAS
  qDebug() << "updating Activity" << m_id;
#endif
  bool ch = false;

  updateVar(json, m_verb, "verb", ch);
  updateVar(json, m_url, "url", ch);
  updateVar(json, m_content, "content", ch);
  
  if (json.contains("actor")) {
    m_actor = QASActor::getActor(json["actor"].toMap(), parent());
    connectSignals(m_actor);
  }

  if (json.contains("object")) {
    m_object = QASObject::getObject(json["object"].toMap(), parent(),
                                    isLikeVerb(m_verb));
    connectSignals(m_object);
    if (!m_object->author())
      m_object->setAuthor(m_actor);
  }

  updateVar(json, m_published, "published", ch);
  updateVar(json, m_updated, "updated", ch);
  updateVar(json, m_generatorName, "generator", "displayName", ch);

  if (m_verb == "post" && m_object && m_object->inReplyTo())
    m_object->inReplyTo()->addReply(m_object);

  if (isLikeVerb(m_verb) && m_object && m_actor) 
    m_object->addLike(m_actor, !m_verb.startsWith("un"));

  if (m_verb == "share" && m_object && m_actor) 
    m_object->addShare(m_actor);

  if (json.contains("to"))
    m_to = QASObjectList::getObjectList(json["to"].toList(), parent());

  if (json.contains("cc"))
    m_cc = QASObjectList::getObjectList(json["cc"].toList(), parent());

  if (ch)
    emit changed();
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

bool QASActivity::hasTo() const { 
  return m_to && m_to->size(); 
}

//------------------------------------------------------------------------------

bool QASActivity::hasCc() const { 
  return m_cc && m_cc->size(); 
}


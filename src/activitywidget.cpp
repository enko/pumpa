/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "activitywidget.h"
#include "texttoolbutton.h"
#include "pumpa_defines.h"

#include <QDebug>

//------------------------------------------------------------------------------

ActivityWidget::ActivityWidget(QASActivity* a, QWidget* parent) :
  QFrame(parent),
  m_irtObjectWidget(NULL),
  m_activity(a)
{
  const QString verb = m_activity->verb();
  QASObject* obj = m_activity->object();
  QString objType = obj->type();

  bool showObject = (verb == "post");

  QHBoxLayout* topLayout = new QHBoxLayout;
  topLayout->setContentsMargins(0, 0, 0, 0);
  m_textLabel = new RichTextLabel(this);
  topLayout->addWidget(m_textLabel, 0, Qt::AlignTop);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));

  m_objectWidget = makeObjectWidgetAndConnect(obj, !showObject);
  connect(m_objectWidget, SIGNAL(moreClicked()), this, SLOT(onMoreClicked()));

  if (objType == "comment") {
    QASObject* irtObj = obj->inReplyTo();
    if (irtObj) {
      m_irtObjectWidget = makeObjectWidgetAndConnect(irtObj, true);
      connect(m_irtObjectWidget, SIGNAL(moreClicked()),
              this, SLOT(hideOriginalObject()));
      if (irtObj->url().isEmpty()) {
        m_irtObjectWidget->setVisible(false);
        irtObj->refresh();
      } else if (!showObject) {
        m_irtObjectWidget->setVisible(false);
      }
    }
  }
  QVBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  //  layout->addWidget(m_textLabel, 0, Qt::AlignTop);
  layout->addLayout(topLayout);
  if (m_irtObjectWidget)
      layout->addWidget(m_irtObjectWidget, 0, Qt::AlignTop);
  layout->addWidget(m_objectWidget, 0, Qt::AlignTop);
  layout->addWidget(new QLabel("<hr />"));

  updateText();

  setLayout(layout);
}

//------------------------------------------------------------------------------

void ActivityWidget::onObjectChanged() {
  updateText();
}

//------------------------------------------------------------------------------

void ActivityWidget::refreshTimeLabels() {
  updateText();
}

//------------------------------------------------------------------------------

void ActivityWidget::hideOriginalObject() {
  if (!m_irtObjectWidget)
    return;

  QASObject* obj = m_activity->object();
  QASObjectList* replies = m_irtObjectWidget->object()->replies();
  if (replies && replies->contains(obj))
    m_objectWidget->setVisible(false);
}

//------------------------------------------------------------------------------

void ActivityWidget::onMoreClicked() {
  if (!m_irtObjectWidget)
    return;

  m_irtObjectWidget->setVisible(true);
}

//------------------------------------------------------------------------------

void ActivityWidget::updateText() {
  QString verb = m_activity->verb();
  QString text = m_activity->content();
  QString objType = m_activity->object()->type();

  QString generatorName = m_activity->generatorName();
  if (!generatorName.isEmpty() && (verb != "share"))
    text += " via " + generatorName;

  if (verb == "post" && objType == "note") {
    if (m_activity->hasTo())
      text += " To: " + recipientsToString(m_activity->to());
    
    if (m_activity->hasCc())
      text += " CC: " + recipientsToString(m_activity->cc());
  }

  m_textLabel->setText(text);
}
 
//------------------------------------------------------------------------------

QString ActivityWidget::recipientsToString(QASObjectList* rec) {
  if (!rec)
    return "";

  QStringList ret;

  for (size_t i=0; i<rec->size(); ++i) {
    QASObject* r = rec->at(i);
    if (r->type() == "collection" && r->id() == PUBLIC_RECIPIENT_ID) {
      ret << "Public";
    } else {
      QString name = r->displayName();
      QString url = r->url();

      if (url.isEmpty())
        ret << name;
      else
        ret << QString("<a href=\"%1\">%2</a>").arg(url).arg(name);
    }
  }

  return ret.join(", ");
}

//------------------------------------------------------------------------------

ObjectWidget* ActivityWidget::makeObjectWidgetAndConnect(QASObject* obj,
                                                         bool shortObject) {
  ObjectWidget* ow = new ObjectWidget(obj, this, shortObject);

  connect(ow, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  connect(ow, SIGNAL(newReply(QASObject*)),
          this,  SIGNAL(newReply(QASObject*)));
  connect(ow, SIGNAL(like(QASObject*)),
          this,  SIGNAL(like(QASObject*)));
  connect(ow, SIGNAL(share(QASObject*)),
          this,  SIGNAL(share(QASObject*)));

  connect(obj, SIGNAL(changed()), this, SLOT(onObjectChanged()),
          Qt::UniqueConnection);

  return ow;
}

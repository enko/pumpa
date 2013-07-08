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

#include "newactivitywidget.h"
#include "pumpa_defines.h"

#include <QDebug>

//------------------------------------------------------------------------------

NewActivityWidget::NewActivityWidget(QASActivity* a, QWidget* parent) :
  AbstractActivityWidget(a, parent),
  m_irtObjectWidget(NULL)
{
  const QString verb = m_activity->verb();
  QASObject* obj = m_activity->object();
  QString objType = obj->type();

  bool showObject = (verb == "post" || objType == "person");

  m_textLabel = new RichTextLabel(this);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));

  m_objectWidget = new WrappedObjectWidget(obj, this, !showObject);
  connect(m_objectWidget, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  connect(m_objectWidget, SIGNAL(newReply(QASObject*)),
          this,  SIGNAL(newReply(QASObject*)));
  connect(m_objectWidget, SIGNAL(like(QASObject*)),
          this,  SIGNAL(like(QASObject*)));
  connect(m_objectWidget, SIGNAL(share(QASObject*)),
          this,  SIGNAL(share(QASObject*)));

  connect(obj, SIGNAL(changed()), this, SLOT(onObjectChanged()),
          Qt::UniqueConnection);

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(m_textLabel, 0, Qt::AlignTop);

  if (objType == "comment") {
    QASObject* irtObj = obj->inReplyTo();
    if (irtObj) {
      m_irtObjectWidget = new WrappedObjectWidget(irtObj, this, true);
      connect(m_irtObjectWidget, SIGNAL(moreClicked()),
              this, SLOT(hideOriginalObject()));
      layout->addWidget(m_irtObjectWidget, 0, Qt::AlignTop);
      if (irtObj->url().isEmpty())
        irtObj->refresh();
    }
  }
  layout->addWidget(m_objectWidget, 0, Qt::AlignTop);
  layout->addWidget(new QLabel("<hr />"));

  updateText();

  setLayout(layout);
}

//------------------------------------------------------------------------------

void NewActivityWidget::onObjectChanged() {
  updateText();
}

//------------------------------------------------------------------------------

void NewActivityWidget::refreshTimeLabels() {
  updateText();
}

//------------------------------------------------------------------------------

void NewActivityWidget::hideOriginalObject() {
  if (!m_irtObjectWidget)
    return;

  m_objectWidget->setVisible(false);
}

//------------------------------------------------------------------------------

void NewActivityWidget::updateText() {
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

QString NewActivityWidget::recipientsToString(QASObjectList* rec) {
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

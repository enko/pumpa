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

#include <QDebug>

//------------------------------------------------------------------------------

NewActivityWidget::NewActivityWidget(QASActivity* a, QWidget* parent) :
  AbstractActivityWidget(a, parent)
{
  QASObject* obj = m_activity->object();

  m_textLabel = new RichTextLabel(this);
  // m_actorWidget = new ActorWidget(m_activity->actor(), this, true);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  updateText();

  m_objectWidget = new ObjectWidget(obj, this);
  connect(m_objectWidget, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));

  connect(obj, SIGNAL(changed()), this, SLOT(onObjectChanged()),
          Qt::UniqueConnection);


  // QHBoxLayout* m_acrossLayout = new QHBoxLayout;
  // m_acrossLayout->setSpacing(10);
  // m_acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  // m_acrossLayout->addWidget(m_textLabel, 0, Qt::AlignTop); 

  m_layout = new QVBoxLayout;
  m_layout->addWidget(m_textLabel);
  m_layout->addWidget(m_objectWidget);

  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void NewActivityWidget::updateText() {
  QString content = m_activity->content();
  // QASObject* obj = m_activity->object();
  // QString objContent = obj->content();
  // if (!objContent.isEmpty()) {
  //   objContent.replace(QRegExp("<[^>]*>"), " ");
  //   objContent = objContent.section(QRegExp("\\s+"), 0, 10,
  //                                   QString::SectionSkipEmpty);
  //   QASActor* author = obj->author();
  //   content += "<br />";
  //   if (author && !author->displayName().isEmpty())
  //     content += author->displayName() + ": ";
  //   content += "\"" + objContent + " ...\"";
  // }
  m_textLabel->setText(content);
}
 
//------------------------------------------------------------------------------

void NewActivityWidget::onObjectChanged() {
  updateText();
}

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

#include "shortactivitywidget.h"

#include <QDebug>

//------------------------------------------------------------------------------

AbstractActivityWidget::AbstractActivityWidget(QASActivity* a,
                                               QWidget* parent) :
  QFrame(parent),
  m_activity(a)
{}

//------------------------------------------------------------------------------

ShortActivityWidget::ShortActivityWidget(QASActivity* a, QWidget* parent) :
  AbstractActivityWidget(a, parent)
{
  m_textLabel = new RichTextLabel(this);
  m_actorWidget = new ActorWidget(m_activity->actor(), this, true);

  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));

  updateText();

  QHBoxLayout* m_acrossLayout = new QHBoxLayout;
  m_acrossLayout->setSpacing(10);
  m_acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  m_acrossLayout->addWidget(m_textLabel, 0, Qt::AlignTop); 

  setLayout(m_acrossLayout);
}

//------------------------------------------------------------------------------

void ShortActivityWidget::updateText() {
  QString content = m_activity->content();
  QASObject* obj = m_activity->object();
  QString objContent = obj->content();
  if (!objContent.isEmpty()) {
    objContent.replace(QRegExp("<[^>]*>"), " ");
    objContent = objContent.section(QRegExp("\\s+"), 0, 10,
                                    QString::SectionSkipEmpty);
    QASActor* author = obj->author();
    content += "<br />";
    if (author && !author->displayName().isEmpty())
      content += author->displayName() + ": ";
    content += "\"" + objContent + " ...\"";
  }
  m_textLabel->setText(content);
}
 
//------------------------------------------------------------------------------

void ShortActivityWidget::onObjectChanged() {
  updateText();
}

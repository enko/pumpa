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

#include "objectwidget.h"

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QASObject* obj, QWidget* parent, bool shortWidget) : 
  QFrame(parent),
  m_object(obj),
  m_short(shortWidget)
{
  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));

  m_layout = new QVBoxLayout;
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);

  m_objectWidget = new FullObjectWidget(m_object, parent);
  connect(m_objectWidget, SIGNAL(linkHovered(const QString&)),
          this, SIGNAL(linkHovered(const QString&)));
  connect(m_objectWidget, SIGNAL(like(QASObject*)),
          this, SIGNAL(like(QASObject*)));
  connect(m_objectWidget, SIGNAL(share(QASObject*)),
          this, SIGNAL(share(QASObject*)));
  connect(m_objectWidget, SIGNAL(newReply(QASObject*)),
          this, SIGNAL(newReply(QASObject*)));
  m_layout->addWidget(m_objectWidget);

  if (m_short) {
    m_shortObjectWidget = new ShortObjectWidget(m_object, parent);
    connect(m_shortObjectWidget, SIGNAL(moreClicked()),
            this, SLOT(showMore()));
    m_objectWidget->setVisible(false);
    m_layout->addWidget(m_shortObjectWidget);
  }
  
  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void ObjectWidget::showMore() {
  if (!m_short || !m_shortObjectWidget)
    return;
  m_short = false;
  m_shortObjectWidget->setVisible(false);
  m_objectWidget->setVisible(true);
  emit moreClicked();
}
  
//------------------------------------------------------------------------------

void ObjectWidget::onChanged() {
  setVisible(!m_object->url().isEmpty());
}

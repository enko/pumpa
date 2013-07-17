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
  ObjectWidgetWithSignals(parent),
  m_objectWidget(NULL),
  m_shortObjectWidget(NULL),
  m_contextLabel(NULL),
  m_contextButton(NULL),
  m_topLayout(NULL),
  m_object(obj),
  m_irtObject(NULL),
  m_short(shortWidget)
{
  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));

  m_layout = new QVBoxLayout;
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->setSpacing(0);

  // Add label with context "Re:" text and "show context" button for
  // replies.
  if (m_object->type() == "comment") {
    m_irtObject = m_object->inReplyTo();
    if (m_irtObject) {
      m_topLayout = new QHBoxLayout;
      m_topLayout->setContentsMargins(0, 0, 0, 0);

      m_contextLabel = new RichTextLabel(this, true);
      connect(m_irtObject, SIGNAL(changed()), this, SLOT(updateContextLabel()));
      m_topLayout->addWidget(m_contextLabel, 0, Qt::AlignVCenter);

      m_topLayout->addSpacing(10);
      m_contextButton = new TextToolButton(this);
      connect(m_contextButton, SIGNAL(clicked()), this, SLOT(onShowContext()));
      m_topLayout->addWidget(m_contextButton, 0, Qt::AlignVCenter);

      if (m_irtObject->url().isEmpty()) {
        m_contextLabel->setVisible(false);
        m_contextButton->setVisible(false);
        m_irtObject->refresh();
      } else {
        updateContextLabel();
      }
      m_layout->addLayout(m_topLayout);
    }
  }

  m_objectWidget = new FullObjectWidget(m_object, parent);
  ObjectWidgetWithSignals::connectSignals(m_objectWidget, this);
  m_layout->addWidget(m_objectWidget);

  if (m_short) {
    m_shortObjectWidget = new ShortObjectWidget(m_object, parent);
    connect(m_shortObjectWidget, SIGNAL(moreClicked()),
            this, SLOT(showMore()));
    if (m_contextLabel)
      m_contextLabel->setVisible(false);
    if (m_contextButton)
      m_contextButton->setVisible(false);
    m_objectWidget->setVisible(false);
    m_layout->addWidget(m_shortObjectWidget);
  }
  
  QASActor* author = m_object->author();
  if (author && author->url().isEmpty())
    m_object->refresh();

  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void ObjectWidget::showMore() {
  if (!m_short || !m_shortObjectWidget)
    return;

  m_short = false;
  m_shortObjectWidget->setVisible(false);
  m_objectWidget->setVisible(true);
  if (m_contextLabel && !m_irtObject->url().isEmpty())
    m_contextLabel->setVisible(true);
  if (m_contextButton && !m_irtObject->url().isEmpty())
    m_contextButton->setVisible(true);
  emit moreClicked();
}
  
//------------------------------------------------------------------------------

void ObjectWidget::onChanged() {
  setVisible(!m_object->url().isEmpty() && !m_object->isDeleted());
}

//------------------------------------------------------------------------------

void ObjectWidget::updateContextLabel() {
  if (!m_irtObject || !m_contextLabel)
    return;

  QString text = ShortObjectWidget::objectExcerpt(m_irtObject);
  m_contextLabel->setText("Re: " + text);
  m_contextButton->setText("Show context");
  if (!m_short && !m_irtObject->url().isEmpty()) {
    m_contextLabel->setVisible(true);
    m_contextButton->setVisible(true);
  }
}

//------------------------------------------------------------------------------

void ObjectWidget::onShowContext() {
  if (!m_irtObject || !m_topLayout)
    return;

  emit showContext(m_irtObject);
}
    
//------------------------------------------------------------------------------

void ObjectWidget::refreshTimeLabels() {
  if (m_objectWidget)
    m_objectWidget->refreshTimeLabels();
  if (m_shortObjectWidget)
    m_shortObjectWidget->refreshTimeLabels();
}

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

#include "shortobjectwidget.h"
#include "actorwidget.h"
#include "util.h"

#include <QVBoxLayout>
#include <QToolButton>

//------------------------------------------------------------------------------

ShortObjectWidget::ShortObjectWidget(QASObject* obj, QWidget* parent) :
  QFrame(parent),
  m_object(obj)
{
  m_textLabel = new RichTextLabel(this);

  QASActor* actor = qobject_cast<QASActor*>(m_object);
  if (!actor)
    actor = m_object->author();
  ActorWidget* actorWidget = new ActorWidget(actor, this, true);

  QToolButton* moreButton = new QToolButton(this);
  moreButton->setText("+");
  moreButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  moreButton->setFocusPolicy(Qt::NoFocus);
  connect(moreButton, SIGNAL(clicked()), this, SIGNAL(moreClicked()));

  QHBoxLayout* acrossLayout = new QHBoxLayout;
  acrossLayout->setSpacing(10);
  acrossLayout->addWidget(actorWidget, 0, Qt::AlignTop);
  acrossLayout->addWidget(m_textLabel, 0, Qt::AlignTop);
  acrossLayout->addWidget(moreButton, 0, Qt::AlignTop);

  updateText();
  
  setLayout(acrossLayout);

  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));
}

//------------------------------------------------------------------------------

void ShortObjectWidget::updateText() {
  QString text = m_object->displayName();
  if (text.isEmpty()) {
    text = m_object->content();
    if (!text.isEmpty()) {
      text.replace(QRegExp(HTML_TAG_REGEX), " ");
      text = text.section(QRegExp("\\s+"), 0, 8, QString::SectionSkipEmpty) +
        " ...";
    }
  }

  QASActor* author = m_object->author();
  if (author && !author->displayName().isEmpty())
    text = author->displayName() + ": \"" + text + "\"";

  m_textLabel->setText(text);
}

//------------------------------------------------------------------------------

void ShortObjectWidget::onChanged() {
  updateText();
}

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

#include "contextwidget.h"
#include "pumpa_defines.h"
#include "activitywidget.h"

#include <QDebug>

//------------------------------------------------------------------------------

ContextWidget::ContextWidget(QWidget* parent) :
  ASWidget(parent),
  m_numReplies(-1),
  m_object(NULL)
{}

//------------------------------------------------------------------------------

void ContextWidget::setObject(QASObject* obj) {
  clear();
  
  m_object = obj;
  connect(m_object, SIGNAL(changed()), this, SLOT(update()),
          Qt::UniqueConnection);

  ObjectWidget* ow = new ObjectWidget(m_object, this);
  ObjectWidgetWithSignals::connectSignals(ow, this);
  connect(ow, SIGNAL(showContext(QASObject*)),
          this, SIGNAL(showContext(QASObject*)));

  m_itemLayout->insertWidget(0, ow);
  m_itemLayout->addStretch();

  m_object->refresh();
  if (m_object->replies())
    m_object->replies()->refresh();

  updateNumReplies();
  m_firstTime = false;
}

//------------------------------------------------------------------------------

bool ContextWidget::updateNumReplies() {
  int oldNr = m_numReplies;
  m_numReplies = m_object->replies() ? m_object->replies()->size() : -1;

  return oldNr != m_numReplies;
}


//------------------------------------------------------------------------------

void ContextWidget::update() {
  if (!isVisible() && updateNumReplies())
    emit highlightMe();
}


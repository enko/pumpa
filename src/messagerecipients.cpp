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

#include "messagerecipients.h"

#include <QDebug>

//------------------------------------------------------------------------------

MessageRecipients::MessageRecipients(QWidget* parent) : QWidget(parent) {
  m_layout = new QGridLayout;
  m_layout->setColumnStretch(0, 10);
  m_layout->setColumnStretch(1, 1);
  m_layout->setContentsMargins(0, 0, 0, 0);

  m_buttonMapper = new QSignalMapper(this);
  connect(m_buttonMapper, SIGNAL(mapped(QObject*)),
          this, SLOT(onRemoveClicked(QObject*)));
  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void MessageRecipients::addRecipient(QASObject* obj) {
  if (m_widgets.contains(obj))
    return;

  QString text("<b>" + obj->displayName() + "</b>");

  QASActor* actor = obj->asActor();
  if (actor) 
    text += " (" + actor->webFinger() + ")";
  
  QLabel* label = new QLabel(text);
  QToolButton* button = new QToolButton;
  button->setText(tr("-"));
  connect(button, SIGNAL(clicked()), m_buttonMapper, SLOT(map()));
  m_buttonMapper->setMapping(button, obj);

  m_list.append(obj);
  int row = m_list.size()-1;
  m_layout->addWidget(label, row, 0);
  m_layout->addWidget(button, row, 1);

  m_widgets.insert(obj, qMakePair(label, button));
}

//------------------------------------------------------------------------------

void MessageRecipients::removeRecipient(QASObject* obj) {
  QPair<QLabel*, QToolButton*> widgets = m_widgets[obj];
  m_layout->removeWidget(widgets.first);
  m_layout->removeWidget(widgets.second);
  widgets.first->deleteLater();
  widgets.second->deleteLater();
  m_list.removeAll(obj);
  m_widgets.remove(obj);
}

//------------------------------------------------------------------------------

void MessageRecipients::clear() {
  QLayoutItem* item;
  while ((item = m_layout->takeAt(0)) != 0) {
    if (dynamic_cast<QWidgetItem*>(item)) {
      QWidget* w = item->widget();
      delete w;
    }
    delete item;
  }
  m_list.clear();
  m_widgets.clear();
}

//------------------------------------------------------------------------------

void MessageRecipients::onRemoveClicked(QObject* qObj) {
  QASObject* obj = qobject_cast<QASObject*>(qObj);
  if (!obj)
    return;

  removeRecipient(obj);
}

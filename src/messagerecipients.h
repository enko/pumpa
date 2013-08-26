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

#ifndef MESSAGE_RECIPIENTS_H
#define MESSAGE_RECIPIENTS_H

#include <QLabel>
#include <QWidget>
#include <QGridLayout>
#include <QToolButton>
#include <QSignalMapper>

#include "qasactor.h"

//------------------------------------------------------------------------------

class MessageRecipients : public QWidget {
  Q_OBJECT

public:
  MessageRecipients(QWidget* parent=0);

  void clear();

  void addRecipient(QASObject* obj);
  void removeRecipient(QASObject* obj);

  RecipientList recipients() const { return m_list; }

private slots:
  void onRemoveClicked(QObject*);

private:
  RecipientList m_list;
  QGridLayout* m_layout;

  QSignalMapper* m_buttonMapper;
  QMap<QASObject*, QPair<QLabel*, QToolButton*> > m_widgets;
};

#endif

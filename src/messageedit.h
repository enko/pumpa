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

#ifndef MESSAGE_EDIT_H
#define MESSAGE_EDIT_H

#include <QTextEdit>
#include <QKeyEvent>
#include <QCompleter>
#include <QStringListModel>

#include "qasactor.h"
#include "fancyhighlighter.h"

//------------------------------------------------------------------------------

class MessageEdit : public QTextEdit {
  Q_OBJECT
public:
  MessageEdit(QWidget* parent=0);

  typedef QMap<QString, QASActor*> completion_t;
  void setCompletions(const completion_t* completions);
  void hideCompletion();

signals:
  void ready();
  void addRecipient(QASActor*);

protected slots:
  void insertCompletion(QString);

protected:
  virtual void focusInEvent(QFocusEvent *event);
  virtual void keyPressEvent(QKeyEvent* event);
  QString wordAtCursor() const;

  FancyHighlighter* m_highlighter;
  QCompleter* m_completer;
  QStringListModel* m_model;
  const completion_t* m_completions;
};

#endif

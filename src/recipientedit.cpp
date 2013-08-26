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

#include "recipientedit.h"

#include <QStringListModel>
#include <QDebug>

//------------------------------------------------------------------------------

RecipientEdit::RecipientEdit(QWidget* parent) : 
  QLineEdit(parent),
  m_completer(NULL) 
{
}

//------------------------------------------------------------------------------

void RecipientEdit::setChoices(QStringList choices) {
  m_choices = choices;
  if (m_completer)
    delete m_completer;

  m_completer = new QCompleter(this);
  m_completer->setWidget(this);

  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  QStringListModel* model = new QStringListModel(choices);
  m_completer->setModel(model);
  //m_completer->setCompletionMode(QCompleter::PopupCompletion);
  
  connect(m_completer, SIGNAL(activated(QString)),
          this, SLOT(insertCompletion(QString)));
}

//------------------------------------------------------------------------------

void RecipientEdit::keyPressEvent(QKeyEvent* event) {
  int key = event->key();

  QLineEdit::keyPressEvent(event);

  QPair<int, int> wordPos = wordPosAtCursor();
  QString completionPrefix = text().mid(wordPos.first, wordPos.second);
  qDebug() << "completionPrefix" << completionPrefix;

  if (completionPrefix != m_completer->completionPrefix()) {
    m_completer->setCompletionPrefix(completionPrefix);
    m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
  }

  if (!event->text().isEmpty() && completionPrefix.length() > 2) {
    m_completer->complete();
  }
}

//------------------------------------------------------------------------------

QPair<int, int> RecipientEdit::wordPosAtCursor() {
  static QRegExp whitespace("\\s+");

  int cur = cursorPosition();
  QString txt = text();

  int startPos = cur ? txt.lastIndexOf(whitespace, cur-1) + 1 : 0;

  int endPos = txt.indexOf(whitespace, cur);
  if (endPos == -1)
    endPos = txt.count();

  int len = endPos - startPos;

  return qMakePair(startPos, len);
}

//------------------------------------------------------------------------------

void RecipientEdit::insertCompletion(QString completion) {
  QPair<int, int> wordPos = wordPosAtCursor();
  setSelection(wordPos.first, wordPos.second);
  insert(completion);
}

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

#include "messageedit.h"

//------------------------------------------------------------------------------

MessageEdit::MessageEdit(QWidget* parent) : QTextEdit(parent) {
  setAcceptRichText(false);
  highlighter = new FancyHighlighter(document());
}

//------------------------------------------------------------------------------

void MessageEdit::keyPressEvent(QKeyEvent* event) {
  int key = event->key();
  int mods = event->modifiers();

  if (key == Qt::Key_Return && (mods & Qt::ControlModifier)) {
    emit ready();
  // } else if (key == Qt::Key_Tab && mods == Qt::NoModifier) {
  //   complete();    
  } else {
    QTextEdit::keyPressEvent(event);
  }
}

//------------------------------------------------------------------------------

// void MessageEdit::complete() {
//   QTextCursor cur = textCursor();
//   cur.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
//   cur.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor);

//   QString word = cur.selectedText().trimmed();
//   if (word.isEmpty())
//     return;

//   QChar ch = word.at(0);

//   // qDebug() << "complete():" << word;

//   // // Auto completion only for groups 
//   // if (ch != '!' && ch != '@')
//   //   return;
  
//   // word = word.mid(1);

//   // QStringList list;

//   // if (ch == '!')
//   //   list = YAICSWindow::getGroups();
//   // else if (ch == '@')
//   //   list = YAICSWindow::getFriends();

//   // if (list.isEmpty())
//   //   return;

//   // QStringList completions = list.filter(QRegExp("^"+word));
//   // // qDebug() << "complete():" << completions;

//   // if (completions.count() != 1)
//   //   return;

//   // QString cWord = completions.at(0);
//   // insertPlainText(cWord.mid(word.count()));
// }

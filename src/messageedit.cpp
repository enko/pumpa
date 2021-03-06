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

#include <QDebug>
#include <QScrollBar>
#include <QAbstractItemView>

//------------------------------------------------------------------------------

MessageEdit::MessageEdit(QWidget* parent) : QTextEdit(parent),
                                            m_completions(NULL)
{
  setAcceptRichText(false);

  m_highlighter = new FancyHighlighter(document());

  m_completer = new QCompleter(this);
  m_completer->setWidget(this);

  m_completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_completer->setCompletionMode(QCompleter::PopupCompletion);
  m_completer->setModelSorting(QCompleter::UnsortedModel);
  m_completer->setMaxVisibleItems(10);
  
  m_model = new QStringListModel(this);
  m_completer->setModel(m_model);

  connect(m_completer, SIGNAL(activated(QString)),
          this, SLOT(insertCompletion(QString)));
}

//------------------------------------------------------------------------------

void MessageEdit::setCompletions(const completion_t* completions) {
  m_completions = completions;
  m_model->setStringList(m_completions->keys());
}

//------------------------------------------------------------------------------

void MessageEdit::hideCompletion() {
  if (m_completer) 
    m_completer->popup()->hide();
}

//------------------------------------------------------------------------------

// completion code partially from here:
// http://qt-project.org/forums/viewthread/5376

void MessageEdit::keyPressEvent(QKeyEvent* event) {
  int key = event->key();
  int mods = event->modifiers();

  QAbstractItemView* popup = m_completer->popup();

  if (popup->isVisible()) {
    switch (key) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Tab:
    case Qt::Key_Escape:
      hideCompletion();
      event->ignore();
      return;
    }
  }

  if (key == Qt::Key_Return && (mods & Qt::ControlModifier)) {
    emit ready();
    return;
  }

  QTextEdit::keyPressEvent(event);

  const QString completionPrefix = wordAtCursor();

  if (completionPrefix != m_completer->completionPrefix()) {
    m_completer->setCompletionPrefix(completionPrefix);
    QModelIndex idx = m_completer->completionModel()->index(0, 0);
    popup->setCurrentIndex(idx);
  }

  if (!event->text().isEmpty() && completionPrefix.length() >= 2) {
    QRect r = cursorRect();
    r.setWidth(popup->sizeHintForColumn(0) +
               popup->verticalScrollBar()->sizeHint().width());
    r.setHeight(popup->sizeHintForRow(0));
    m_model->setStringList(m_completions->keys());
    m_completer->complete(r);
  } else {
    hideCompletion();
  }
}

//------------------------------------------------------------------------------

void MessageEdit::insertCompletion(QString completion) {
  if (m_completer->widget() != this)
    return;

  // qDebug() << "insertCompletion" << completion;

  hideCompletion();

  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);

  tc.removeSelectedText();
  tc.deletePreviousChar();

  QASActor* actor = m_completions->value(completion);
  QString newText = QString("[%1](%2)").arg(actor->displayName()).
    arg(actor->url());
  tc.insertText(newText);
  setTextCursor(tc);

  emit addRecipient(actor);
}

//------------------------------------------------------------------------------
 
QString MessageEdit::wordAtCursor() const {
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  if (tc.document()->characterAt(tc.selectionStart()-1) == '@')
    return tc.selectedText();
  return "";
}

//------------------------------------------------------------------------------

void MessageEdit::focusInEvent(QFocusEvent *event) {
  if (m_completer)
    m_completer->setWidget(this);
  QTextEdit::focusInEvent(event);
}

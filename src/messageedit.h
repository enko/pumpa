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

//------------------------------------------------------------------------------

class MessageEdit : public QTextEdit {
  Q_OBJECT
public:
  MessageEdit(QWidget* parent=0);

  void setCompletions(const QMap<QString, QString>* completions);
  void hideCompletion() {
    if (m_completer) m_completer->popup()->hide();
  }

signals:
  void ready();

protected slots:
  void insertCompletion(QString);

protected:
  virtual void focusInEvent(QFocusEvent *event);
  virtual void keyPressEvent(QKeyEvent* event);
  QString wordAtCursor() const;

  QCompleter* m_completer;
  QStringListModel* m_model;
  const QMap<QString, QString>* m_completions;
};

#endif

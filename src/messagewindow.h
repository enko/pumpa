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

#ifndef MESSAGE_WINDOW_H
#define MESSAGE_WINDOW_H

#include <QDialog>
#include <QShowEvent>
#include <QVBoxLayout>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

#include "messageedit.h"
#include "qactivitystreams.h"

//------------------------------------------------------------------------------

class MessageWindow : public QDialog {
  Q_OBJECT

public:
  MessageWindow(QASObject* obj=NULL, QWidget* parent=0);

  virtual void accept();

protected:
  void showEvent(QShowEvent*);

signals:
  void sendMessage(QString);
  void sendReply(QASObject*, QString);

private:
  QVBoxLayout* layout;

  QLabel* infoLabel;
  QHBoxLayout* infoLayout;

  MessageEdit* textEdit;
  QHBoxLayout* buttonLayout;

  QPushButton* cancelButton;
  QPushButton* sendButton;

  QASObject* m_obj;
};

#endif

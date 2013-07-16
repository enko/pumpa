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
#include <QComboBox>
#include <QFormLayout>

#include "messageedit.h"
#include "qactivitystreams.h"
#include "pumpasettings.h"

//------------------------------------------------------------------------------

class MessageWindow : public QDialog {
  Q_OBJECT

public:
  MessageWindow(const PumpaSettings* s, QWidget* parent=0);
  virtual void accept();

  void newMessage(QASObject* obj);

protected:
  virtual void showEvent(QShowEvent*);

signals:
  void sendMessage(QString, int, int);
  void sendReply(QASObject*, QString);

private:
  QVBoxLayout* layout;

  QLabel* m_infoLabel;
  QLabel* m_markupLabel;
  QHBoxLayout* infoLayout;

  QComboBox* m_toComboBox;
  QComboBox* m_ccComboBox;
  QFormLayout* m_addressLayout;

  MessageEdit* textEdit;
  QHBoxLayout* buttonLayout;

  QPushButton* cancelButton;
  QPushButton* sendButton;

  QASObject* m_obj;
  const PumpaSettings* m_s;
};

#endif

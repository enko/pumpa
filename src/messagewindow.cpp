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

#include "messagewindow.h"

//------------------------------------------------------------------------------

MessageWindow::MessageWindow(QASObject* obj, QWidget* parent) :
  QDialog(parent), m_obj(obj)
{
  infoLabel = new QLabel(this);
  infoLabel->setText(QString("Post a ") + (obj == NULL ? "note" : "reply"));

  infoLayout = new QHBoxLayout;
  infoLayout->addWidget(infoLabel);

  textEdit = new MessageEdit(this);

  connect(textEdit, SIGNAL(ready()), this, SLOT(accept()));

  layout = new QVBoxLayout;
  layout->addLayout(infoLayout);
  layout->addWidget(textEdit);

  cancelButton = new QPushButton("Cancel");
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  
  sendButton = new QPushButton("Send message");
  connect(sendButton, SIGNAL(clicked()), this, SLOT(accept()));
  sendButton->setDefault(true);

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(cancelButton);
  buttonLayout->addWidget(sendButton);
  layout->addLayout(buttonLayout);
  
  setLayout(layout);

  textEdit->setFocus(Qt::OtherFocusReason);

  QTextCursor cursor = textEdit->textCursor();
  cursor.movePosition(QTextCursor::End);
  textEdit->setTextCursor(cursor);
}

//------------------------------------------------------------------------------

void MessageWindow::showEvent(QShowEvent*) {
  textEdit->setFocus(Qt::OtherFocusReason);
  activateWindow();
}

//------------------------------------------------------------------------------

void MessageWindow::accept() {
  QString msg = textEdit->toPlainText();

  if (m_obj == NULL)
    emit sendMessage(msg);
  else
    emit sendReply(m_obj, msg);

  QDialog::accept();
}

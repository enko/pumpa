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
#include "pumpa_defines.h"

//------------------------------------------------------------------------------

MessageWindow::MessageWindow(const PumpaSettings* s,
                             QWidget* parent) :
  QDialog(parent),
  m_toComboBox(NULL),
  m_ccComboBox(NULL),
  m_addressLayout(NULL),
  m_obj(NULL),
  m_s(s)
{
  setMinimumSize(QSize(400,400));

  m_infoLabel = new QLabel(this);

  m_markupLabel = new QLabel(this);
  m_markupLabel->setText(QString("<a href=\"%2\">[markup]</a>").
                       arg(MARKUP_DOC_URL));
  m_markupLabel->setOpenExternalLinks(true);
  m_markupLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                       Qt::LinksAccessibleByMouse);

  infoLayout = new QHBoxLayout;
  infoLayout->addWidget(m_infoLabel);
  infoLayout->addStretch();
  infoLayout->addWidget(m_markupLabel);

  QStringList addressItems;
  addressItems << ""
               << "Public"
               << "Followers";
  
  m_toComboBox = new QComboBox(this);
  m_toComboBox->addItems(addressItems);
  
  m_ccComboBox = new QComboBox(this);
  m_ccComboBox->addItems(addressItems);
  
  m_addressLayout = new QFormLayout;
  m_addressLayout->addRow("To:", m_toComboBox);
  m_addressLayout->addRow("Cc:", m_ccComboBox);

  m_addPictureButton = new TextToolButton("Add picture", this);

  textEdit = new MessageEdit(this);

  connect(textEdit, SIGNAL(ready()), this, SLOT(accept()));

  layout = new QVBoxLayout;
  layout->addLayout(infoLayout);
  layout->addLayout(m_addressLayout);
  layout->addWidget(m_addPictureButton);
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

void MessageWindow::newMessage(QASObject* obj) {
  bool isReply = (obj != NULL);
  m_obj = obj;
  m_infoLabel->setText(QString("Post a %1").
                       arg(obj == NULL ? "note" : "reply"));
  m_toComboBox->setCurrentIndex(m_s->defaultToAddress());
  m_ccComboBox->setCurrentIndex(2);

  textEdit->clear();

  m_toComboBox->setVisible(!isReply);
  m_addressLayout->labelForField(m_toComboBox)->setVisible(!isReply);
  m_ccComboBox->setVisible(!isReply);
  m_addressLayout->labelForField(m_ccComboBox)->setVisible(!isReply);

  m_addPictureButton->setVisible(!isReply);
}

//------------------------------------------------------------------------------

void MessageWindow::showEvent(QShowEvent*) {
  textEdit->setFocus(Qt::OtherFocusReason);
  textEdit->selectAll();
  activateWindow();
}

//------------------------------------------------------------------------------

void MessageWindow::accept() {
  QString msg = textEdit->toPlainText();

  if (m_obj == NULL) {
    int to = m_toComboBox->currentIndex();
    int cc = m_ccComboBox->currentIndex();
    emit sendMessage(msg, to, cc);
  } else {
    emit sendReply(m_obj, msg);
  }

  QDialog::accept();
}

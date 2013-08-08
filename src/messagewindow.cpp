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
#include "util.h"

#include <QFileDialog>
#include <QMessageBox>

//------------------------------------------------------------------------------

const int max_picture_size = 160;

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
  m_markupLabel->setText(QString("<a href=\"%2\">" + tr("[markup]") + "</a>").
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
               << tr("Public")
               << tr("Followers");
  
  m_toComboBox = new QComboBox(this);
  m_toComboBox->addItems(addressItems);
  
  m_ccComboBox = new QComboBox(this);
  m_ccComboBox->addItems(addressItems);
  
  m_addressLayout = new QFormLayout;
  m_addressLayout->addRow(tr("To:"), m_toComboBox);
  m_addressLayout->addRow(tr("Cc:"), m_ccComboBox);

  m_addPictureButton = new TextToolButton(this);
  connect(m_addPictureButton, SIGNAL(clicked()), this, SLOT(onAddPicture()));

  m_removePictureButton = new TextToolButton(tr("&Remove picture"), this);
  connect(m_removePictureButton, SIGNAL(clicked()),
          this, SLOT(onRemovePicture()));

  m_pictureButtonLayout = new QHBoxLayout;
  m_pictureButtonLayout->addWidget(m_addPictureButton, 0, Qt::AlignTop);
  m_pictureButtonLayout->addWidget(m_removePictureButton, 0, Qt::AlignTop);
  m_pictureButtonLayout->addStretch();

  m_pictureLabel = new QLabel(this);
  m_pictureLabel->setScaledContents(true);
  m_pictureLabel->setMaximumSize(max_picture_size, max_picture_size);
  m_pictureLabel->setFocusPolicy(Qt::NoFocus);
  m_pictureLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);

  m_pictureTitle = new QLineEdit(this);
  m_pictureTitle->setPlaceholderText(tr("Picture title (optional)"));

  m_previewLabel = new RichTextLabel(this);
  m_previewLabel->setLineWidth(1);
  m_previewLabel->setFrameStyle(QFrame::Box);
  m_previewLabel->hide();

  m_textEdit = new MessageEdit(this);
  connect(m_textEdit, SIGNAL(ready()), this, SLOT(accept()));
  connect(m_textEdit, SIGNAL(textChanged()), this, SLOT(updatePreview()));

  layout = new QVBoxLayout;
  layout->addLayout(infoLayout);
  layout->addLayout(m_addressLayout);
  layout->addLayout(m_pictureButtonLayout);
  layout->addWidget(m_pictureLabel, 0, Qt::AlignHCenter);
  layout->addWidget(m_pictureTitle);
  layout->addWidget(m_textEdit);
  layout->addWidget(m_previewLabel);

  m_cancelButton = new QPushButton(tr("Cancel"));
  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

  m_previewButton = new QPushButton(tr("Preview"));
  connect(m_previewButton, SIGNAL(clicked()), this, SLOT(togglePreview()));
  
  m_sendButton = new QPushButton(tr("Send message"));
  connect(m_sendButton, SIGNAL(clicked()), this, SLOT(accept()));
  m_sendButton->setDefault(true);

  buttonLayout = new QHBoxLayout;
  buttonLayout->addWidget(m_cancelButton);
  buttonLayout->addWidget(m_previewButton);
  buttonLayout->addWidget(m_sendButton);
  layout->addLayout(buttonLayout);
  
  setLayout(layout);

  m_textEdit->setFocus(Qt::OtherFocusReason);

  QTextCursor cursor = m_textEdit->textCursor();
  cursor.movePosition(QTextCursor::End);
  m_textEdit->setTextCursor(cursor);
}

//------------------------------------------------------------------------------

void MessageWindow::newMessage(QASObject* obj) {
  bool isReply = (obj != NULL);
  m_obj = obj;

  m_infoLabel->setText(obj == NULL ? tr("Post a note") : tr("Post a reply"));
  m_toComboBox->setCurrentIndex(m_s->defaultToAddress());
  m_ccComboBox->setCurrentIndex(m_s->defaultCcAddress());

  m_toComboBox->setVisible(!isReply);
  m_addressLayout->labelForField(m_toComboBox)->setVisible(!isReply);
  m_ccComboBox->setVisible(!isReply);
  m_addressLayout->labelForField(m_ccComboBox)->setVisible(!isReply);

  updateAddPicture();
}

//------------------------------------------------------------------------------

void MessageWindow::clear() {
  m_imageFileName = "";
  m_textEdit->clear();
  m_pictureTitle->clear();
  m_previewLabel->clear();
}

//------------------------------------------------------------------------------

void MessageWindow::showEvent(QShowEvent*) {
  m_textEdit->setFocus(Qt::OtherFocusReason);
  m_textEdit->selectAll();
  activateWindow();
}

//------------------------------------------------------------------------------

void MessageWindow::accept() {
  m_textEdit->hideCompletion();
  QString msg = m_textEdit->toPlainText();

  if (m_obj == NULL) {
    int to = m_toComboBox->currentIndex();
    int cc = m_ccComboBox->currentIndex();

    if (m_imageFileName.isEmpty()) {
      emit sendMessage(msg, to, cc);
    } else {
      QString title = m_pictureTitle->text();
      emit sendImage(msg, title, m_imageFileName, to, cc);
    }
  } else {
    emit sendReply(m_obj, msg);
  }

  QDialog::accept();
}

//------------------------------------------------------------------------------

void MessageWindow::onAddPicture() {
  QString fileName =
    QFileDialog::getOpenFileName(this, tr("Select Image"), "",
                                 tr("Image files (*.png *.jpg *.jpeg *.gif)"
                                    ";;All files (*.*)"));

  if (!fileName.isEmpty()) {
    m_imageFileName = fileName;
    updateAddPicture();
  }
}

//------------------------------------------------------------------------------

void MessageWindow::onRemovePicture() {
  m_imageFileName = "";
  updateAddPicture();
}

//------------------------------------------------------------------------------

void MessageWindow::updateAddPicture() {
  if (m_obj != NULL) { // if reply just hide everything
    m_addPictureButton->setVisible(false);
    m_removePictureButton->setVisible(false);
    m_pictureLabel->setVisible(false);
    m_pictureTitle->setVisible(false);
    m_removePictureButton->setVisible(false);
    return;
  }

  QPixmap p;
  if (!m_imageFileName.isEmpty()) {
    p.load(m_imageFileName);
    if (p.isNull()) {
      QMessageBox::critical(this, tr("Sorry!"),
                            tr("That file didn't appear to be an image."));
      m_imageFileName = "";
    }
  }

  m_addPictureButton->setVisible(true);
  if (m_imageFileName.isEmpty()) {
    m_addPictureButton->setText(tr("&Add picture"));
    m_removePictureButton->setVisible(false);
    m_pictureLabel->setVisible(false);
    m_pictureTitle->setVisible(false);
  } else {
    m_pictureLabel->setPixmap(p);
    m_addPictureButton->setText(tr("&Change picture"));
    m_removePictureButton->setVisible(true);
    m_pictureLabel->setVisible(true);
    m_pictureTitle->setVisible(true);
  }
}

//------------------------------------------------------------------------------

void MessageWindow::updatePreview() {
  if (m_previewLabel->isVisible()) 
    m_previewLabel->setText(addTextMarkup(m_textEdit->toPlainText()));
}

//------------------------------------------------------------------------------

void MessageWindow::togglePreview() {
  m_previewLabel->setVisible(!m_previewLabel->isVisible());
  updatePreview();
}

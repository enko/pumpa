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

#include "objectwidget.h"

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QASObject* obj, QWidget* parent) :
  QFrame(parent), m_infoLabel(NULL), m_object(obj)
{
  m_layout = new QVBoxLayout(this);
  m_textLabel = new RichTextLabel(this);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  
  if (obj->type() == "image") {
    m_imageLabel = new QLabel(this);
    m_imageLabel->setMaximumSize(320, 320);
    m_imageLabel->setFocusPolicy(Qt::NoFocus);
    m_imageUrl = obj->imageUrl();
    updateImage();

    m_layout->addWidget(m_imageLabel);
  }

  m_layout->addWidget(m_textLabel);

  if (obj->type() == "comment") {
    m_infoLabel = new RichTextLabel(this);
    connect(m_infoLabel, SIGNAL(linkHovered(const QString&)),
            this, SIGNAL(linkHovered(const QString&)));

    m_layout->addWidget(m_infoLabel);
  }
  
  setLayout(m_layout);
}

//------------------------------------------------------------------------------

void ObjectWidget::setText(QString text) {
  m_textLabel->setText(text);
}

//------------------------------------------------------------------------------

void ObjectWidget::setInfo(QString text) {
  if (m_infoLabel == NULL) {
    qDebug() << "[WARNING] Trying to set info text to a non-comment object.";
    return;
  }
  m_infoLabel->setText(text);
}

//------------------------------------------------------------------------------

void ObjectWidget::fileReady(const QString& fn) {
  updateImage(fn);
}

//------------------------------------------------------------------------------

// FIXME this is duplicated in ActorWidget -> should be made more
// general and reused.
void ObjectWidget::updateImage(const QString& fileName) {
  static QString defaultImage = ":/images/broken_image.png";
  QString fn = fileName;

  if (fn.isEmpty()) {
    FileDownloader* fd = FileDownloader::get(m_imageUrl);

    if (fd->ready()) {
      fn = fd->fileName();
      fd->deleteLater();
    } else {
      connect(fd, SIGNAL(fileReady(const QString&)),
              this, SLOT(fileReady(const QString&)));
      fd->download();
    }
  }

  if (fn.isEmpty())
    fn = defaultImage;
  if (fn != m_localFile) {
    m_localFile = fn;
    QPixmap pix(m_localFile);
    if (pix.isNull()) {
      m_localFile = defaultImage;
      pix.load(m_localFile);
    }
    m_imageLabel->setPixmap(pix);
  }
}    

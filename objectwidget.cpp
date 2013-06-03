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
  QFrame(parent), m_object(obj)
{
  // useful for debugging layouts and margins
  // setLineWidth(1);
  // setFrameStyle(QFrame::Box);

  layout = new QVBoxLayout(this);
  textLabel = new RichTextLabel(this);

  if (obj->type() == "image") {
    imageLabel = new QLabel;
    imageLabel->setMaximumSize(320, 320);
    imageLabel->setFocusPolicy(Qt::NoFocus);
    m_imageUrl = obj->imageUrl();
    updateImage();

    layout->addWidget(imageLabel);
  }
  
  layout->addWidget(textLabel);

  setLayout(layout);
}

//------------------------------------------------------------------------------

void ObjectWidget::setText(QString text) {
  textLabel->setText(text);
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
    imageLabel->setPixmap(pix);
  }
}    

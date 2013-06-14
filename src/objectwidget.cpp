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

QString actorNames(QASActorList* alist) {
  QString text;
  for (size_t i=0; i<alist->size(); i++) {
    QASActor* a = alist->at(i);
    text += QString("<a href=\"%1\">%2</a>")
      .arg(a->url())
      .arg(a->displayNameOrYou());
    if (i != alist->size()-1)
      text += ", ";
  }
  return text;
}

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QASObject* obj, QWidget* parent) :
  QFrame(parent),
  m_infoLabel(NULL),
  m_likesLabel(NULL),
  m_sharesLabel(NULL),
  m_titleLabel(NULL),
  m_object(obj)
{
  m_layout = new QVBoxLayout(this);

  if (!obj->displayName().isEmpty()) {
    m_titleLabel = new QLabel("<b>" + obj->displayName() + "</b>");
    m_layout->addWidget(m_titleLabel);
  }

  if (obj->type() == "image") {
    m_imageLabel = new QLabel(this);
    m_imageLabel->setMaximumSize(320, 320);
    m_imageLabel->setFocusPolicy(Qt::NoFocus);
    m_imageUrl = obj->imageUrl();
    updateImage();

    m_layout->addWidget(m_imageLabel);
  }

  m_textLabel = new RichTextLabel(this);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  m_layout->addWidget(m_textLabel);

  if (obj->type() == "comment") {
    m_infoLabel = new RichTextLabel(this);
    connect(m_infoLabel, SIGNAL(linkHovered(const QString&)),
            this, SIGNAL(linkHovered(const QString&)));

    m_layout->addWidget(m_infoLabel);
  }

  updateLikes();

  updateShares();
  
  setLayout(m_layout);

  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));
}

//------------------------------------------------------------------------------

void ObjectWidget::onChanged() {
  updateLikes();
  updateShares();
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

//------------------------------------------------------------------------------

void ObjectWidget::updateLikes() {
  size_t nl = m_object->numLikes();

  if (nl > m_object->likes()->size()) {
    m_object->likes()->refresh();
  }
  
  if (!nl) {
    if (m_likesLabel != NULL) {
      m_layout->removeWidget(m_likesLabel);
      delete m_likesLabel;
      m_likesLabel = NULL;
    }
    return;
  }

  QASActorList* likes = m_object->likes();

  QString text;
  if (m_likesLabel == NULL) {
    m_likesLabel = new RichTextLabel(this);
    connect(m_likesLabel, SIGNAL(linkHovered(const QString&)),
            this,  SIGNAL(linkHovered(const QString&)));
    m_layout->addWidget(m_likesLabel);
  }

  if (likes->size()) {
    text = actorNames(likes);
    int others = nl-likes->size();
    if (others)
      text += QString(" and %1 other %2").arg(others).
        arg(others > 1 ? "persons" : "person");
    text += (nl==1 && !likes->onlyYou()) ? " likes" : " like";
    text += " this.";
  } else {
    if (nl == 1)
      text = "1 person likes this.";
    else
      text = QString("%1 persons like this.").arg(nl);
  }
  
  m_likesLabel->setText(text);
}

//------------------------------------------------------------------------------

void ObjectWidget::updateShares() {
  size_t ns = m_object->numShares();
  if (!ns) {
    if (m_sharesLabel != NULL) {
      m_layout->removeWidget(m_sharesLabel);
      delete m_sharesLabel;
      m_sharesLabel = NULL;
    }
    return;
  }

  if (m_sharesLabel == NULL) {
    m_sharesLabel = new RichTextLabel(this);
    connect(m_sharesLabel, SIGNAL(linkHovered(const QString&)),
            this,  SIGNAL(linkHovered(const QString&)));
    m_layout->addWidget(m_sharesLabel);
  }

  QString text;
  if (m_object->shares()->size()) {
    text = actorNames(m_object->shares());
    int others = ns-m_object->shares()->size();
    if (others)
      text += QString(" and %1 other %2").arg(others).
        arg(others > 1 ? "persons" : "person");
    text += " shared this.";
  } else {
    if (ns == 1)
      text = "1 person shared this.";
    else
      text = QString("%1 persons shared this.").arg(ns);
  }
  
  m_sharesLabel->setText(text);
}

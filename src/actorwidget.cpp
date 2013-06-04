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

#include "actorwidget.h"
#include "filedownloader.h"

//------------------------------------------------------------------------------

ActorWidget::ActorWidget(QASActor* a, QWidget* parent, bool small) :
  QLabel(parent), m_actor(a)
{
  int max_size = small ? 32 : 64;

  setScaledContents(true);
  setMaximumSize(max_size, max_size);
  setFocusPolicy(Qt::NoFocus);

  onImageChanged();
  // connect(m_actor, SIGNAL(imageChanged()), this, SLOT(onImageChanged()));
}

//------------------------------------------------------------------------------

void ActorWidget::onImageChanged() {
  m_url = m_actor->imageUrl();
  updatePixmap();
}

//------------------------------------------------------------------------------

void ActorWidget::fileReady(const QString& fn) {
  updatePixmap(fn);
}

//------------------------------------------------------------------------------

void ActorWidget::updatePixmap(const QString& fileName) {
  static QString defaultImage = ":/images/default.png";
  QString fn = fileName;

  // if (fn.isEmpty()) {
  //   FileDownloader* fd = new FileDownloader(this);
  //   connect(fd, SIGNAL(fileReady(const QString&)),
  //           this, SLOT(fileReady(const QString&)));
  //   fn = fd->getFile(url);
  // }

  // if (fn.isEmpty()) {
  //   FileDownloader* fd = NULL;
  //   fn = FileDownloader::getFile(url, fd);
  //   if (fd)
  //     connect(fd, SIGNAL(fileReady(const QString&)),
  //             this, SLOT(fileReady(const QString&)));
  // }

  if (fn.isEmpty()) {
    FileDownloader* fd = FileDownloader::get(m_url);

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
    if (pix.isNull())
      pix.load(m_localFile,"JPEG");
    if (pix.isNull())
      pix.load(m_localFile,"PNG");
    if (pix.isNull()) {
      m_localFile = defaultImage;
      pix.load(m_localFile);
    }
    setPixmap(pix);
  }
}    

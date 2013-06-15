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
}

//------------------------------------------------------------------------------

void ActorWidget::onImageChanged() {
  m_url = m_actor->imageUrl();
  updatePixmap();
}

//------------------------------------------------------------------------------

void ActorWidget::updatePixmap() {
  FileDownloader* fd = FileDownloader::get(m_url, true);
  connect(fd, SIGNAL(fileReady()), this, SLOT(updatePixmap()),
          Qt::UniqueConnection);
  setPixmap(fd->pixmap(":/images/default.png"));
}

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

#ifndef _ACTORWIDGET_H_
#define _ACTORWIDGET_H_

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>

#include "qactivitystreams.h"

//------------------------------------------------------------------------------

class ActorWidget : public QLabel {
  Q_OBJECT
public:
  ActorWidget(QASActor* a, QWidget* parent = 0);

public slots:
  virtual void fileReady(const QString& fn);
  void onImageChanged();

private:
  void updatePixmap(const QString& fileName="");

private:
  QASActor* m_actor;
  QString m_url;
  QString m_localFile;
};

#endif /* _ACTORWIDGET_H_ */

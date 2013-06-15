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

#ifndef _OBJECTWIDGET_H_
#define _OBJECTWIDGET_H_

#include <QFrame>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

#include "qactivitystreams.h"
#include "filedownloader.h"
#include "richtextlabel.h"

//------------------------------------------------------------------------------

class ObjectWidget : public QFrame {
  Q_OBJECT

public:
  ObjectWidget(QASObject* obj, QWidget* parent = 0);

  QASObject* object() const { return m_object; }

  void setText(QString text);
  void setInfo(QString text);

signals:
  void linkHovered(const QString&);

private slots:
  void onChanged();
  void updateImage();

private:
  void updateLikes();
  void updateShares();

  QString m_imageUrl;
  QString m_localFile;

  RichTextLabel* m_textLabel;
  QLabel* m_imageLabel;
  QVBoxLayout* m_layout;
  RichTextLabel* m_infoLabel;
  RichTextLabel* m_likesLabel;
  RichTextLabel* m_sharesLabel;
  QLabel* m_titleLabel;

  QASObject* m_object;
};

#endif /* _OBJECTWIDGET_H_ */

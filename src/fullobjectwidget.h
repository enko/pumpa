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

#ifndef _FULLOBJECTWIDGET_H_
#define _FULLOBJECTWIDGET_H_

#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>

#include "qactivitystreams.h"
#include "richtextlabel.h"
#include "texttoolbutton.h"
#include "imagelabel.h"
#include "objectwidgetwithsignals.h"

//------------------------------------------------------------------------------

class FullObjectWidget : public ObjectWidgetWithSignals {
  Q_OBJECT

public:
  FullObjectWidget(QASObject* obj, QWidget* parent = 0, bool childWidget=false);

  QASObject* object() const { return m_object; }

  virtual void refreshTimeLabels() { updateInfoText(); }

private slots:
  void onChanged();
  void updateImage();
  void imageClicked();
  void onHasMoreClicked();

  void favourite();
  void repeat();
  void reply();
  void onFollow();
  void onFollowAuthor();
  void updateFollowAuthorButton(bool wait = false);
  void onDeleteClicked();

private:
  bool hasValidIrtObject();
  void setText(QString text);
  void updateInfoText();

  void updateLikes();
  void updateShares();

  QString recipientsToString(QASObjectList* rec);
  QString processText(QString old_text, bool getImages=false);

  void addHasMoreButton(QASObjectList* ol, int li);
  void updateFavourButton(bool wait = false);
  void updateShareButton(bool wait = false);
  void updateFollowButton(bool wait = false);

  bool isFollowable(QASObject* obj) const;

  void addObjectList(QASObjectList* ol);

  QString m_imageUrl;
  QString m_localFile;

  RichTextLabel* m_textLabel;
  ImageLabel* m_imageLabel;

  RichTextLabel* m_infoLabel;
  RichTextLabel* m_likesLabel;
  RichTextLabel* m_sharesLabel;
  QLabel* m_titleLabel;
  QPushButton* m_hasMoreButton;

  TextToolButton* m_favourButton;
  TextToolButton* m_shareButton;
  TextToolButton* m_commentButton;
  TextToolButton* m_followButton;
  TextToolButton* m_followAuthorButton;
  TextToolButton* m_deleteButton;

  QVBoxLayout* m_contentLayout;
  QHBoxLayout* m_buttonLayout;
  QVBoxLayout* m_commentsLayout;

  QASObject* m_object;
  QASActor* m_actor;
  QASActor* m_author;

  QList<QASObject*> m_repliesList;
  QSet<QString> m_repliesMap;

  bool m_childWidget;
};

#endif /* _FULLOBJECTWIDGET_H_ */

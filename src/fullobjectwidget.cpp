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

#include "fullobjectwidget.h"
#include "pumpa_defines.h"
#include "util.h"
#include "shortobjectwidget.h"

#include <QDesktopServices>
#include <QMessageBox>

//------------------------------------------------------------------------------

QSet<QString> s_allowedTags;

//------------------------------------------------------------------------------

FullObjectWidget::FullObjectWidget(QASObject* obj, QWidget* parent,
                                   bool childWidget) :
  ObjectWidgetWithSignals(parent),
  m_infoLabel(NULL),
  m_likesLabel(NULL),
  m_sharesLabel(NULL),
  m_titleLabel(NULL),
  m_hasMoreButton(NULL),
  m_favourButton(NULL),
  m_shareButton(NULL),
  m_commentButton(NULL),
  m_followButton(NULL),
  m_followAuthorButton(NULL),
  m_deleteButton(NULL),
  m_object(NULL),
  m_actor(NULL),
  m_author(NULL),
  m_childWidget(childWidget)
{
#ifdef DEBUG_WIDGETS
  qDebug() << "Creating FullObjectWidget";
#endif
  QVBoxLayout* rightLayout = new QVBoxLayout;
  rightLayout->setContentsMargins(0, 0, 0, 0);

  m_contentLayout = new QVBoxLayout;
  m_contentLayout->setContentsMargins(0, 0, 0, 0);

  m_titleLabel = new QLabel(this); 
  m_contentLayout->addWidget(m_titleLabel);

  m_imageLabel = new ImageLabel(this);
  connect(m_imageLabel, SIGNAL(clicked()), this, SLOT(imageClicked()));
  m_imageLabel->setCursor(Qt::PointingHandCursor);
  m_contentLayout->addWidget(m_imageLabel);

  m_textLabel = new RichTextLabel(this);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  m_contentLayout->addWidget(m_textLabel, 0, Qt::AlignTop);

  m_infoLabel = new RichTextLabel(this);
  connect(m_infoLabel, SIGNAL(linkHovered(const QString&)),
          this, SIGNAL(linkHovered(const QString&)));
  m_contentLayout->addWidget(m_infoLabel, 0, Qt::AlignTop);

  m_buttonLayout = new QHBoxLayout;

  m_favourButton = new TextToolButton(this);
  connect(m_favourButton, SIGNAL(clicked()), this, SLOT(favourite()));
  m_buttonLayout->addWidget(m_favourButton, 0, Qt::AlignTop);

  m_followAuthorButton = new TextToolButton(this);
  connect(m_followAuthorButton, SIGNAL(clicked()),
          this, SLOT(onFollowAuthor()));
  m_buttonLayout->addWidget(m_followAuthorButton, 0, Qt::AlignTop);

  m_shareButton = new TextToolButton(this);
  connect(m_shareButton, SIGNAL(clicked()), this, SLOT(repeat()));
  m_buttonLayout->addWidget(m_shareButton, 0, Qt::AlignTop);

  m_deleteButton = new TextToolButton(tr("delete"), this);
  connect(m_deleteButton, SIGNAL(clicked()), this, SLOT(onDeleteClicked()));
  m_buttonLayout->addWidget(m_deleteButton, 0, Qt::AlignTop);

  m_commentButton = new TextToolButton(tr("comment"), this);
  connect(m_commentButton, SIGNAL(clicked()), this, SLOT(reply()));
  m_buttonLayout->addWidget(m_commentButton, 0, Qt::AlignTop);

  m_followButton = new TextToolButton(this);
  connect(m_followButton, SIGNAL(clicked()), this, SLOT(onFollow()));
  m_buttonLayout->addWidget(m_followButton, 0, Qt::AlignTop);

  m_buttonLayout->addStretch();

  m_commentsLayout = new QVBoxLayout;

  rightLayout->addLayout(m_contentLayout);
  rightLayout->addLayout(m_buttonLayout);
  rightLayout->addLayout(m_commentsLayout);

  // If this object is not an actor itself, show the author in the
  // avatar image.
  m_actorWidget = new ActorWidget(NULL, this);

  QHBoxLayout* acrossLayout = new QHBoxLayout;
  acrossLayout->setSpacing(10);
  acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  acrossLayout->addLayout(rightLayout);

  changeObject(obj);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
  
  setLayout(acrossLayout);
}

//------------------------------------------------------------------------------

FullObjectWidget::~FullObjectWidget() {
#ifdef DEBUG_WIDGETS
  qDebug() << "Deleting FullObjectWidget" << m_object->id();
#endif
}

//------------------------------------------------------------------------------

void FullObjectWidget::changeObject(QASAbstractObject* obj) {
  if (m_object != NULL) {
    disconnect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));
    if (m_author)
      disconnect(m_author, SIGNAL(changed()),
                 this, SLOT(updateFollowAuthorButton()));
    QASObjectList* ol = m_object->replies();
    if (ol)
      disconnect(ol, SIGNAL(changed()), this, SLOT(onChanged()));

    clearObjectList();
  }

  m_object = qobject_cast<QASObject*>(obj);
  if (!m_object)
    return;

  const QString objType = m_object->type();

  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));

  if (objType == "comment") {
    setLineWidth(1);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
  }

  if (!m_object->displayName().isEmpty()) {
    m_titleLabel->setText("<b>" + m_object->displayName() + "</b>");
    m_titleLabel->setVisible(true);
  } else {
    m_titleLabel->setVisible(false);
  }

  if (objType == "image") {
    m_imageLabel->setVisible(true);
    m_imageUrl = m_object->imageUrl();
    updateImage();
  } else {
    m_imageLabel->setVisible(false);
  }

  m_author = m_object->author();
  if (m_author)
    connect(m_author, SIGNAL(changed()),
            this, SLOT(updateFollowAuthorButton()));
  
  m_commentable = objType == "note" || objType == "comment" ||
    objType == "image";
  if (m_commentable) {
    m_favourButton->setVisible(true);
    m_followAuthorButton->setVisible(true);
    m_shareButton->setVisible(true);
    m_deleteButton->setVisible(m_author && m_author->isYou());
    m_commentButton->setVisible(true);
  } else {
    m_favourButton->setVisible(false);
    m_followAuthorButton->setVisible(false);
    m_shareButton->setVisible(false);
    m_deleteButton->setVisible(false);
    m_commentButton->setVisible(false);
  }

  m_followButton->setVisible(objType == "person");

  m_actor = m_object->asActor();

  QASActor* actorOrAuthor = m_actor ? m_actor : m_author;
  m_actorWidget->setActor(actorOrAuthor);

  onChanged();
}

//------------------------------------------------------------------------------

bool FullObjectWidget::hasValidIrtObject() {
  QASObject* irtObj = m_object->inReplyTo();
  return irtObj && !irtObj->id().isEmpty();
}

//------------------------------------------------------------------------------

void FullObjectWidget::onChanged() {
  if (!m_object)
    return;

  updateLikes();
  updateShares();

  updateFavourButton();
  updateShareButton();
  m_commentButton->setVisible(m_commentable && 
                              (m_object->type() != "comment" ||
                               hasValidIrtObject()));
  updateFollowButton();
  updateFollowAuthorButton();

  QString text = m_object->content();
  if (m_actor) {
    text = m_actor->summary();
    if (text.isEmpty())
      text = tr("[No description]");
  }

  setText(processText(text, true));

  updateInfoText();

  QASObjectList* ol = m_object->replies();
  if (ol) {
    connect(ol, SIGNAL(changed()), this, SLOT(onChanged()),
            Qt::UniqueConnection);
    if (ol->size() > 0)
      addObjectList(ol);
  }
}

//------------------------------------------------------------------------------

void FullObjectWidget::setText(QString text) {
  m_textLabel->setText(text);
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateInfoText() {
  if (m_infoLabel == NULL)
    return;

  QString infoStr;
  if (m_actor) {
    QString aid = m_actor->webFinger();
    infoStr = QString("<a href=\"%2\">%1</a>").arg(aid).arg(m_object->url());
    
    QString location = m_actor->location();
    if (!location.isEmpty())
      infoStr += " " + QString(tr("at %1")).arg(location) + " ";
  } else {
    infoStr = QString("<a href=\"%2\">%1</a>").
      arg(relativeFuzzyTime(m_object->published())).
      arg(m_object->url());

    QASActor* author = m_object->author();
    if (author)
      infoStr = QString("<a href=\"%2\">%1</a>").
        arg(author->displayName()).
        arg(author->url()) + " " + QString(tr("at %1")).arg(infoStr) + " ";
  }
  m_infoLabel->setText(infoStr);
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateFavourButton(bool wait) {
  if (!m_favourButton)
    return;

  QString text = m_object->liked() ? tr("unlike") : tr("like");
  if (wait)
    text = "...";
  m_favourButton->setText(text);
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateShareButton(bool /*wait*/) {
  if (!m_shareButton)
    return;

  m_shareButton->setText(tr("share"));
}

//------------------------------------------------------------------------------

bool FullObjectWidget::isFollowable(QASObject* obj) const {
  if (!obj)
    return false;

  QASActor* actor = obj->asActor();
  return obj->type() == "person" && actor && !actor->isYou();
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateFollowButton(bool /*wait*/) {
  if (!m_followButton)
    return;
  
  if (!isFollowable(m_object)) {
    m_followButton->setVisible(false);
    return;
  }

  m_followButton->setVisible(true);
  m_followButton->setText(m_actor->followed() ? tr("stop following") :
                          tr("follow"));
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateFollowAuthorButton(bool /*wait*/) {
  if (!m_followAuthorButton)
    return;
  
  if (!m_author || !isFollowable(m_author)) {
    m_followAuthorButton->setVisible(false);
    return;
  }

  m_followAuthorButton->setVisible(true);

  QString text = (m_author->followed() ? tr("stop following") : tr("follow"))
    + " ";
  text += m_author->preferredUsername();

  m_followAuthorButton->setText(text);
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateImage() {
  FileDownloader* fd = FileDownloader::get(m_imageUrl, true);
  connect(fd, SIGNAL(fileReady()), this, SLOT(updateImage()),
          Qt::UniqueConnection);
  m_imageLabel->setPixmap(fd->pixmap(":/images/broken_image.png"));
}    

//------------------------------------------------------------------------------

void FullObjectWidget::updateLikes() {
  size_t nl = m_object->numLikes();

  if (nl <= 0) {
    if (m_likesLabel != NULL) {
      m_contentLayout->removeWidget(m_likesLabel);
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
    m_contentLayout->addWidget(m_likesLabel);
  }

  QString nstr = likes->actorNames();
  if (likes->onlyYou())
    text = QString(" ") + tr("You like this.");
  else if (nl==1) 
    text = " " + QString(tr("%1 likes this.")).arg(nstr);
  else 
    text = " " + QString(tr("%1 like this.")).arg(nstr);
  m_likesLabel->setText(text);
}

//------------------------------------------------------------------------------

void FullObjectWidget::updateShares() {
  size_t ns = m_object->numShares();
  if (!ns) {
    if (m_sharesLabel != NULL) {
      m_contentLayout->removeWidget(m_sharesLabel);
      delete m_sharesLabel;
      m_sharesLabel = NULL;
    }
    return;
  }

  if (m_sharesLabel == NULL) {
    m_sharesLabel = new RichTextLabel(this);
    connect(m_sharesLabel, SIGNAL(linkHovered(const QString&)),
            this,  SIGNAL(linkHovered(const QString&)));
    m_contentLayout->addWidget(m_sharesLabel);
  }

  QString text;
  if (m_object->shares()->size()) {
    text = m_object->shares()->actorNames();
    int others = ns-m_object->shares()->size();
    if (others >= 1)
      text += QString(" ") + tr("and %Ln other person(s)", 0, others);
    text += QString(" ") + tr("shared this.");
  } else {
    if (ns >= 1)
      text = tr("%Ln person(s) shared this.", 0, ns);
  }
  
  m_sharesLabel->setText(text);
}

//------------------------------------------------------------------------------

void FullObjectWidget::imageClicked() {
  QString url = m_object->fullImageUrl();
  if (!url.isEmpty())
    QDesktopServices::openUrl(url);
}  

//------------------------------------------------------------------------------

void FullObjectWidget::addObjectList(QASObjectList* ol) {
  int li = 0; // index where to insert next widget in the layout
  int li_before = li;
  if (m_hasMoreButton != NULL)
    li++;

  /*
    For now we sort by time, or more accurately by whatever number the
    QASObject::sortInt() returns. Higher number is newer, goes further
    down the list.
  
    Comments' lists returned by the pump API are with newest at the
    top, so we start from the end, and can assume that the next one is
    always newer.
  */

  int i = 0; // index into m_repliesList
  for (size_t j=0; j<ol->size(); j++) {
    QASObject* replyObj = ol->at(ol->size()-j-1);
    QString replyId = replyObj->id();
    qint64 sortInt = replyObj->sortInt();

    while (i < m_repliesList.size() &&
           m_repliesList[i]->id() != replyId &&
           m_repliesList[i]->sortInt() < sortInt)
      i++;

    if (m_repliesMap.contains(replyId))
      continue;

    if (i < m_repliesList.size() && m_repliesList[i]->id() == replyId)
      continue;

    FullObjectWidget* ow = new FullObjectWidget(replyObj, this, true);
    ObjectWidgetWithSignals::connectSignals(ow, this);

    m_commentsLayout->insertWidget(li + i, ow);
    m_repliesList.insert(i, replyObj);
    m_repliesMap.insert(replyId);
  }

  if (ol->hasMore() && (qulonglong)m_repliesList.size() < ol->totalItems()) {
    addHasMoreButton(ol, li_before);
  } else if (m_hasMoreButton != NULL) {
    m_commentsLayout->removeWidget(m_hasMoreButton);
    delete m_hasMoreButton;
    m_hasMoreButton = NULL;
  }
}

//------------------------------------------------------------------------------

void FullObjectWidget::addHasMoreButton(QASObjectList* ol, int li) {
  QString buttonText = QString(tr("Show all %1 replies")).
    arg(ol->totalItems());
  if (m_hasMoreButton == NULL) {
    m_hasMoreButton = new QPushButton(this);
    m_hasMoreButton->setFocusPolicy(Qt::NoFocus);
    m_commentsLayout->insertWidget(li, m_hasMoreButton);
    connect(m_hasMoreButton, SIGNAL(clicked()), 
            this, SLOT(onHasMoreClicked()));
  }

  m_hasMoreButton->setText(buttonText);
}

//------------------------------------------------------------------------------

void FullObjectWidget::onHasMoreClicked() {
  m_hasMoreButton->setText("...");
  refreshObject(m_object->replies());
}

//------------------------------------------------------------------------------

void FullObjectWidget::favourite() {
  updateFavourButton(true);
  emit like(m_object);
}

//------------------------------------------------------------------------------

void FullObjectWidget::repeat() {
  updateShareButton(true);
  emit share(m_object);
}

//------------------------------------------------------------------------------

void FullObjectWidget::reply() {
  emit newReply(m_object);
}

//------------------------------------------------------------------------------

void FullObjectWidget::onDeleteClicked() {
  const int max_len = 40;
  QString excerpt = ShortObjectWidget::objectExcerpt(m_object).
    replace(QRegExp("\\s+"), " ");
  if (excerpt.count() > max_len) {
    excerpt.truncate(max_len-4);
    excerpt += " ...";
  }
  
  QString typeName = tr("post");
  QString tn = m_object->type();
  if (tn == "note")
    typeName = tr("note");
  else if (tn == "comment")
    typeName = tr("comment");
  else if (tn == "image")
    typeName = tr("image");

  QString msg = QString(tr("Are you sure you want to delete this %1?")).
    arg(typeName) + "\n\"" + excerpt.trimmed() + "\"";

  int ret = QMessageBox::warning(this, CLIENT_FANCY_NAME, msg,
                                 QMessageBox::Cancel | QMessageBox::Yes,
                                 QMessageBox::Cancel);
  if (ret == QMessageBox::Yes)
    emit deleteObject(m_object);
}

//------------------------------------------------------------------------------

void FullObjectWidget::onFollow() {
  updateFollowButton(true);
  if (isFollowable(m_actor))
    emit follow(m_actor->id(), !m_actor->followed());
}

//------------------------------------------------------------------------------

void FullObjectWidget::onFollowAuthor() {
  bool doFollow = !m_author->followed();
  if (!doFollow) {
    QString msg = tr("Are you sure you want to stop following") + " " +
      m_author->displayNameOrWebFinger();
    int ret = QMessageBox::warning(this, CLIENT_FANCY_NAME, msg,
                                   QMessageBox::Cancel | QMessageBox::Yes,
                                   QMessageBox::Cancel);
    if (ret != QMessageBox::Yes)
      return;
  }

  updateFollowAuthorButton(true);
  if (isFollowable(m_author))
    emit follow(m_author->id(), doFollow);
}

//------------------------------------------------------------------------------

QString FullObjectWidget::processText(QString old_text, bool getImages) {
  if (s_allowedTags.isEmpty()) {
    s_allowedTags 
      << "br" << "p" << "b" << "i" << "blockquote" << "div" << "abbr"
      << "code" << "h1" << "h2" << "h3" << "h4" << "h5"
      << "em" << "ol" << "li" << "ul" << "hr" << "strong" << "u";
    s_allowedTags << "pre";
    s_allowedTags << "a";
    s_allowedTags << "img";
  }
  
  QString text = old_text.trimmed();
  int pos;

  // Shorten links that are too long, this is OK, since you can still
  // click the link.
  QRegExp rxa("<a\\s[^>]*href=([^>\\s]+)[^>]*>([^<]*)</a>");
  pos = 0;
  while ((pos = rxa.indexIn(text, pos)) != -1) {
    int len = rxa.matchedLength();
    QString url = rxa.cap(1);
    QString linkText = rxa.cap(2);

    if ((linkText.startsWith("http://") || linkText.startsWith("https://")) &&
        linkText.length() > MAX_WORD_LENGTH) {
      linkText = linkText.left(MAX_WORD_LENGTH-3) + "...";
      QString newText = QString("<a href=%1>%2</a>").arg(url).arg(linkText);
      text.replace(pos, len, newText);
      pos += newText.length();
    } else
      pos += len;
  }

  // Detect single HTML tags for filtering.
  QRegExp rx("<(\\/?)([a-zA-Z0-9]+)([^>]*)>");
  pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString slash = rx.cap(1);
    QString tag = rx.cap(2).toLower();
    QString inside = rx.cap(3);

    if (tag == "img") { // Replace img's with placeholder
      QString imagePlaceholder = "[image]";

      if (getImages) {
        QRegExp rxi("\\s+src=\"?(" URL_REGEX ")\"?");
        int spos = rxi.indexIn(inside);
        if (spos != -1) {
          QString imgSrc = rxi.cap(1);
          // qDebug() << "[DEBUG] processText: img" << imgSrc;
          
          FileDownloader* fd = FileDownloader::get(imgSrc, true);
          connect(fd, SIGNAL(fileReady()), this, SLOT(onChanged()),
                  Qt::UniqueConnection);
          if (fd->ready())
            imagePlaceholder = 
              QString("<a href=\"%2\"><img border=\"0\" src=\"%1\" /></a>").
              arg(fd->fileName()).arg(imgSrc);
        }
      }
      text.replace(pos, len, imagePlaceholder);
      pos += imagePlaceholder.length();
      // qDebug() << "[DEBUG] processText: removing image";
    } else if (s_allowedTags.contains(tag)) {
      pos += len;
    } else { // drop all other HTML tags
      if (tag != "span") 
        qDebug() << "[DEBUG] processText: dropping unsupported tag" << tag;
      text.remove(pos, len);
    }
  }

  text.replace("< ", "&lt; ");

  // remove trailing <br>:s
  while (text.endsWith("<br>"))
    text.chop(4);

  return text;
}

//------------------------------------------------------------------------------

void FullObjectWidget::clearObjectList() {
  QLayoutItem* item;
  while ((item = m_commentsLayout->takeAt(0)) != 0) {
    if (dynamic_cast<QWidgetItem*>(item)) {
      QWidget* w = item->widget();

      FullObjectWidget* ow = qobject_cast<FullObjectWidget*>(w);
      if (ow)
        ObjectWidgetWithSignals::disconnectSignals(ow, this);

      delete w;
    }
    delete item;
  }
  m_repliesMap.clear();
  m_repliesList.clear();
  m_hasMoreButton = NULL;
}

//------------------------------------------------------------------------------

void FullObjectWidget::refreshTimeLabels() { 
  updateInfoText();

  for (int i=0; i<m_commentsLayout->count(); i++) {
    QLayoutItem* item = m_commentsLayout->itemAt(i);

    if (dynamic_cast<QWidgetItem*>(item)) {
      ObjectWidgetWithSignals* ow =
        qobject_cast<ObjectWidgetWithSignals*>(item->widget());
      if (ow)
        ow->refreshTimeLabels();
    }
  }
}

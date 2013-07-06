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
#include "actorwidget.h"
#include "pumpa_defines.h"
#include "util.h"

#include <QDesktopServices>

//------------------------------------------------------------------------------

QSet<QString> s_allowedTags;

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

QString splitLongWords(QString text) {
  QRegExp rx("(^|\\s)([^\\s<>\"]{40,})(\\s|$)");
  int pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString word = rx.cap(2);
    QString newText = rx.cap(1);

    int wpos = 0;
    while (true) {
      newText += word.mid(wpos, 5);
      wpos += 5;
      if (wpos >= word.length())
        break;
      else
        newText += "&shy;";
    }
    // qDebug() << "[DEBUG] splitLongWords:" << word << "=>" << newText;

    text.replace(pos, len, newText);
    pos += newText.count();
  }
  return text;
}

//------------------------------------------------------------------------------

QString ObjectWidget::processText(QString old_text, bool getImages) {
  if (s_allowedTags.isEmpty()) {
    s_allowedTags 
      << "br" << "p" << "b" << "i" << "blockquote" << "div"
      << "code" << "h1" << "h2" << "h3" << "h4" << "h5"
      << "em" << "ol" << "li" << "ul" << "strong";
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
    QString tag = rx.cap(2);
    QString inside = rx.cap(3);

    if (tag == "img") { // Replace img's with placeholder
      QString imagePlaceholder = "[image]";

      if (getImages) {
        QRegExp rxi("\\s+src=\"?(" URL_REGEX ")\"?");
        int spos = rxi.indexIn(inside);
        if (spos != -1) {
          QString imgSrc = rxi.cap(1);
          qDebug() << "[DEBUG] processText: img" << imgSrc;
          
          FileDownloader* fd = FileDownloader::get(imgSrc, true);
          connect(fd, SIGNAL(fileReady()), this, SLOT(onObjectChanged()),
                  Qt::UniqueConnection);
          if (fd->ready())
            imagePlaceholder = 
              QString("<img src=\"%1\" />").arg(fd->fileName());
        }
      }
      text.replace(pos, len, imagePlaceholder);
      pos += imagePlaceholder.length();
      // qDebug() << "[DEBUG] processText: removing image";
    } else if (s_allowedTags.contains(tag)) {
      pos += len;
    } else { // drop all other HTML tags
      qDebug() << "[DEBUG] processText: dropping unsupported tag" << tag;
      text.remove(pos, len);
    }
  }

  text.replace("< ", "&lt; ");

  // remove trailing <br>:s
  while (text.endsWith("<br>"))
    text.chop(4);

  // qDebug() << "processText:" << old_text;
  // qDebug() << "          ->" << text;
  // return splitLongWords(text);
  return text;
}

//------------------------------------------------------------------------------

QString relativeFuzzyTime(QDateTime sTime) {
  QString dateStr = sTime.toString("ddd d MMMM yyyy");

  int secs = sTime.secsTo(QDateTime::currentDateTime().toUTC());
  if (secs < 0)
    secs = 0;
  int mins = qRound((float)secs/60);
  int hours = qRound((float)secs/60/60);
    
  if (secs < 60) { 
    dateStr = QString("a few seconds ago");
  } else if (mins < 60) {
    dateStr = QString("%1 minute%2 ago").arg(mins).arg(mins==1?"":"s");
  } else if (hours < 24) {
    dateStr = QString("%1 hour%2 ago").arg(hours).arg(hours==1?"":"s");
  }
  return dateStr;
}

//------------------------------------------------------------------------------

ImageLabel::ImageLabel(QWidget* parent) : QLabel(parent) {
  setMaximumSize(320, 320);
  setFocusPolicy(Qt::NoFocus);
}

//------------------------------------------------------------------------------

void ImageLabel::mousePressEvent(QMouseEvent* event) {
  QLabel::mousePressEvent(event);
  emit clicked();
}

//------------------------------------------------------------------------------

ObjectWidget::ObjectWidget(QASObject* obj, QWidget* parent) :
  QFrame(parent),
  m_infoLabel(NULL),
  m_likesLabel(NULL),
  m_sharesLabel(NULL),
  m_titleLabel(NULL),
  m_hasMoreButton(NULL),
  m_object(obj)
{
  // setLineWidth(1);
  // setFrameStyle(QFrame::Box);
  m_layout = new QVBoxLayout(this);
  m_layout->setContentsMargins(0, 0, 0, 0);

  if (!obj->displayName().isEmpty()) {
    m_titleLabel = new QLabel("<b>" + obj->displayName() + "</b>");
    m_layout->addWidget(m_titleLabel);
  }

  if (obj->type() == "image") {
    m_imageLabel = new ImageLabel(this);
    if (!obj->fullImageUrl().isEmpty()) {
      connect(m_imageLabel, SIGNAL(clicked()), this, SLOT(imageClicked()));
      m_imageLabel->setCursor(Qt::PointingHandCursor);
    }
    m_imageUrl = obj->imageUrl();
    updateImage();

    m_layout->addWidget(m_imageLabel);
  }

  m_textLabel = new RichTextLabel(this);
  connect(m_textLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  m_layout->addWidget(m_textLabel, 0, Qt::AlignTop);

  // if (obj->type() == "comment") {
  //   m_infoLabel = new RichTextLabel(this);
  //   connect(m_infoLabel, SIGNAL(linkHovered(const QString&)),
  //           this, SIGNAL(linkHovered(const QString&)));

  //   m_layout->addWidget(m_infoLabel, 0, Qt::AlignTop);
  // }

  m_favourButton = new QToolButton();
  m_favourButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_favourButton->setFocusPolicy(Qt::NoFocus);
  updateFavourButton();
  connect(m_favourButton, SIGNAL(clicked()), this, SLOT(favourite()));

  m_shareButton = new QToolButton();
  m_shareButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_shareButton->setFocusPolicy(Qt::NoFocus);
  updateShareButton();
  connect(m_shareButton, SIGNAL(clicked()), this, SLOT(repeat()));

  m_commentButton = new QToolButton();
  m_commentButton->setText("comment");
  m_commentButton->setToolButtonStyle(Qt::ToolButtonTextOnly);
  m_commentButton->setFocusPolicy(Qt::NoFocus);
  connect(m_commentButton, SIGNAL(clicked()), this, SLOT(reply()));

  m_buttonLayout = new QHBoxLayout;
  m_buttonLayout->addWidget(m_favourButton, 0, Qt::AlignTop);
  m_buttonLayout->addWidget(m_shareButton, 0, Qt::AlignTop);
  m_buttonLayout->addWidget(m_commentButton, 0, Qt::AlignTop);
  m_buttonLayout->addStretch();

  m_layout->addLayout(m_buttonLayout);

  QASObjectList* ol = m_object->replies();
  if (ol) {
    connect(ol, SIGNAL(changed()), this, SLOT(onChanged()),
            Qt::UniqueConnection);
    
    if (m_object->numReplies() > 0)
      addObjectList(ol);
  }

  // updateLikes();

  // updateShares();
  onChanged();
  
  setLayout(m_layout);

  connect(m_object, SIGNAL(changed()), this, SLOT(onChanged()));
}

//------------------------------------------------------------------------------

void ObjectWidget::onChanged() {
  updateLikes();
  updateShares();
  // setText(processText(m_object()->content(), true));
  setText(m_object->content());

  if (m_object->numReplies() > 0) {
    QASObjectList* ol = m_object->replies();
    addObjectList(ol);
  }
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

void ObjectWidget::updateFavourButton(bool wait) {
  QString text = m_object->liked() ? "unlike" : "like";
  if (wait)
    text = "...";
  m_favourButton->setText(text);
}

//------------------------------------------------------------------------------

void ObjectWidget::updateShareButton(bool /*wait*/) {
  m_shareButton->setText("share");
}

//------------------------------------------------------------------------------

void ObjectWidget::updateImage() {
  FileDownloader* fd = FileDownloader::get(m_imageUrl, true);
  connect(fd, SIGNAL(fileReady()), this, SLOT(updateImage()),
          Qt::UniqueConnection);
  m_imageLabel->setPixmap(fd->pixmap(":/images/broken_image.png"));
}    

//------------------------------------------------------------------------------

void ObjectWidget::updateLikes() {
  size_t nl = m_object->numLikes();

  if (nl <= 0) {
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

  text = actorNames(likes);
  text += (nl==1 && !likes->onlyYou()) ? " likes" : " like";
  text += " this.";
  
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
    if (others > 0)
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

//------------------------------------------------------------------------------

void ObjectWidget::imageClicked() {
  QString url = m_object->fullImageUrl();
  if (!url.isEmpty())
    QDesktopServices::openUrl(url);
}  

//------------------------------------------------------------------------------

void ObjectWidget::addObjectList(QASObjectList* ol) {
  int li = 3; // index where to insert next widget in the layout
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

    QASActor* author = replyObj->author();

    ActorWidget* aw = new ActorWidget(author, this, true);
    ObjectWidget* ow = new ObjectWidget(replyObj, this);

    ow->setLineWidth(1);
    ow->setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
    
    QString content = processText(replyObj->content(), false);

    ow->setText(content);
    ow->setInfo(QString("<a href=\"%2\">%1</a> at <a href=\"%4\">%3</a>").
                arg(author->displayName()).
                arg(author->url()).
                arg(relativeFuzzyTime(replyObj->published())).
                arg(replyObj->url()));
    connect(ow, SIGNAL(linkHovered(const QString&)),
            this, SIGNAL(linkHovered(const QString&)));

    QHBoxLayout* replyLayout = new QHBoxLayout;
    replyLayout->setContentsMargins(0, 0, 0, 0);
    replyLayout->addWidget(aw, 0, Qt::AlignTop);
    replyLayout->addWidget(ow, 0, Qt::AlignTop);
    
    m_layout->insertLayout(li + i, replyLayout);
    m_repliesList.insert(i, replyObj);
    m_repliesMap.insert(replyId);
  }

  if (ol->hasMore() && (qulonglong)m_repliesList.size() < ol->totalItems()) {
    // qDebug() << "[DEBUG]:" << "addHasMoreButton:"
    //          << ol->hasMore() << (qulonglong)m_repliesList.size()
    //          << ol->totalItems();
    addHasMoreButton(ol, li_before);
  } else if (m_hasMoreButton != NULL) {
    m_layout->removeWidget(m_hasMoreButton);
    delete m_hasMoreButton;
    m_hasMoreButton = NULL;
  }
}

//------------------------------------------------------------------------------

void ObjectWidget::addHasMoreButton(QASObjectList* ol, int li) {
  QString buttonText = QString("Show all %1 replies").
    arg(ol->totalItems());
  if (m_hasMoreButton == NULL) {
    m_hasMoreButton = new QPushButton(this);
    m_hasMoreButton->setFocusPolicy(Qt::NoFocus);
    m_layout->insertWidget(li, m_hasMoreButton);
    connect(m_hasMoreButton, SIGNAL(clicked()), 
            this, SLOT(onHasMoreClicked()));
  }

  m_hasMoreButton->setText(buttonText);
}

//------------------------------------------------------------------------------

void ObjectWidget::onHasMoreClicked() {
  m_hasMoreButton->setText("...");
  m_object->replies()->refresh();
}

//------------------------------------------------------------------------------

void ObjectWidget::favourite() {
  updateFavourButton(true);
  emit like(m_object);
}

//------------------------------------------------------------------------------

void ObjectWidget::repeat() {
  updateShareButton(true);
  emit share(m_object);
}

//------------------------------------------------------------------------------

void ObjectWidget::reply() {
  emit newReply(m_object);
}

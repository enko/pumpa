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

#include "activitywidget.h"
#include "pumpa_defines.h"

#include <QDebug>
#include <QRegExp>

QSet<QString> s_allowedTags;

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
    qDebug() << "[DEBUG] splitLongWords:" << word << "=>" << newText;

    text.replace(pos, len, newText);
    pos += newText.count();
  }
  return text;
}

//------------------------------------------------------------------------------

QString processText(QString old_text) {
  if (s_allowedTags.isEmpty()) {
    s_allowedTags 
      << "br" << "p" << "b" << "i" << "blockquote" << "div"
      /*<< "pre" */<< "code" << "h1" << "h2" << "h3" << "h4" << "h5"
      << "em" << "ol" << "li" << "ul" << "strong";
    s_allowedTags << "a";
  }
  
  QString text = old_text.trimmed();

  //  QRegExp rxa("<a\\s[^>]*href=([^>\\s]*)[^>]*>([^<]*)</a>");
  QRegExp rxa("<a\\s[^>]*href=([^>\\s]+)[^>]*>([^<]*)</a>");
  int pos = 0;
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

  QRegExp rx("<(\\/?)([a-zA-Z0-9]+)[^>]*>");
  pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString tag = rx.cap(2);
    QString slash = rx.cap(1);

    if (tag == "img") {
      QString imagePlaceholder = "[image]";
      text.replace(pos, len, imagePlaceholder);
      pos += imagePlaceholder.length();
      qDebug() << "[DEBUG] processText: removing image";
    } else if (tag == "pre") {
      QString newTag = QString("<%1p><%1code>").arg(slash);
      text.replace(pos, len, newTag);
      pos += newTag.length();
    } else if (s_allowedTags.contains(tag)) {
      pos += len;
    } else {
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
  return splitLongWords(text);
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

ActivityWidget::ActivityWidget(QASActivity* a, QWidget* parent) :
  AbstractActivityWidget(a, parent),  m_hasMoreButton(NULL)
{
  QASObject* noteObj = a->object();

  m_infoLabel = new RichTextLabel(this);
  m_objectWidget = new ObjectWidget(noteObj, this);
  m_actorWidget = new ActorWidget(effectiveAuthor(), this);

  connect(m_infoLabel, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));
  connect(m_objectWidget, SIGNAL(linkHovered(const QString&)),
          this,  SIGNAL(linkHovered(const QString&)));

  updateText();

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

  m_rightLayout = new QVBoxLayout;
  m_rightLayout->setContentsMargins(0, 0, 0, 0);
  m_rightLayout->addWidget(m_infoLabel);
  m_rightLayout->addWidget(m_objectWidget);
  m_rightLayout->addLayout(m_buttonLayout);

  connect(noteObj, SIGNAL(changed()), this, SLOT(onObjectChanged()));

  QASObjectList* ol = noteObj->replies();
  if (ol) {
    connect(ol, SIGNAL(changed()), this, SLOT(onObjectChanged()));
    
    if (noteObj->numReplies() > 0)
      addObjectList(ol);
  }

  QHBoxLayout* m_acrossLayout = new QHBoxLayout;
  m_acrossLayout->setSpacing(10);
  m_acrossLayout->addWidget(m_actorWidget, 0, Qt::AlignTop);
  m_acrossLayout->addLayout(m_rightLayout, 0); 

  setLayout(m_acrossLayout);

  // connect(msg, SIGNAL(hasUpdated()), this, SLOT(onMessageHasUpdated()));
}

//------------------------------------------------------------------------------

QASActor* ActivityWidget::effectiveAuthor() {
  QASActor* author = m_activity->object()->author();
  return author ? author : m_activity->actor();
}

//------------------------------------------------------------------------------

// void ActivityWidget::mousePressEvent(QMouseEvent* e) {
//   emit clickedStatus(msg->getId());
//   QFrame::mousePressEvent(e);
// }

//------------------------------------------------------------------------------

void ActivityWidget::updateFavourButton(bool wait) {
  const QASObject* noteObj = m_activity->object();
  QString text = noteObj->liked() ? "unlike" : "like";
  if (wait)
    text = "...";
  m_favourButton->setText(text);
}

//------------------------------------------------------------------------------

void ActivityWidget::updateShareButton(bool /*wait*/) {
  m_shareButton->setText("share");
}

//------------------------------------------------------------------------------

void ActivityWidget::updateText() {
  const QASObject* noteObj = m_activity->object();
  const QASActor* author = effectiveAuthor();

  QString text = QString("<a href=\"%2\">%1</a> at <a href=\"%4\">%3</a>").
    arg(author->displayName()).
    arg(author->url()).
    arg(relativeFuzzyTime(noteObj->published())).
    arg(noteObj->url());

  QString generatorName = m_activity->generatorName();
  if (!generatorName.isEmpty())
    text += " via " + generatorName;

  QASObject* irtObj = noteObj->inReplyTo();
  if (irtObj && !irtObj->url().isEmpty())
    text += " in reply to a <a href=\"" + irtObj->url() + "\">note</a>";

  if (m_activity->verb() == "share")
    text += " (shared by " + m_activity->actor()->displayName() + ")";

  m_infoLabel->setText(text);

  m_objectWidget->setText(processText(noteObj->content()));
}

//------------------------------------------------------------------------------

void ActivityWidget::favourite() {
  updateFavourButton(true);
  emit like(m_activity->object());
}

//------------------------------------------------------------------------------

void ActivityWidget::repeat() {
  updateShareButton(true);
  emit share(m_activity->object());
}

//------------------------------------------------------------------------------

void ActivityWidget::reply() {
  emit newReply(m_activity->object());
}
 
//------------------------------------------------------------------------------

void ActivityWidget::onObjectChanged() {
  updateText();
  updateFavourButton();
  updateShareButton();

  const QASObject* noteObj = m_activity->object();
  if (noteObj->numReplies() > 0) {
    QASObjectList* ol = noteObj->replies();
    addObjectList(ol);
  }
}

//------------------------------------------------------------------------------

void ActivityWidget::addObjectList(QASObjectList* ol) {
  int li = 3; // index where to insert next widget in the layout

  if (ol->hasMore() && (qulonglong)m_repliesList.size() < ol->totalItems()) {
    addHasMoreButton(ol, li++);
  } else if (m_hasMoreButton != NULL) {
    m_rightLayout->removeWidget(m_hasMoreButton);
    delete m_hasMoreButton;
    m_hasMoreButton = NULL;
  }

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
    
    QString content = processText(replyObj->content());

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
    
    m_rightLayout->insertLayout(li + i, replyLayout);
    m_repliesList.insert(i, replyObj);
    m_repliesMap.insert(replyId);
  }
}

//------------------------------------------------------------------------------

void ActivityWidget::addHasMoreButton(QASObjectList* ol, int li) {
  QString buttonText = QString("Show all %1 replies").
    arg(ol->totalItems());
  if (m_hasMoreButton == NULL) {
    m_hasMoreButton = new QPushButton(this);
    m_hasMoreButton->setFocusPolicy(Qt::NoFocus);
    m_rightLayout->insertWidget(li, m_hasMoreButton);
    connect(m_hasMoreButton, SIGNAL(clicked()), 
            this, SLOT(onHasMoreClicked()));
  }

  m_hasMoreButton->setText(buttonText);
}

//------------------------------------------------------------------------------

void ActivityWidget::onHasMoreClicked() {
  m_hasMoreButton->setText("...");
  emit request(m_activity->object()->replies()->urlOrProxy(), QAS_REPLIES);
}

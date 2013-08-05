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

#include "util.h"

#include <QTextStream>
#include <QStringList>
#include <QRegExp>
#include <QObject>
#include <QDebug>
#include <QFile>

#ifdef DEBUG_MEMORY
#include <sys/resource.h>
#include <unistd.h>
#endif

#include "sundown/markdown.h"
#include "sundown/html.h"
#include "sundown/buffer.h"

//------------------------------------------------------------------------------

QString markDown(QString text) {
  struct sd_callbacks callbacks;
  struct html_renderopt options;
  sdhtml_renderer(&callbacks, &options, 0);

  struct sd_markdown* markdown = sd_markdown_new(0, 16, &callbacks, &options);

  struct buf* ob = bufnew(64);
  // QByteArray ba = text.toLocal8Bit();
  QByteArray ba = text.toUtf8();

  sd_markdown_render(ob, (const unsigned char*)ba.constData(), ba.size(),
                     markdown);
  sd_markdown_free(markdown);

  // QString ret = QString::fromLocal8Bit((char*)ob->data, ob->size);
  QString ret = QString::fromUtf8((char*)ob->data, ob->size);
  bufrelease(ob);

  return ret;
}

//------------------------------------------------------------------------------

QString siteUrlFixer(QString url) {
  if (!url.startsWith("http://") && !url.startsWith("https://"))
    url = "https://" + url;

  if (url.endsWith('/'))
    url.chop(1);

  return url;
}

//------------------------------------------------------------------------------

QString linkifyUrls(QString text) {
  QRegExp rx(QString("(\\s+)%1").arg(URL_REGEX_STRICT));

  int pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString before = rx.cap(1);
    QString url = rx.cap(2);
    QString newText = QString("%2<a href=\"%1\">%1</a>").arg(url).
      arg(before);

    text.replace(pos, len, newText);
    pos += newText.count();
  }
  return text;
}

//------------------------------------------------------------------------------

QString changePairedTags(QString text, 
                         QString begin, QString end,
                         QString newBegin, QString newEnd,
                         QString nogoItems) {
  QRegExp rx(QString(MD_PAIR_REGEX).arg(begin).arg(end).arg(nogoItems));
  int pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString newText = QString(newBegin + rx.cap(1) + newEnd);
    text.replace(pos, len, newText);
    pos += newText.count();
  }
  return text;
}

//------------------------------------------------------------------------------

QString siteUrlToAccountId(QString username, QString url) {
  if (url.startsWith("http://"))
    url.remove(0, 7);
  if (url.startsWith("https://"))
    url.remove(0, 8);

  if (url.endsWith('/'))
    url.chop(1);
 
  return username + "@" + url;
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
    dateStr = QObject::tr("a few seconds ago");
  } else if (mins == 1) {
    dateStr = QObject::tr("one minute ago");
  } else if (mins < 60) {
    dateStr = QObject::tr("%n minute(s) ago", 0, mins);
  } else if (hours >= 1 && hours < 24) {
    dateStr = QObject::tr("%n hour(s) ago", 0, hours);
  }
  return dateStr;
}

//------------------------------------------------------------------------------

bool splitWebfingerId(QString accountId, QString& username, QString& server) {
  static QRegExp rx("^([\\w\\._-+]+)@([\\w\\._-+]+)$");
  if (!rx.exactMatch(accountId.trimmed()))
    return false;

  username = rx.cap(1);
  server = rx.cap(2);
  return true;
}

//------------------------------------------------------------------------------

long getMaxRSS() {
#ifdef DEBUG_MEMORY
  struct rusage rusage;
  getrusage(RUSAGE_SELF, &rusage);
  return rusage.ru_maxrss;
#else
  return 0;
#endif
}

//------------------------------------------------------------------------------

long getCurrentRSS() {
#ifdef DEBUG_MEMORY
  QFile fp("/proc/self/statm");
  if (!fp.open(QIODevice::ReadOnly))
    return -1;

  QTextStream in(&fp);
  QString line = in.readLine();
  QStringList parts = line.split(" ");

  return parts[1].toLong() * sysconf( _SC_PAGESIZE);
#else
  return 0;
#endif
}

//------------------------------------------------------------------------------

void checkMemory(QString desc) {
  static long oldMem = -1;
  
  long mem = getCurrentRSS();
  long diff = 0;
  if (oldMem > 0)
    diff = mem-oldMem;

  QString msg("RESIDENT MEMORY");
  if (!desc.isEmpty())
    msg += " (" + desc + ")";
  msg += QString(": %1 KB").arg((float)mem/1024.0, 0, 'f', 2);
  if (diff != 0)
    msg += QString(" (%2%1)").arg(diff).arg(diff > 0 ? '+' : '-');

  qDebug() << msg;
}

//------------------------------------------------------------------------------

QString addTextMarkup(QString text) {
  QString oldText = text;

#ifdef DEBUG_MARKUP
  qDebug() << "\n[DEBUG] MARKUP\n" << text;
#endif

  // Remove any inline HTML tags
  // text.replace(QRegExp(HTML_TAG_REGEX), "&lt;\\1&gt;");
  QRegExp rx(HTML_TAG_REGEX);
  QRegExp urlRx(URL_REGEX);
  int pos = 0;
  
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString tag = rx.cap(1);
    if (urlRx.exactMatch(tag)) {
      pos += len;
    } else {
      QString newText = "&lt;" + tag + "&gt;";
      text.replace(pos, len, newText);
      pos += newText.length();
    }
  }

#ifdef DEBUG_MARKUP
  qDebug() << "\n[DEBUG] MARKUP (clean inline HTML)\n" << text;
#endif

  text = markDown(text);

#ifdef DEBUG_MARKUP
  qDebug() << "\n[DEBUG] MARKUP (apply Markdown)\n" << text;
#endif
  text = linkifyUrls(text);

#ifdef DEBUG_MARKUP
  qDebug() << "\n[DEBUG] MARKUP (linkify plain URLs)\n" << text;
#endif
  
  return text;
}

//------------------------------------------------------------------------------


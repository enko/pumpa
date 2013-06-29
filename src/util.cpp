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

#include <QRegExp>

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
  QRegExp rx(URL_REGEX);

  int pos = 0;
  while ((pos = rx.indexIn(text, pos)) != -1) {
    int len = rx.matchedLength();
    QString newText = QString("<a href=\"%1\">%1</a>").arg(rx.cap(1));
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

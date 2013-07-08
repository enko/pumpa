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

#ifndef _UTIL_H_
#define _UTIL_H_

#include <QString>

//------------------------------------------------------------------------------

#define URL_REGEX "((https?://|www.)[^\\s\"]+\\.[^\\s\"]+[^\\s\\.\"])"
#define MD_NOGO_ITEMS "\\*`_"
#define MD_PAIR_REGEX "%1([^\\s%3][^%3]*[^\\s%3]|[^\\s%3])%2"
#define HTML_TAG_REGEX "<([^>]+)>"

//------------------------------------------------------------------------------

/*
  Fixes site url, removes extra / from end, adds https:// if missing.
*/
QString siteUrlFixer(QString url);

/* 
   Finds things that look like URLs and changes them into a href
   links.  
*/
QString linkifyUrls(QString text, QString before=".*", QString after=".*");


/* 
   Finds things delimited by 'begin' and 'end' and changes them to be
   delimited by 'newBegin' and 'newEnd'.
*/
QString changePairedTags(QString text,
                         QString begin, QString end,
                         QString newBegin, QString newEnd,
                         QString nogoItems = MD_NOGO_ITEMS);

/*
  Transforms foo and https://bar.com to foo@bar.com
*/
QString siteUrlToAccountId(QString username, QString url);

QString markDown(QString text);

#endif /* _UTIL_H_ */

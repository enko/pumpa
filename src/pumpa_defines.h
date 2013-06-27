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

#ifndef _PUMPA_DEFINES_H_
#define _PUMPA_DEFINES_H_

#define CLIENT_NAME           "pumpa"
#define CLIENT_FANCY_NAME     "Pumpa"
#define CLIENT_VERSION        "0.3"

#define WEBSITE_URL           "http://saz.im/software/pumpa.html"
#define MARKUP_DOC_URL        "http://saz.im/software/pumpa.html#markup"

#define FEED_INBOX            8
#define FEED_MENTIONS         4
#define FEED_DIRECT           2
#define FEED_MEANWHILE        1

//------------------------------------------------------------------------------

// First byte is used to tell the slot receiving the network reply how
// to interpret the response. (Most are just what activitystreams
// class to hand it off to, or some simple action to perform at once).

#define QAS_NULL         0
#define QAS_COLLECTION   1
#define QAS_ACTIVITY     2
#define QAS_OBJECTLIST   3
#define QAS_OBJECT       4
#define QAS_ACTORLIST    5
// #define QAS_ACTOR        6
#define QAS_SELF_PROFILE 7

// The higher bits can be used for info for the whatever method is
// handling the further processing.
#define QAS_NEWER        (1 << 8)
#define QAS_OLDER        (1 << 9)

#define QAS_REFRESH      (1 << 10)
#define QAS_TOGGLE_LIKE  (1 << 11)

//------------------------------------------------------------------------------

#endif /* _PUMPA_DEFINES_H_ */

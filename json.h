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

#ifndef _JSON_H_
#define _JSON_H_

#include <QVariantMap>

//------------------------------------------------------------------------------

QVariantMap parseJson(QByteArray data);

QByteArray serializeJson(QVariantMap json);

const char* serializeJsonC(QVariantMap json);

QString debugDumpJson(QVariantMap json, QString name = "",
                      QString indent = "");

QString debugDumpJson(QVariantList json, QString name = "",
                      QString indent = "");

QString debugDumpJson(QVariant json, QString name = "",
                      QString indent = "");

#endif /* _JSON_H_ */

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

#include "json.h"

#ifdef QT5
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#else
#include <QVariant>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#endif

//------------------------------------------------------------------------------

QVariantMap parseJson(QByteArray data) {
#ifdef QT5
  return QJsonDocument::fromJson(data).object().toVariantMap();
#else
  QJson::Parser parser;
  bool ok;

  QVariantMap json = parser.parse(data, &ok).toMap();
  if (!ok)
    qWarning("Unable to parse JSON! " + data);
  return json;
#endif
}

//------------------------------------------------------------------------------

QByteArray serializeJson(QVariantMap json) {
#ifdef QT5
  QJsonDocument jd(QJsonObject::fromVariantMap(json));
  return jd.toJson();
#else
  QJson::Serializer serializer;
  QByteArray data = serializer.serialize(json);
  return data;
#endif
}

//------------------------------------------------------------------------------

const char* serializeJsonC(QVariantMap json) {
  return QString(serializeJson(json)).toLatin1().data();
}

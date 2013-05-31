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

#ifndef _JSON_WRAPPER_H_
#define _JSON_WRAPPER_H_

#ifdef QT5
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

//------------------------------------------------------------------------------

QJsonObject convertJson(const QByteArray& data) {
  return QJsonDocument::fromJson(data).object();
}

//------------------------------------------------------------------------------

QByteArray toJson(const QVariantMap& map) {
  QJsonDocument jd(QJsonObject::fromVariantMap(map));
  return jd.toJson();
}

//------------------------------------------------------------------------------

QString jsonDump(const QJsonObject& obj) {
  return QString(QJsonDocument(obj).toJson());
}

//------------------------------------------------------------------------------

const char* jsonDumpC(const QJsonObject& obj) {
  return jsonDump(obj).toLatin1().data();
}

//------------------------------------------------------------------------------

#else
#include <QVariantMap>
// #include <QJson>
typedef QVariantMap QJsonObject;

QJsonObject convertJson(const QByteArray& /*data*/) {
  // QJson::Parser parser;
  // bool ok;

  // QVariantMap result = parser.parse(data, &ok).toMap();
  // if (!ok) {
  //   qFatal("An error occurred during parsing");
  //   exit (1);
  // }

  // return result;
  return QVariantMap();
}

//------------------------------------------------------------------------------

QByteArray toJson(const QVariantMap& /*map*/) {
  // QJson::Serializer serializer;
 
  // bool ok;
  // QByteArray json = serializer.serialize(map, &ok);
  
  // if (!ok) {
  //   qCritical() << "Something went wrong:" << serializer.errorMessage();
  // }
  // return json;
  return QByteArray();
}

//------------------------------------------------------------------------------

QString jsonDump(const QJsonObject& obj) {
  return QString(toJson(obj));
}

#endif

//------------------------------------------------------------------------------

const char* jsonDumpC(const QJsonObject& obj) {
  return jsonDump(obj).toLatin1().data();
}

#endif /* _JSON_WRAPPER_H_ */

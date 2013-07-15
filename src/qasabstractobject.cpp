/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

//------------------------------------------------------------------------------

#include "qasabstractobject.h"

//------------------------------------------------------------------------------

QDateTime parseTime(QString timeStr) {
  // 2013-05-28T16:43:06Z 
  // 55 minutes ago kl. 20:39 -> 19:44

  QDateTime dt = QDateTime::fromString(timeStr, Qt::ISODate); 
                                       // "yyyy-MM-ddThh:mm:ssZ");
  dt.setTimeSpec(Qt::UTC);

  return dt;
}

//------------------------------------------------------------------------------

QASAbstractObject::QASAbstractObject(int asType, QObject* parent) :
  QObject(parent),
  m_asType(asType) 
{}

//------------------------------------------------------------------------------

void QASAbstractObject::connectSignals(QASAbstractObject* obj,
                                       bool changed, bool req) {
  if (!obj)
    return;

  if (changed)
    connect(obj, SIGNAL(changed()),
            this, SIGNAL(changed()), Qt::UniqueConnection);
  if (req)
    connect(obj, SIGNAL(request(QString, int)),
            parent(), SLOT(request(QString, int)), Qt::UniqueConnection);
}

//------------------------------------------------------------------------------

void QASAbstractObject::refresh() {
  QDateTime now = QDateTime::currentDateTime();

  if (m_lastRefreshed.isNull() || m_lastRefreshed.secsTo(now) > 1)
    emit request(apiLink(), m_asType);

  m_lastRefreshed = now;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, QString& var, QString name, 
                                  bool& changed) {
  QString oldVar = var;
  if (obj.contains(name))
    var = obj[name].toString();
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, bool& var, QString name,
                                  bool& changed) {
  bool oldVar = var;
  if (obj.contains(name))
    var = obj[name].toBool();
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, qulonglong& var,
                                  QString name, bool& changed,
                                  bool ignoreDecrease) {
  qulonglong oldVar = var;
  if (obj.contains(name))
    var = obj[name].toULongLong();
  if ((var > oldVar) || ((var < oldVar) && !ignoreDecrease))
    changed = true;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, QDateTime& var,
                                  QString name, bool& changed) {
  QDateTime oldVar = var;
  if (obj.contains(name))
    var = parseTime(obj[name].toString());
  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, QString& var, QString name1,
                                  QString name2, bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, changed);
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, bool& var, QString name1,
                                  QString name2, bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, changed);
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateVar(QVariantMap obj, QString& var, QString name1,
                                  QString name2, QString name3, bool& changed) {
  if (obj.contains(name1))
    updateVar(obj[name1].toMap(), var, name2, name3, changed);
}

//------------------------------------------------------------------------------

void QASAbstractObject::addVar(QVariantMap& obj, QString var, QString name) {
  if (var.isEmpty())
    return;
  obj[name] = var;
}

//------------------------------------------------------------------------------

void QASAbstractObject::updateUrlOrProxy(QVariantMap obj, QString& var,
                                         bool& changed) {
  QString oldVar = var;
  bool dummy;
  updateVar(obj, var, "url", dummy);
  updateVar(obj, var, "pump_io", "proxyURL", dummy);

  if (oldVar.contains("/api/proxy/") && !var.contains("/api/proxy/"))
    var = oldVar;

  if (oldVar != var) changed = true;
}

//------------------------------------------------------------------------------

qint64 QASAbstractObject::sortIntByDateTime(QDateTime dt) {
  return dt.toMSecsSinceEpoch();
}

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

#ifndef _QASABSTRACTOBJECT_H_
#define _QASABSTRACTOBJECT_H_

#include <QObject>
#include <QDateTime>
#include <QVariantMap>

#include "pumpa_defines.h"
#include "json.h"

//------------------------------------------------------------------------------

class QASAbstractObject : public QObject {
  Q_OBJECT

public:
  // virtual void refresh();
  virtual QString apiLink() const { return ""; }
  int asType() const { return m_asType; }
  virtual bool isDeleted() const { return false; }

  QDateTime lastRefreshed() const { return m_lastRefreshed; }
  void lastRefreshed(QDateTime dt) { m_lastRefreshed = dt; }

signals:
  void changed();
  // void request(QString, int);

protected:
  QASAbstractObject(int asType, QObject* parent);
  virtual void connectSignals(QASAbstractObject* obj,
                              bool changed=true, bool req=true);

  static qint64 sortIntByDateTime(QDateTime dt);

  static void updateVar(QVariantMap, QString&, QString, bool&);
  static void updateVar(QVariantMap, bool&, QString, bool&);
  static void updateVar(QVariantMap, qulonglong&, QString, bool&,
                        bool ignoreDecrease=false);
  static void updateVar(QVariantMap, QDateTime&, QString, bool&);
  static void updateVar(QVariantMap, QString&, QString, QString, bool&);
  static void updateVar(QVariantMap, bool&, QString, QString, bool&);
  static void updateVar(QVariantMap, QString&, QString, QString, QString,
                        bool&);
  static void addVar(QVariantMap&, QString, QString);
  static void updateUrlOrProxy(QVariantMap, QString&, bool&);

  QDateTime m_lastRefreshed;
  int m_asType;
};

#endif /* _QASABSTRACTOBJECT_H_ */

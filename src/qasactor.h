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

#ifndef _QASACTOR_H_
#define _QASACTOR_H_

#include "qasobject.h"

//------------------------------------------------------------------------------

class QASActor : public QASObject {
  Q_OBJECT

private:
  QASActor(QString id, QObject* parent);

public:
  static void clearCache();

  static QASActor* getActor(QVariantMap json, QObject* parent);
  virtual void update(QVariantMap json);

  QString webFinger() const { return m_webFinger; }
  QString webFingerName() const { return m_webFingerName; }
  QString displayNameOrWebFinger() const;
  QString displayNameOrWebFingerShort() const;
  QString preferredUsername() const { return m_preferredUsername; }

  QString displayNameOrYou() const { return isYou() ? "You" : displayName(); }
  bool isYou() const { return m_isYou; }
  void setYou() { m_isYou = true; }

  bool followed() const { return m_followed; }
  bool followedJson() const { return m_followed_json; }
  void setFollowed(bool b);
  QString summary() const { return m_summary; }
  QString location() const { return m_location; }

private:
  bool m_followed;
  bool m_followed_json;
  bool m_isYou;
  QString m_summary;
  QString m_location;

  QString m_webFinger;
  QString m_webFingerName;
  QString m_preferredUsername;
};

#endif /* _QASACTOR_H_ */

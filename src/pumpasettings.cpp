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

#include <QFile>

#include "pumpasettings.h"
#include "pumpa_defines.h"
#include "util.h"

//------------------------------------------------------------------------------

PumpaSettings::PumpaSettings(QString filename, QObject* parent) :
  QObject(parent) {
  if (filename.isEmpty())
    m_s = new QSettings(CLIENT_NAME, CLIENT_NAME, this);
  else
    m_s = new QSettings(filename, QSettings::IniFormat, this);

  QFile::setPermissions(m_s->fileName(),
                        QFile::ReadOwner | QFile::WriteOwner);
}

//------------------------------------------------------------------------------

QVariant PumpaSettings::getValue(QString name, QVariant defaultValue,
                                 QString group) const {
  m_s->beginGroup(group);
  QVariant v = m_s->value(name, defaultValue);
  m_s->endGroup();
  return v;
}  

//------------------------------------------------------------------------------

QString PumpaSettings::siteUrl() const {
  return siteUrlFixer(getValue("site_url", "", "Account").toString());
}

//------------------------------------------------------------------------------

int PumpaSettings::reloadTime() const {
  int reloadTime = getValue("reload_time", 1, "General").toInt();
  if (reloadTime < 1)
    reloadTime = 1;
  return reloadTime;
}

//------------------------------------------------------------------------------

int PumpaSettings::highlightFeeds() const {
  return getValue("highlight_feeds", 0, "General").toInt();
}

//------------------------------------------------------------------------------

int PumpaSettings::popupFeeds() const {
  return getValue("popup_feeds", 0, "General").toInt();
}

//------------------------------------------------------------------------------
// Setters
//------------------------------------------------------------------------------

void PumpaSettings::setValue(QString name, QVariant value, QString group) {
  m_s->beginGroup(group);
  m_s->setValue(name, value);
  m_s->endGroup();
}

//------------------------------------------------------------------------------

void PumpaSettings::useTrayIcon(bool b) { 
  bool old = useTrayIcon();
  setValue("use_tray_icon", b); 
  if (old != b)
    emit trayIconChanged();
}

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

#ifndef _PUMPAPP_H_
#define _PUMPAPP_H_

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QJsonDocument>

#include "QtKOAuth"

class PumpApp : public QObject {
  Q_OBJECT

public:
  PumpApp(QObject* parent=0);

private slots:
  void errorMessage(const QString& msg);

  void onTemporaryTokenReceived(QString temporaryToken,
                                QString temporaryTokenSecret);
  void onAuthorizationReceived(QString token, QString verifier);
  void onAccessTokenReceived(QString token, QString tokenSecret);
  void onRequestReady(QByteArray);
  void onAuthorizedRequestDone();
  
private:
  void fetchInbox();
  void postNote(QString);
  void feed(QString verb, QVariantMap object);
  void request(QString endpoint, QString method="GET",
               QVariantMap data=QVariantMap()); //attempts=10

  void getOAuthAccess();
  
  QString client_id;
  QString client_secret;

  void writeSettings();
  void readSettings();

  QSettings* settings;
  
  QString siteUrl;
  QString userName;

  QString token;
  QString tokenSecret;

  KQOAuthManager *oaManager;
  KQOAuthRequest *oaRequest;
};

#endif /* _PUMPAPP_H_ */

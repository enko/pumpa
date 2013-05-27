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

#ifndef _PUMPAPP_H_
#define _PUMPAPP_H_

#include <QObject>
#include <QSettings>
#include <QDebug>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonArray>

#include "QtKOAuth"

#define CLIENT_NAME           "pumpa"
#define CLIENT_FANCY_NAME     "pumpa"

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
  // void onRequestReady(QByteArray);
  // void onAuthorizedRequestDone();
  void onOAuthClientRegDone();

  void onAuthorizedRequestReady(QByteArray response, int id);

private:
  void getOAuthAccess();
  void registerOAuthClient();

  void fetchAll();
  void fetchInbox();
  void postNote(QString);
  void feed(QString verb, QVariantMap object);
  void request(QString endpoint, 
               KQOAuthRequest::RequestHttpMethod method = KQOAuthRequest::GET,
               QVariantMap data=QVariantMap()); //attempts=10
  
  void writeSettings();
  void readSettings();

  QSettings* settings;
  
  QString siteUrl;
  QString userName;

  QString clientId;
  QString clientSecret;

  QString token;
  QString tokenSecret;

  KQOAuthManager *oaManager;
  KQOAuthRequest *oaRequest;

  QNetworkAccessManager *netManager;
};

#endif /* _PUMPAPP_H_ */

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

#include <QJsonDocument>
#include <QJsonObject>

#include "pumpapp.h"

#include "qactivitystreams.h"

//------------------------------------------------------------------------------

PumpApp::PumpApp(QObject* parent) : QObject(parent) {
  settings = new QSettings("pumpa", "pumpa", this);
  readSettings();

  // frodo:8000
  client_id = "qCPXxO_D5knAGhD8yLP99A";
  client_secret = "bPViYRQWRBTkbQ568nWrTyHSVCxBiL9zVnqKwVhVQCk";

  oaRequest = new KQOAuthRequest(this);
  oaManager = new KQOAuthManager(this);

  oaRequest->setEnableDebugOutput(true);

  if (token.isEmpty()) {
    getOAuthAccess();
  } else {
    // postNote("Hello from pumpa!");
    fetchInbox();  
  }
}

//------------------------------------------------------------------------------

void PumpApp::fetchInbox() {
  QString inbox = "major"; // "minor"

  QString endpoint = "api/user/"+userName+"/inbox";

  // if (direct)
  //   endpoint += "/direct";

  endpoint += "/"+inbox;

  request(endpoint);
}

//------------------------------------------------------------------------------

void PumpApp::postNote(QString note) {
  if (userName.isEmpty() || note.isEmpty()) {
    qDebug() << "You need to set userName and have non-empty note.";
    return;
  }

  QVariantMap obj;
  obj["objectType"] = "note";
  obj["content"] = note;

  feed("post", obj);
}

//------------------------------------------------------------------------------

void PumpApp::feed(QString verb, QVariantMap object) {
  QString endpoint = "api/user/"+userName+"/feed";

  QVariantMap data;
  if (!verb.isEmpty()) {
    data["verb"] = verb;
    data["object"] = object;
  }

  request(endpoint, KQOAuthRequest::POST, data);
}

//------------------------------------------------------------------------------

void PumpApp::request(QString endpoint,
                      KQOAuthRequest::RequestHttpMethod method,
                      QVariantMap data) {
  if (endpoint[0] != '/')
    endpoint = '/'+endpoint;

  oaRequest->initRequest(KQOAuthRequest::AuthorizedRequest, 
                         QUrl(siteUrl+endpoint));
  oaRequest->setConsumerKey(client_id);
  oaRequest->setConsumerSecretKey(client_secret);

  oaRequest->setToken(token);
  oaRequest->setTokenSecret(tokenSecret);

  oaRequest->setHttpMethod(method); 

  if (method == KQOAuthRequest::POST) {
    QJsonDocument jd(QJsonObject::fromVariantMap(data));
  //   QByteArray ba();

  // qDebug() << "Sending" << ba;

    oaRequest->setContentType("application/json");
    oaRequest->setRawData(jd.toJson());
  }

  oaManager->executeRequest(oaRequest);
    
  connect(oaManager, SIGNAL(requestReady(QByteArray)),
          this, SLOT(onRequestReady(QByteArray)));
  connect(oaManager, SIGNAL(authorizedRequestDone()),
          this, SLOT(onAuthorizedRequestDone()));
}

//------------------------------------------------------------------------------

void PumpApp::getOAuthAccess() {
  connect(oaManager, SIGNAL(temporaryTokenReceived(QString, QString)),
          this, SLOT(onTemporaryTokenReceived(QString, QString)));
  connect(oaManager, SIGNAL(authorizationReceived(QString,QString)),
          this, SLOT(onAuthorizationReceived(QString, QString)));
  connect(oaManager, SIGNAL(accessTokenReceived(QString,QString)),
          this, SLOT(onAccessTokenReceived(QString,QString)));
  connect(oaManager, SIGNAL(requestReady(QByteArray)),
          this, SLOT(onRequestReady(QByteArray)));

  oaRequest->initRequest(KQOAuthRequest::TemporaryCredentials,
                         QUrl(siteUrl+"/oauth/request_token"));

  oaRequest->setConsumerKey(client_id);
  oaRequest->setConsumerSecretKey(client_secret);

  oaManager->setHandleUserAuthorization(true);
  oaManager->executeRequest(oaRequest);
}

//------------------------------------------------------------------------------

void PumpApp::errorMessage(const QString& msg) {
  qDebug() << "errorMessage:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::onTemporaryTokenReceived(QString token, QString tokenSecret) {
  qDebug() << "Temporary token received: " << token << tokenSecret;

    QUrl userAuthURL(siteUrl+"/oauth/authorize");

    if (oaManager->lastError() == KQOAuthManager::NoError) {
      qDebug() << "Asking for user's permission to access protected resources."
               << "Opening URL: " << userAuthURL;
      oaManager->getUserAuthorization(userAuthURL);
    }
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizationReceived(QString token, QString verifier) {
    qDebug() << "User authorization received: " << token << verifier;

    oaManager->getUserAccessTokens(QUrl(siteUrl+"/oauth/access_token"));
    if (oaManager->lastError() != KQOAuthManager::NoError) {
      qDebug() << "DONE1";
    }
}

//------------------------------------------------------------------------------

void PumpApp::onAccessTokenReceived(QString token, QString tokenSecret) {
    qDebug() << "Access token received: " << token << tokenSecret;

    this->token = token;
    this->tokenSecret = tokenSecret;

    writeSettings();

    qDebug() << "Access tokens now stored.";
}

//------------------------------------------------------------------------------

void PumpApp::onRequestReady(QByteArray response) {
  qDebug() << "onRequestReady"; // << response;
  QJsonObject obj = QJsonDocument::fromJson(response).object();
  
  QASCollection coll(obj, this);
  // QJsonArray arr = obj["items"].toArray();
  
  // for (int i=0; i<arr.count(); i++) {
  //   QJsonObject obj = arr.at(i).toObject(); 
  //   qDebug() << obj["object"].toObject()["content"].toString();
  // }

  qDebug() << "DONE2!";
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestDone() {
  qDebug() << "onAuthorizedRequestDone";
}

//------------------------------------------------------------------------------

void PumpApp::writeSettings() {
  QSettings& s = *settings;
  s.beginGroup("Account");

  s.setValue("site_url", siteUrl);
  s.setValue("username", userName);

  s.setValue("oauth_token", token);
  s.setValue("oauth_token_secret", tokenSecret);

  s.endGroup();
}

//------------------------------------------------------------------------------

void PumpApp::readSettings() {
  QSettings& s = *settings;
  s.beginGroup("Account");

  siteUrl = s.value("site_url", "http://frodo:8000").toString();
  userName = s.value("username", "").toString();

  token = s.value("oauth_token", "").toString();
  tokenSecret = s.value("oauth_token_secret", "").toString();

  s.endGroup();
}

//------------------------------------------------------------------------------

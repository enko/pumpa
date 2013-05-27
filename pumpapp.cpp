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

#include <QJsonDocument>
#include <QJsonObject>

#include "pumpapp.h"

#include "qactivitystreams.h"

#define OAR_USER_ACCESS       0
#define OAR_FETCH_INBOX       1

//------------------------------------------------------------------------------

PumpApp::PumpApp(QWidget* parent) : QMainWindow(parent) {
  settings = new QSettings(CLIENT_NAME, CLIENT_NAME, this);
  readSettings();

  netManager = new QNetworkAccessManager(this);

  oaRequest = new KQOAuthRequest(this);
  oaManager = new KQOAuthManager(this);
  connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray, int)),
          this, SLOT(onAuthorizedRequestReady(QByteArray, int)));

  inboxWidget = new CollectionWidget(this);

  setWindowTitle(CLIENT_FANCY_NAME);
  setCentralWidget(inboxWidget);
  show();

  // oaRequest->setEnableDebugOutput(true);

  if (clientId.isEmpty())
    registerOAuthClient();
  else if (token.isEmpty())
    getOAuthAccess();
  else
    fetchAll();
}

//------------------------------------------------------------------------------

void PumpApp::errorMessage(const QString& msg) {
  qDebug() << "errorMessage:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::writeSettings() {
  QSettings& s = *settings;

  s.beginGroup("MainWindow");
  s.setValue("size", size());
  s.setValue("pos", pos());
  s.endGroup();

  s.beginGroup("Account");
  s.setValue("site_url", siteUrl);
  s.setValue("username", userName);
  s.setValue("oauth_client_id", clientId);
  s.setValue("oauth_client_secret", clientSecret);
  s.setValue("oauth_token", token);
  s.setValue("oauth_token_secret", tokenSecret);
  s.endGroup();
}

//------------------------------------------------------------------------------

void PumpApp::readSettings() {
  QSettings& s = *settings;

  s.beginGroup("MainWindow");
  resize(s.value("size", QSize(550, 500)).toSize());
  move(s.value("pos", QPoint(0, 0)).toPoint());
  s.endGroup();

  s.beginGroup("Account");
  siteUrl = s.value("site_url", "http://frodo:8000").toString();
  userName = s.value("username", "").toString();
  clientId = s.value("oauth_client_id", "").toString();
  clientSecret = s.value("oauth_client_secret", "").toString();
  token = s.value("oauth_token", "").toString();
  tokenSecret = s.value("oauth_token_secret", "").toString();
  s.endGroup();
}

//------------------------------------------------------------------------------

void PumpApp::registerOAuthClient() {
  QNetworkRequest req;
  req.setUrl(QUrl(siteUrl+"/api/client/register"));
  req.setHeader(QNetworkRequest::ContentTypeHeader, 
                "application/x-www-form-urlencoded");

  QString data = QString("client_name=") + CLIENT_FANCY_NAME +
    "&type=client_associate&application_type=native";
  QByteArray postData = QUrl::toPercentEncoding(data, "=&");
  
  QNetworkReply *reply = netManager->post(req, postData);

  connect(reply, SIGNAL(finished()), this, SLOT(onOAuthClientRegDone()));
}

//------------------------------------------------------------------------------

void PumpApp::onOAuthClientRegDone() {
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  QByteArray data = reply->readAll();

  QJsonObject json = QJsonDocument::fromJson(data).object();
  clientId = json["client_id"].toString();
  clientSecret = json["client_secret"].toString();

  qDebug() << "Registered client to [" << siteUrl << "] successfully.";
  writeSettings();

  getOAuthAccess();
}

//------------------------------------------------------------------------------

void PumpApp::getOAuthAccess() {
  connect(oaManager, SIGNAL(temporaryTokenReceived(QString, QString)),
          this, SLOT(onTemporaryTokenReceived(QString, QString)));
  connect(oaManager, SIGNAL(authorizationReceived(QString,QString)),
          this, SLOT(onAuthorizationReceived(QString, QString)));
  connect(oaManager, SIGNAL(accessTokenReceived(QString,QString)),
          this, SLOT(onAccessTokenReceived(QString,QString)));

  oaRequest->initRequest(KQOAuthRequest::TemporaryCredentials,
                         QUrl(siteUrl+"/oauth/request_token"));

  oaRequest->setConsumerKey(clientId);
  oaRequest->setConsumerSecretKey(clientSecret);

  oaManager->setHandleUserAuthorization(true);
  oaManager->executeRequest(oaRequest);
}

//------------------------------------------------------------------------------

void PumpApp::onTemporaryTokenReceived(QString /*token*/,
                                       QString /*tokenSecret*/) {
  QUrl userAuthURL(siteUrl+"/oauth/authorize");

  if (oaManager->lastError() == KQOAuthManager::NoError) {
    qDebug() << "Asking for user's permission to access protected resources."
             << "Opening URL: " << userAuthURL;
    oaManager->getUserAuthorization(userAuthURL);
  }
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizationReceived(QString /*token*/,
                                      QString /*verifier*/) {
  oaManager->getUserAccessTokens(QUrl(siteUrl+"/oauth/access_token"));
}

//------------------------------------------------------------------------------

void PumpApp::onAccessTokenReceived(QString token, QString tokenSecret) {
    this->token = token;
    this->tokenSecret = tokenSecret;

    writeSettings();

    qDebug() << "User authorised for [" << siteUrl << "]";

    fetchAll();
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestReady(QByteArray response, int id) {
  qDebug() << "onRequestReady" << id;

  if (id == OAR_USER_ACCESS) {
    qDebug() << "uhm, ok.";
  } else if (id == OAR_FETCH_INBOX) {
    qDebug() << response;
    QJsonObject obj = QJsonDocument::fromJson(response).object();
  
    QASCollection coll(obj, this);

    inboxWidget->setCollection(coll);
  } else {
    qDebug() << "Unknown request id!";
  }
}

//------------------------------------------------------------------------------

void PumpApp::fetchAll() {
  fetchInbox();
}

//------------------------------------------------------------------------------

void PumpApp::fetchInbox() {
  QString inbox = "major"; 
    // "minor";

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
  oaRequest->setConsumerKey(clientId);
  oaRequest->setConsumerSecretKey(clientSecret);

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

  oaManager->executeAuthorizedRequest(oaRequest, OAR_FETCH_INBOX);
    
  // connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray)),
  //         this, SLOT(onRequestReady(QByteArray)));
  // connect(oaManager, SIGNAL(authorizedRequestDone()),
  //         this, SLOT(onAuthorizedRequestDone()));
}

//------------------------------------------------------------------------------


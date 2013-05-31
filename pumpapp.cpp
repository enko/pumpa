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

#include "pumpapp.h"
#include "json.h"
#include "qactivitystreams.h"
#include "messagewindow.h"

//------------------------------------------------------------------------------

PumpApp::PumpApp(QWidget* parent) : QMainWindow(parent) {
  settings = new QSettings(CLIENT_NAME, CLIENT_NAME, this);
  readSettings();

  netManager = new QNetworkAccessManager(this);

  oaRequest = new KQOAuthRequest(this);
  oaManager = new KQOAuthManager(this);
  connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray, int)),
          this, SLOT(onAuthorizedRequestReady(QByteArray, int)));
  connect(oaManager, SIGNAL(authorizedRequestDone()),
          this, SLOT(onAuthorizedRequestDone()));

  createActions();
  createMenu();

  inboxWidget = new CollectionWidget(this);

  setWindowTitle(CLIENT_FANCY_NAME);
  setWindowIcon(QIcon(":/pumpa.png"));
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

PumpApp::~PumpApp() {
  writeSettings();
}

//------------------------------------------------------------------------------

void PumpApp::errorMessage(QString msg) {
  qDebug() << "errorMessage:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::createActions() {
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

  openPrefsAction = new QAction(tr("Preferences"), this);
  // prefsAct->setShortcut(tr("Ctrl+P"));
  connect(openPrefsAction, SIGNAL(triggered()), this, SLOT(preferences()));

  reloadAction = new QAction(tr("&Reload timeline"), this);
  reloadAction->setShortcut(tr("Ctrl+R"));
  connect(reloadAction, SIGNAL(triggered()), 
          this, SLOT(reload()));

  // loadOlderAct = new QAction(tr("Load &older in timeline"), this);
  // loadOlderAct->setShortcut(tr("Ctrl+O"));
  // connect(loadOlderAct, SIGNAL(triggered()), this, SLOT(loadOlder()));

  // pauseAct = new QAction(tr("&Pause home timeline"), this);
  // pauseAct->setShortcut(tr("Ctrl+P"));
  // pauseAct->setCheckable(true);
  // connect(pauseAct, SIGNAL(triggered()), this, SLOT(pauseTimeline()));

  aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAction = new QAction("About &Qt", this);
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  newNoteAction = new QAction(tr("New &Note"), this);
  newNoteAction->setShortcut(tr("Ctrl+N"));
  connect(newNoteAction, SIGNAL(triggered()), this, SLOT(newNote()));

  newPictureAction = new QAction(tr("New &Picture"), this);
  newPictureAction->setShortcut(tr("Ctrl+P"));
  connect(newPictureAction, SIGNAL(triggered()), this, SLOT(newPicture()));
}

//------------------------------------------------------------------------------

void PumpApp::createMenu() {
  fileMenu = new QMenu(tr("&Pumpa"), this);
  fileMenu->addAction(newNoteAction);
  fileMenu->addAction(newPictureAction);
  // fileMenu->addSeparator();
  fileMenu->addAction(reloadAction);
  // fileMenu->addAction(loadOlderAct);
  // fileMenu->addAction(pauseAct);
  fileMenu->addSeparator();
  fileMenu->addAction(openPrefsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);
  menuBar()->addMenu(fileMenu);

  helpMenu = new QMenu(tr("&Help"), this);
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutQtAction);
  menuBar()->addMenu(helpMenu);
}

//------------------------------------------------------------------------------

void PumpApp::preferences() {
  //settingsDialog->exec();
}

//------------------------------------------------------------------------------

void PumpApp::exit() { 
  qApp->exit();
}

//------------------------------------------------------------------------------

void PumpApp::about() { 
  static const QString GPL = 
    "<p>Pumpa is free software: you can redistribute it and/or modify it "
    "under the terms of the GNU General Public License as published by "
    "the Free Software Foundation, either version 3 of the License, or "
    "(at your option) any later version.</p>"
    "<p>Pumpa is distributed in the hope that it will be useful, but "
    "WITHOUT ANY WARRANTY; without even the implied warranty of "
    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
    "General Public License for more details.</p>"
    "<p>You should have received a copy of the GNU General Public License "
    "along with Pumpa.  If not, see "
    "<a href=\"http://www.gnu.org/licenses/\">http://www.gnu.org/licenses/</a>."
    "</p>";

  QMessageBox::about(this, tr("About " CLIENT_FANCY_NAME), 
                     "<p><b>" CLIENT_FANCY_NAME " " CLIENT_VERSION "</b> - "
                     "A simple Qt-based pump.io client.</p>"
                     "<p>Copyright &copy; 2013 Mats Sj&ouml;berg.</p>"+GPL);
}

//------------------------------------------------------------------------------

void PumpApp::newNote() {
  MessageWindow* w = new MessageWindow(this);
  connect(w, SIGNAL(sendMessage(QString)), this, SLOT(postNote(QString)));
  w->show();
}

//------------------------------------------------------------------------------

void PumpApp::newPicture() {
}

//------------------------------------------------------------------------------

void PumpApp::reload() {
  fetchAll();
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

  QVariantMap json = parseJson(data);
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
    // qDebug() << response;
    QVariantMap obj = parseJson(response);
  
    QASCollection coll(obj, this);

    inboxWidget->addCollection(coll);
  } else if (id == OAR_NEW_POST) {
    // nice to refresh inbox after posting
    fetchInbox();
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

  request(endpoint, OAR_FETCH_INBOX);
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

  feed("post", obj, OAR_NEW_POST);
}

//------------------------------------------------------------------------------

void PumpApp::feed(QString verb, QVariantMap object, int oar_id) {
  QString endpoint = "api/user/"+userName+"/feed";

  QVariantMap data;
  if (!verb.isEmpty()) {
    data["verb"] = verb;
    data["object"] = object;
  }

  request(endpoint, oar_id, KQOAuthRequest::POST, data);
}

//------------------------------------------------------------------------------

void PumpApp::request(QString endpoint, int oar_id,
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
  //   QByteArray ba();

  // qDebug() << "Sending" << ba;

    oaRequest->setContentType("application/json");
    oaRequest->setRawData(serializeJson(data));
  }

  oaManager->executeAuthorizedRequest(oaRequest, oar_id);
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestDone() {
  qDebug() << "onAuthorizedRequestDone";
}


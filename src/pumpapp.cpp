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

#include <QStatusBar>

//------------------------------------------------------------------------------

PumpApp::PumpApp(QWidget* parent) : 
  QMainWindow(parent),
  oaWizard(NULL)
{
  settings = new QSettings(CLIENT_NAME, CLIENT_NAME, this);
  readSettings();
  
  netManager = new QNetworkAccessManager(this);

  oaRequest = new KQOAuthRequest(this);
  oaManager = new KQOAuthManager(this);
  connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray, int)),
          this, SLOT(onAuthorizedRequestReady(QByteArray, int)));

  createActions();
  createMenu();

  inboxWidget = new CollectionWidget(this);
  connect(inboxWidget, SIGNAL(request(QString, int)),
          this, SLOT(request(QString, int)));
  connect(inboxWidget, SIGNAL(newReply(QASObject*)),
          this, SLOT(newNote(QASObject*)));
  connect(inboxWidget, SIGNAL(linkHovered(const QString&)),
          this,  SLOT(statusMessage(const QString&)));

  directMajorWidget = new CollectionWidget(this);
  connect(directMajorWidget, SIGNAL(request(QString, int)),
          this, SLOT(request(QString, int)));
  connect(directMajorWidget, SIGNAL(newReply(QASObject*)),
          this, SLOT(newNote(QASObject*)));
  connect(directMajorWidget, SIGNAL(linkHovered(const QString&)),
          this,  SLOT(statusMessage(const QString&)));

  directMinorWidget = new CollectionWidget(this);
  connect(directMinorWidget, SIGNAL(request(QString, int)),
          this, SLOT(request(QString, int)));
  connect(directMinorWidget, SIGNAL(newReply(QASObject*)),
          this, SLOT(newNote(QASObject*)));
  connect(directMinorWidget, SIGNAL(linkHovered(const QString&)),
          this,  SLOT(statusMessage(const QString&)));

  tabWidget = new TabWidget(this);
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));
  tabWidget->addTab(inboxWidget, "inbox");
  tabWidget->addTab(directMinorWidget, "mentions");
  tabWidget->addTab(directMajorWidget, "direct");

  setWindowTitle(CLIENT_FANCY_NAME);
  setWindowIcon(QIcon(":/images/pumpa.png"));
  setCentralWidget(tabWidget);

  // oaRequest->setEnableDebugOutput(true);
  syncOAuthInfo();

  timerId = -1;

  if (!haveOAuth()) {
    oaWizard = new OAuthWizard(this);
    connect(oaWizard, SIGNAL(firstPageCommitted(QString, QString)),
            this, SLOT(onFirstPageCommitted(QString, QString)));
    connect(oaWizard, SIGNAL(rejected()), this, SLOT(exit()));
    connect(oaWizard, SIGNAL(accepted()), this, SLOT(wizardDone()));
    connect(this, SIGNAL(userAuthorizationStarted()),
            oaWizard, SLOT(gotoSecondPage()));
    oaWizard->show();
  } else
    startPumping();
}

//------------------------------------------------------------------------------

PumpApp::~PumpApp() {
  writeSettings();
}

//------------------------------------------------------------------------------

void PumpApp::startPumping() {
  show();
  fetchAll();
  resetTimer();
}

//------------------------------------------------------------------------------

bool PumpApp::haveOAuth() {
  return !clientId.isEmpty() && !clientSecret.isEmpty() &&
    !token.isEmpty() && !tokenSecret.isEmpty();
}

//------------------------------------------------------------------------------

void PumpApp::tabSelected(int index) {
  tabWidget->deHighlightTab(index);
}

//------------------------------------------------------------------------------

void PumpApp::onFirstPageCommitted(QString userName, QString serverUrl) {
  m_userName = userName;
  m_siteUrl = siteUrlFixer(serverUrl);
  registerOAuthClient();
}

//------------------------------------------------------------------------------

void PumpApp::wizardDone() {
  QString token = oaWizard->field("token").toString();
  QString verifier = oaWizard->field("verifier").toString();

  onAuthorizationReceived(token, verifier);
}

//------------------------------------------------------------------------------

void PumpApp::timerEvent(QTimerEvent* event) {
  if (event->timerId() != timerId)
    return;
  fetchAll();
}

//------------------------------------------------------------------------------

void PumpApp::resetTimer() {
  if (timerId != -1)
    killTimer(timerId);
  timerId = startTimer(m_reloadTime*60*1000);
}

//------------------------------------------------------------------------------

void PumpApp::syncOAuthInfo() {
  FileDownloader::setOAuthInfo(m_siteUrl, clientId, clientSecret,
                               token, tokenSecret);
}

//------------------------------------------------------------------------------

void PumpApp::statusMessage(const QString& msg) {
  statusBar()->showMessage(msg);
}

//------------------------------------------------------------------------------

void PumpApp::notifyMessage(QString msg) {
  statusMessage(msg);
  qDebug() << "[STATUS]:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::errorMessage(QString msg) {
  statusMessage("Error: " + msg);
  qDebug() << "[ERROR]:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::createActions() {
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

  openPrefsAction = new QAction(tr("Preferences"), this);
  // prefsAct->setShortcut(tr("Ctrl+P"));
  openPrefsAction->setEnabled(false);
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
  newPictureAction->setEnabled(false);
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

void PumpApp::newNote(QASObject* obj) {
  QASObject* irtObj = obj->inReplyTo();
  if (irtObj)
    obj = irtObj;

  qDebug() << "[DEBUG] Opening reply window to" << obj->type() << obj->id();

  MessageWindow* w = new MessageWindow(obj, this);
  connect(w, SIGNAL(sendMessage(QString)),
          this, SLOT(postNote(QString)));
  connect(w, SIGNAL(sendReply(QASObject*, QString)),
          this, SLOT(postReply(QASObject*, QString)));
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

  QFile::setPermissions(s.fileName(), QFile::ReadOwner | QFile::WriteOwner);

  s.beginGroup("General");
  s.setValue("reload_time", m_reloadTime);
  s.endGroup();

  s.beginGroup("MainWindow");
  s.setValue("size", size());
  s.setValue("pos", pos());
  s.endGroup();

  s.beginGroup("Account");
  s.setValue("site_url", m_siteUrl);
  s.setValue("username", m_userName);
  s.setValue("oauth_client_id", clientId);
  s.setValue("oauth_client_secret", clientSecret);
  s.setValue("oauth_token", token);
  s.setValue("oauth_token_secret", tokenSecret);
  s.endGroup();
}

//------------------------------------------------------------------------------

void PumpApp::readSettings() {
  QSettings& s = *settings;

  s.beginGroup("General");
  m_reloadTime = s.value("reload_time", 5).toInt();
  if (m_reloadTime < 1)
    m_reloadTime = 1;
  s.endGroup();

  s.beginGroup("MainWindow");
  resize(s.value("size", QSize(550, 500)).toSize());
  move(s.value("pos", QPoint(0, 0)).toPoint());
  s.endGroup();

  s.beginGroup("Account");
  m_siteUrl = siteUrlFixer(s.value("site_url", "").toString());
  m_userName = s.value("username", "").toString();
  clientId = s.value("oauth_client_id", "").toString();
  clientSecret = s.value("oauth_client_secret", "").toString();
  token = s.value("oauth_token", "").toString();
  tokenSecret = s.value("oauth_token_secret", "").toString();
  s.endGroup();
}

//------------------------------------------------------------------------------

QString PumpApp::siteUrlFixer(QString url) {
  if (!url.startsWith("http://") && !url.startsWith("https://"))
    url = "https://" + url;

  if (url.endsWith('/'))
    url.chop(1);

  return url;
}

//------------------------------------------------------------------------------

void PumpApp::registerOAuthClient() {
  notifyMessage("Registering client ...");

  QNetworkRequest req;
  req.setUrl(QUrl(m_siteUrl+"/api/client/register"));
  req.setHeader(QNetworkRequest::ContentTypeHeader, 
                "application/json");

  QVariantMap post;
  post["type"] = "client_associate";
  post["application_type"] = "native";
  post["application_name"] = CLIENT_FANCY_NAME;
  // logo_uri

  QByteArray postData = serializeJson(post);
  
  // qDebug() << "data=" << postData;

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

  writeSettings();

  notifyMessage("Registered client to [" + m_siteUrl + "] successfully.");

  getOAuthAccess();
}

//------------------------------------------------------------------------------

void PumpApp::getOAuthAccess() {
  notifyMessage("Authorising user ...");

  connect(oaManager, SIGNAL(temporaryTokenReceived(QString, QString)),
          this, SLOT(onTemporaryTokenReceived(QString, QString)));
  // connect(oaManager, SIGNAL(authorizationReceived(QString,QString)),
  //         this, SLOT(onAuthorizationReceived(QString, QString)));
  connect(oaManager, SIGNAL(accessTokenReceived(QString,QString)),
          this, SLOT(onAccessTokenReceived(QString,QString)));

  oaRequest->initRequest(KQOAuthRequest::TemporaryCredentials,
                         QUrl(m_siteUrl+"/oauth/request_token"));

  oaRequest->setConsumerKey(clientId);
  oaRequest->setConsumerSecretKey(clientSecret);

  // oaManager->setHandleUserAuthorization(true);
  oaManager->executeRequest(oaRequest);
}

//------------------------------------------------------------------------------

void PumpApp::onTemporaryTokenReceived(QString /*token*/,
                                       QString /*tokenSecret*/) {
  QUrl userAuthURL(m_siteUrl+"/oauth/authorize");
  if (oaManager->lastError() == KQOAuthManager::NoError) {
    oaManager->getUserAuthorization(userAuthURL);
    emit userAuthorizationStarted();
  }
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizationReceived(QString token,
                                      QString verifier) {
  oaManager->verifyToken(token, verifier);
  oaManager->getUserAccessTokens(QUrl(m_siteUrl+"/oauth/access_token"));
  show();
}

//------------------------------------------------------------------------------

void PumpApp::onAccessTokenReceived(QString token, QString tokenSecret) {
    this->token = token;
    this->tokenSecret = tokenSecret;
    writeSettings();

    notifyMessage("User authorised for [" + m_siteUrl + "]");
    syncOAuthInfo();
    startPumping();
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestReady(QByteArray response, int id) {
  if (oaManager->lastError()) {
    errorMessage(QString("Network or authorisation error [id=%1].").
                 arg(oaManager->lastError()));
    return;
  }

  if (id == QAS_INBOX_MAJOR) {
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    inboxWidget->addCollection(c);
  } else if (id == QAS_INBOX_MINOR) {
  //   QVariantMap obj = parseJson(response);
  //   QASCollection c(obj, this);
  //   for (size_t i=0; i<3 && i<c.size(); i++) {
  //     QASActivity* act = c.at(i);
  //     qDebug() << "MINOR" << act->content();
  //   }
  } else if (id == (QAS_INBOX_DIRECT_MAJOR)) {
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    directMajorWidget->addCollection(c);
    // for (size_t i=0; i<3 && i<c.size(); i++) {
    //   QASActivity* act = c.at(i);
    //   qDebug() << "MAJOR DIRECT" << act->content();
    // }
  } else if (id == (QAS_INBOX_DIRECT_MINOR)) {
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    directMinorWidget->addCollection(c);
    // for (size_t i=0; i<3 && i<c.size(); i++) {
    //   QASActivity* act = c.at(i);
    //   qDebug() << "MINOR DIRECT" << act->content();
    // }
  } else if (id == QAS_REPLIES) {
    QVariantMap obj = parseJson(response);
    QASObjectList::getObjectList(obj, this);
  } else if (id == QAS_NEW_POST) {
    // nice to refresh inbox after posting
    fetchAll();
  } else {
    qDebug() << "[WARNING] Unknown request id!" << id;
  }
  notifyMessage("Ready!");
}

//------------------------------------------------------------------------------

void PumpApp::fetchAll() {
  notifyMessage("Loading ...");
  fetchInbox(QAS_INBOX_MAJOR);
  fetchInbox(QAS_INBOX_MINOR);
  fetchInbox(QAS_INBOX_DIRECT_MAJOR);
  fetchInbox(QAS_INBOX_DIRECT_MINOR);
}

//------------------------------------------------------------------------------

void PumpApp::fetchInbox(int reqType) {
  QString endpoint = "api/user/"+m_userName+"/inbox";

  if (reqType == QAS_INBOX_DIRECT_MAJOR || reqType == QAS_INBOX_DIRECT_MINOR)
    endpoint += "/direct";

  if (reqType == QAS_INBOX_MAJOR || reqType == QAS_INBOX_DIRECT_MAJOR)
    endpoint += "/major";
  else if (reqType == QAS_INBOX_MINOR || reqType == QAS_INBOX_DIRECT_MINOR) 
    endpoint += "/minor";
  else {
    qDebug() << "fetchInbox: unsupported request type:" << reqType;
    return;
  }

  request(endpoint, reqType);
}

//------------------------------------------------------------------------------

void PumpApp::postNote(QString note) {
  if (note.isEmpty())
    return;

  note.replace("<", "&lt;");
  note.replace(">", "&gt;");
  note.replace("\n", "<br/>");

  QVariantMap obj;
  obj["objectType"] = "note";
  obj["content"] = note;

  feed("post", obj, QAS_NEW_POST);
}

//------------------------------------------------------------------------------

void PumpApp::postReply(QASObject* replyToObj, QString content) {
  content.replace("<", "&lt;");
  content.replace(">", "&gt;");

  QVariantMap obj;
  obj["objectType"] = "comment";
  obj["content"] = content;

  QVariantMap noteObj;
  noteObj["id"] = replyToObj->id();
  noteObj["objectType"] = replyToObj->type();
  obj["inReplyTo"] = noteObj;

  feed("post", obj, QAS_NEW_POST);
}

//------------------------------------------------------------------------------

void PumpApp::feed(QString verb, QVariantMap object, int response_id) {
  QString endpoint = "api/user/"+m_userName+"/feed";

  QVariantMap data;
  if (!verb.isEmpty()) {
    data["verb"] = verb;
    data["object"] = object;
  }

  request(endpoint, response_id, KQOAuthRequest::POST, data);
}

//------------------------------------------------------------------------------

void PumpApp::request(QString endpoint, int response_id,
                      KQOAuthRequest::RequestHttpMethod method,
                      QVariantMap data) {
  if (!endpoint.startsWith("http")) {
    if (endpoint[0] != '/')
      endpoint = '/' + endpoint;
    endpoint = m_siteUrl + endpoint;
  }

  qDebug() << "[REQUEST] (" << response_id << "):" << endpoint;

  oaRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(endpoint));
  oaRequest->setConsumerKey(clientId);
  oaRequest->setConsumerSecretKey(clientSecret);

  oaRequest->setToken(token);
  oaRequest->setTokenSecret(tokenSecret);

  oaRequest->setHttpMethod(method); 

  if (method == KQOAuthRequest::POST) {
    oaRequest->setContentType("application/json");
    oaRequest->setRawData(serializeJson(data));
  }

  oaManager->executeAuthorizedRequest(oaRequest, response_id);
}

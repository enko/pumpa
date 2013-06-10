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

#include <QStatusBar>

#include "pumpapp.h"
#include "json.h"
#include "messagewindow.h"
#include "util.h"

//------------------------------------------------------------------------------

PumpApp::PumpApp(QWidget* parent) : 
  QMainWindow(parent),
  m_wiz(NULL),
  m_requests(0)
{
  settings = new QSettings(CLIENT_NAME, CLIENT_NAME, this);
  readSettings();
  
  oaRequest = new KQOAuthRequest(this);
  oaManager = new KQOAuthManager(this);
  connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray, int)),
          this, SLOT(onAuthorizedRequestReady(QByteArray, int)));

  createActions();
  createMenu();

  inboxWidget = new CollectionWidget(this);
  connectCollection(inboxWidget);

  inboxMinorWidget = new CollectionWidget(this, true);
  connectCollection(inboxMinorWidget);

  directMajorWidget = new CollectionWidget(this);
  connectCollection(directMajorWidget);

  directMinorWidget = new CollectionWidget(this);
  connectCollection(directMinorWidget);

  tabWidget = new TabWidget(this);
  connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabSelected(int)));
  tabWidget->addTab(inboxWidget, "&inbox");
  tabWidget->addTab(directMinorWidget, "&mentions");
  tabWidget->addTab(directMajorWidget, "&direct");
  tabWidget->addTab(inboxMinorWidget, "mean&while");

  setWindowTitle(CLIENT_FANCY_NAME);
  setWindowIcon(QIcon(":/images/pumpa.png"));
  setCentralWidget(tabWidget);

  // oaRequest->setEnableDebugOutput(true);
  syncOAuthInfo();

  timerId = -1;

  if (!haveOAuth()) {
    m_wiz = new OAuthWizard(this);
    connect(m_wiz, SIGNAL(clientRegistered(QString, QString, QString, QString)),
            this, SLOT(onClientRegistered(QString, QString, QString, QString)));
    connect(m_wiz, SIGNAL(accessTokenReceived(QString, QString)),
            this, SLOT(onAccessTokenReceived(QString, QString)));
    connect(m_wiz, SIGNAL(rejected()), this, SLOT(exit()));
    connect(m_wiz, SIGNAL(accepted()), this, SLOT(show()));
    m_wiz->show();
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
  request("/api/user/"+m_userName, QAS_FETCH_SELF);
  fetchAll();
  resetTimer();
}

//------------------------------------------------------------------------------

void PumpApp::connectCollection(CollectionWidget* w) {
  connect(w, SIGNAL(request(QString, int)), this, SLOT(request(QString, int)));
  connect(w, SIGNAL(newReply(QASObject*)), this, SLOT(newNote(QASObject*)));
  connect(w, SIGNAL(linkHovered(const QString&)),
          this, SLOT(statusMessage(const QString&)));
  connect(w, SIGNAL(like(QASObject*)), this, SLOT(onLike(QASObject*)));
  connect(w, SIGNAL(share(QASObject*)), this, SLOT(onShare(QASObject*)));
}

//------------------------------------------------------------------------------

void PumpApp::onClientRegistered(QString userName, QString siteUrl,
                                 QString clientId, QString clientSecret) {
  m_userName = userName;
  m_siteUrl = siteUrl;
  m_clientId = clientId;
  m_clientSecret = clientSecret;

  writeSettings();
}

//------------------------------------------------------------------------------

void PumpApp::onAccessTokenReceived(QString token, QString tokenSecret) {
  m_token = token;
  m_tokenSecret = tokenSecret;
  syncOAuthInfo();

  writeSettings();

  startPumping();
}

//------------------------------------------------------------------------------

bool PumpApp::haveOAuth() {
  return !m_clientId.isEmpty() && !m_clientSecret.isEmpty() &&
    !m_token.isEmpty() && !m_tokenSecret.isEmpty();
}

//------------------------------------------------------------------------------

void PumpApp::tabSelected(int index) {
  tabWidget->deHighlightTab(index);
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
  FileDownloader::setOAuthInfo(m_siteUrl, m_clientId, m_clientSecret,
                               m_token, m_tokenSecret);
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
    "</p>"
    "<p>The Pumpa logo was "
    "<a href=\"http://opengameart.org/content/fruit-and-veggie-inventory\">"
    "created by Joshua Taylor</a> for the "
    "<a href=\"http://lpc.opengameart.org/\">Liberated Pixel Cup</a>."
    "The logo is copyrighted by the artist and is dual licensed under the "
    "CC-BY-SA 3.0 license and the GNU GPL 3.0.";

  QMessageBox::about(this, tr("About " CLIENT_FANCY_NAME), 
                     "<p><b>" CLIENT_FANCY_NAME " " CLIENT_VERSION "</b> - "
                     "A simple Qt-based pump.io client.</p>"
                     "<p>Copyright &copy; 2013 Mats Sj&ouml;berg.</p>"+GPL);
}

//------------------------------------------------------------------------------

void PumpApp::newNote(QASObject* obj) {
  QASObject* irtObj = obj ? obj->inReplyTo() : NULL;
  if (irtObj) {
    obj = irtObj;
    qDebug() << "[DEBUG] Opening reply window to" << obj->type() << obj->id();
  }

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
  s.setValue("oauth_client_id", m_clientId);
  s.setValue("oauth_client_secret", m_clientSecret);
  s.setValue("oauth_token", m_token);
  s.setValue("oauth_token_secret", m_tokenSecret);
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
  m_clientId = s.value("oauth_client_id", "").toString();
  m_clientSecret = s.value("oauth_client_secret", "").toString();
  m_token = s.value("oauth_token", "").toString();
  m_tokenSecret = s.value("oauth_token_secret", "").toString();
  s.endGroup();
}

//------------------------------------------------------------------------------

void PumpApp::fetchAll() {
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

void PumpApp::onLike(QASObject* obj) {
  feed(obj->liked() ? "unlike" : "like", obj->toJson(), QAS_LIKE);
}

//------------------------------------------------------------------------------

void PumpApp::onShare(QASObject* obj) {
  feed("share", obj->toJson(), QAS_SHARE);
}

//------------------------------------------------------------------------------

QString PumpApp::addTextMarkup(QString text) {
  QString oldText = text;

  text.replace("<", "&lt;");
  text.replace(">", "&gt;");

  text = linkifyUrls(text);
  text.replace("\n", "<br/>");
  
  text = changePairedTags(text, "\\*\\*", "\\*\\*",
                             "<strong>", "</strong>");
  text = changePairedTags(text, "\\*", "\\*", "<em>", "</em>");

  text = changePairedTags(text, "__", "__", "<strong>", "</strong>");
  text = changePairedTags(text, "_", "_", "<em>", "</em>");

  text = changePairedTags(text, "``", "``", "<pre>", "</pre>");
  text = changePairedTags(text, "`", "`", "<code>", "</code>");

  qDebug() << "[DEBUG]: addTextMarkup:" << oldText << "=>" << text;
  
  return text;
}

//------------------------------------------------------------------------------

void PumpApp::postNote(QString content) {
  if (content.isEmpty())
    return;

  QVariantMap obj;
  obj["objectType"] = "note";
  obj["content"] = addTextMarkup(content);

  feed("post", obj, QAS_NEW_POST);
}

//------------------------------------------------------------------------------

void PumpApp::postReply(QASObject* replyToObj, QString content) {
  if (content.isEmpty())
    return;

  QVariantMap obj;
  obj["objectType"] = "comment";
  obj["content"] = addTextMarkup(content);

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

  if (!endpoint.startsWith(m_siteUrl)) {
    qDebug() << "[DEBUG] dropping request for" << endpoint;
    return;
  }

  qDebug() << "[REQUEST] (" << response_id << "):" << endpoint;

  oaRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(endpoint));
  oaRequest->setConsumerKey(m_clientId);
  oaRequest->setConsumerSecretKey(m_clientSecret);

  oaRequest->setToken(m_token);
  oaRequest->setTokenSecret(m_tokenSecret);

  oaRequest->setHttpMethod(method); 

  if (method == KQOAuthRequest::POST) {
    oaRequest->setContentType("application/json");
    oaRequest->setRawData(serializeJson(data));
  }

  oaManager->executeAuthorizedRequest(oaRequest, response_id);
  
  notifyMessage("Loading ...");
  m_requests++;
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestReady(QByteArray response, int id) {
  m_requests--;
  if (!m_requests) 
    notifyMessage("Ready!");

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
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    inboxMinorWidget->addCollection(c);
  } else if (id == (QAS_INBOX_DIRECT_MAJOR)) {
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    directMajorWidget->addCollection(c);
  } else if (id == (QAS_INBOX_DIRECT_MINOR)) {
    QVariantMap obj = parseJson(response);
    QASCollection c(obj, this);
    directMinorWidget->addCollection(c);
  } else if (id == QAS_REPLIES) {
    QVariantMap obj = parseJson(response);
    QASObjectList::getObjectList(obj, this);
  } else if (id == QAS_NEW_POST) {
    // nice to refresh inbox after posting
    fetchAll();
  } else if (id == QAS_LIKE) {
    QVariantMap act = parseJson(response);

    // refresh object since fetchAll() might not fetch it if it's old
    // enough
    QASObject* obj = QASObject::getObject(act["object"].toMap(), this);
    request(obj->apiLink(), QAS_OBJECT);
    fetchAll();
  } else if (id == QAS_OBJECT) {
    QVariantMap obj = parseJson(response);
    QASObject::getObject(obj, this);
  } else if (id == QAS_SHARE) {
    fetchAll();
  } else if (id == QAS_FETCH_SELF) {
    QVariantMap obj = parseJson(response);
    m_selfActor = QASActor::getActor(obj["profile"].toMap(), this);
    m_selfActor->setYou();
  } else {
    qDebug() << "[WARNING] Unknown request id!" << id;
    qDebug() << response;
  }
}


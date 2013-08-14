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
#include <QPalette>
#include <QInputDialog>
#include <QLineEdit>
#include <QClipboard>

#include "pumpapp.h"

#include "json.h"
#include "util.h"
#include "filedownloader.h"

//------------------------------------------------------------------------------

PumpApp::PumpApp(QString settingsFile, QWidget* parent) : 
  QMainWindow(parent),
  m_nextRequestId(0),
  m_contextWidget(NULL),
  m_isLoading(false),
  m_wiz(NULL),
  m_messageWindow(NULL),
  m_trayIcon(NULL),
  m_uploadDialog(NULL)
{
  m_s = new PumpaSettings(settingsFile, this);
  resize(m_s->size());
  move(m_s->pos());

  m_settingsDialog = new PumpaSettingsDialog(m_s, this);
  connect(m_settingsDialog, SIGNAL(newAccount()),
          this, SLOT(launchOAuthWizard()));

  QString linkColorStr = m_s->linkColor();
  if (!linkColorStr.isEmpty()) {
    QColor linkColor(linkColorStr);
    if (linkColor.isValid()) {
      QPalette pal(qApp->palette());
      pal.setColor(QPalette::Link, linkColor);
      pal.setColor(QPalette::LinkVisited, linkColor);
      qApp->setPalette(pal);
    } else {
      qDebug() << "[ERROR] cannot parse link_color \"" + linkColorStr + "\"";
    }
  }

  m_nam = new QNetworkAccessManager(this);

  oaManager = new KQOAuthManager(this);
  connect(oaManager, SIGNAL(authorizedRequestReady(QByteArray, int)),
          this, SLOT(onAuthorizedRequestReady(QByteArray, int)));

  createActions();
  createMenu();

#ifdef USE_DBUS
  m_dbus = new QDBusInterface("org.freedesktop.Notifications",
                              "/org/freedesktop/Notifications",
                              "org.freedesktop.Notifications");
  if (!m_dbus->isValid()) {
    qDebug() << "Unable to to connect to org.freedesktop.Notifications "
      "dbus service.";
    m_dbus = NULL;
  }
#endif

  updateTrayIcon();
  connect(m_s, SIGNAL(trayIconChanged()), this, SLOT(updateTrayIcon()));

  m_notifyMap = new QSignalMapper(this);

  int max_tl = m_s->maxTimelineItems();
  int max_fh = m_s->maxFirehoseItems();

  m_tabWidget = new TabWidget(this);

  m_inboxWidget = new CollectionWidget(this, max_tl);
  connectCollection(m_inboxWidget);

  m_inboxMinorWidget = new CollectionWidget(this, max_tl);
  connectCollection(m_inboxMinorWidget);

  m_directMajorWidget = new CollectionWidget(this, max_tl);
  connectCollection(m_directMajorWidget);

  m_directMinorWidget = new CollectionWidget(this, max_tl);
  connectCollection(m_directMinorWidget);

  m_favouritesWidget = new ObjectListWidget(m_tabWidget);
  connectCollection(m_favouritesWidget);
  m_favouritesWidget->hide();

  m_followersWidget = new ObjectListWidget(m_tabWidget);
  connectCollection(m_followersWidget);
  m_followersWidget->hide();

  m_followingWidget = new ObjectListWidget(m_tabWidget);
  connectCollection(m_followingWidget, false);
  m_followingWidget->hide();

  m_userActivitiesWidget = new CollectionWidget(this, max_tl);
  connectCollection(m_userActivitiesWidget, false);
  m_userActivitiesWidget->hide();

  m_firehoseWidget = new CollectionWidget(this, max_fh, 0);
  connectCollection(m_firehoseWidget);

  connect(m_tabWidget, SIGNAL(currentChanged(int)),
          this, SLOT(tabSelected(int)));

  m_tabWidget->addTab(m_inboxWidget, tr("&Inbox"));
  m_tabWidget->addTab(m_directMinorWidget, tr("&Mentions"));
  m_tabWidget->addTab(m_directMajorWidget, tr("&Direct"));
  m_tabWidget->addTab(m_inboxMinorWidget, tr("Mean&while"));
  m_tabWidget->addTab(m_firehoseWidget, tr("Fi&rehose"));

  m_notifyMap->setMapping(m_inboxWidget, FEED_INBOX);
  m_notifyMap->setMapping(m_directMinorWidget, FEED_MENTIONS);
  m_notifyMap->setMapping(m_directMajorWidget, FEED_DIRECT);
  m_notifyMap->setMapping(m_inboxMinorWidget, FEED_MEANWHILE);

  connect(m_notifyMap, SIGNAL(mapped(int)),
          this, SLOT(timelineHighlighted(int)));

  m_loadIcon = new QLabel(this);
  m_loadMovie = new QMovie(":/images/loader.gif");
  statusBar()->addPermanentWidget(m_loadIcon);

  setWindowTitle(CLIENT_FANCY_NAME);
  setWindowIcon(QIcon(CLIENT_ICON));
  setCentralWidget(m_tabWidget);

  // oaRequest->setEnableDebugOutput(true);
  syncOAuthInfo();

  m_timerId = -1;

  if (!haveOAuth())
    launchOAuthWizard();
  else
    startPumping();
}

//------------------------------------------------------------------------------

PumpApp::~PumpApp() {
  m_s->size(size());
  m_s->pos(pos());
}

//------------------------------------------------------------------------------

void PumpApp::launchOAuthWizard() {
  if (!m_wiz) {
    m_wiz = new OAuthWizard(m_nam, this);
    connect(m_wiz, SIGNAL(clientRegistered(QString, QString, QString, QString)),
            this, SLOT(onClientRegistered(QString, QString, QString, QString)));
    connect(m_wiz, SIGNAL(accessTokenReceived(QString, QString)),
            this, SLOT(onAccessTokenReceived(QString, QString)));
    connect(m_wiz, SIGNAL(rejected()), this, SLOT(exit()));
    connect(m_wiz, SIGNAL(accepted()), this, SLOT(show()));
  }
  m_wiz->restart();
  m_wiz->show();
}

//------------------------------------------------------------------------------

void PumpApp::startPumping() {
  resetActivityStreams();

  QString webFinger = siteUrlToAccountId(m_s->userName(), m_s->siteUrl());

  setWindowTitle(QString("%1 - %2").arg(CLIENT_FANCY_NAME).arg(webFinger));

  // Setup endpoints for our timeline widgets
  m_inboxWidget->setEndpoint(inboxEndpoint("major"), this, QAS_FOLLOW);
  m_inboxMinorWidget->setEndpoint(inboxEndpoint("minor"), this);
  m_directMajorWidget->setEndpoint(inboxEndpoint("direct/major"), this);
  m_directMinorWidget->setEndpoint(inboxEndpoint("direct/minor"), this);
  m_followersWidget->setEndpoint(apiUrl(apiUser("followers")), this);
  m_followingWidget->setEndpoint(apiUrl(apiUser("following")), this,
                                 QAS_FOLLOW);
  m_favouritesWidget->setEndpoint(apiUrl(apiUser("favorites")), this);
  m_firehoseWidget->setEndpoint(m_s->firehoseUrl(), this);
  m_userActivitiesWidget->setEndpoint(apiUrl(apiUser("feed")), this);
  show();

  request("/api/user/" + m_s->userName(), QAS_SELF_PROFILE);
  fetchAll(true);

  resetTimer();
}

//------------------------------------------------------------------------------

void PumpApp::connectCollection(ASWidget* w, bool highlight) {
  connect(w, SIGNAL(request(QString, int)), this, SLOT(request(QString, int)));
  connect(w, SIGNAL(newReply(QASObject*)), this, SLOT(newNote(QASObject*)));
  connect(w, SIGNAL(linkHovered(const QString&)),
          this, SLOT(statusMessage(const QString&)));
  connect(w, SIGNAL(like(QASObject*)), this, SLOT(onLike(QASObject*)));
  connect(w, SIGNAL(share(QASObject*)), this, SLOT(onShare(QASObject*)));
  if (highlight)
    connect(w, SIGNAL(highlightMe()), m_notifyMap, SLOT(map()));
  connect(w, SIGNAL(showContext(QASObject*)), 
          this, SLOT(onShowContext(QASObject*)));
  connect(w, SIGNAL(follow(QString, bool)), this, SLOT(follow(QString, bool)));
  connect(w, SIGNAL(deleteObject(QASObject*)),
          this, SLOT(onDeleteObject(QASObject*)));
}

//------------------------------------------------------------------------------

void PumpApp::onClientRegistered(QString userName, QString siteUrl,
                                 QString clientId, QString clientSecret) {
  m_s->userName(userName);
  m_s->siteUrl(siteUrl);
  m_s->clientId(clientId);
  m_s->clientSecret(clientSecret);
}

//------------------------------------------------------------------------------

void PumpApp::onAccessTokenReceived(QString token, QString tokenSecret) {
  m_s->token(token);
  m_s->tokenSecret(tokenSecret);
  syncOAuthInfo();

  startPumping();
}

//------------------------------------------------------------------------------

bool PumpApp::haveOAuth() {
  return !m_s->clientId().isEmpty() &&
    !m_s->clientSecret().isEmpty() &&
    !m_s->token().isEmpty() &&
    !m_s->tokenSecret().isEmpty();
}

//------------------------------------------------------------------------------

void PumpApp::tabSelected(int index) {
  m_tabWidget->deHighlightTab(index);
  resetNotifications();
}

//------------------------------------------------------------------------------

void PumpApp::timerEvent(QTimerEvent* event) {
  if (event->timerId() != m_timerId)
    return;
  m_timerCount++;

  if (m_timerCount >= m_s->reloadTime()) {
    m_timerCount = 0;
    fetchAll(false);
  }
  
  refreshTimeLabels();
}

//------------------------------------------------------------------------------

void PumpApp::resetTimer() {
  if (m_timerId != -1)
    killTimer(m_timerId);
  m_timerId = startTimer(60*1000); // one minute timer
  m_timerCount = 0;
}

//------------------------------------------------------------------------------

void PumpApp::debugAction() {
  checkMemory("debug");
  qDebug() << "inbox" << m_inboxWidget->count();
  qDebug() << "meanwhile" << m_inboxMinorWidget->count();
  qDebug() << "firehose" << m_firehoseWidget->count();
}

//------------------------------------------------------------------------------

void PumpApp::refreshTimeLabels() {
  m_inboxWidget->refreshTimeLabels();
  m_directMinorWidget->refreshTimeLabels();
  m_directMajorWidget->refreshTimeLabels();
  m_inboxMinorWidget->refreshTimeLabels();
  m_firehoseWidget->refreshTimeLabels();
  if (m_contextWidget)
    m_contextWidget->refreshTimeLabels();
}

//------------------------------------------------------------------------------

void PumpApp::syncOAuthInfo() {
  FileDownloader::setOAuthInfo(m_s->siteUrl(), m_s->clientId(),
                               m_s->clientSecret(),
                               m_s->token(), m_s->tokenSecret());
}

//------------------------------------------------------------------------------

void PumpApp::statusMessage(const QString& msg) {
  statusBar()->showMessage(msg);
}

//------------------------------------------------------------------------------

void PumpApp::notifyMessage(QString msg) {
  statusMessage(msg);
  // qDebug() << "[STATUS]:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::timelineHighlighted(int feed) {
  if ((feed & m_s->highlightFeeds()) && m_trayIcon)
    m_trayIcon->setIcon(QIcon(":/images/pumpa_glow.png"));

  if (feed & m_s->popupFeeds())
    sendNotification(CLIENT_FANCY_NAME, tr("You have new messages."));
}

//------------------------------------------------------------------------------

void PumpApp::resetNotifications() {
  if (m_trayIcon)
    m_trayIcon->setIcon(QIcon(CLIENT_ICON));
  m_tabWidget->deHighlightTab();
}

//------------------------------------------------------------------------------

bool PumpApp::sendNotification(QString summary, QString text) {
#ifdef USE_DBUS
  if (m_dbus && m_dbus->isValid()) {

    // https://developer.gnome.org/notification-spec/
    QList<QVariant> args;
    args.append(CLIENT_NAME);       // Application Name
    args.append(0123U);         // Replaces ID (0U)
    args.append(QString());     // Notification Icon
    args.append(summary);       // Summary
    args.append(text);          // Body
    args.append(QStringList()); // Actions

    QVariantMap hints;
    // for hints to make icon, see
    // https://dev.visucore.com/bitcoin/doxygen/notificator_8cpp_source.html
    args.append(hints);
    args.append(3000);

    m_dbus->callWithArgumentList(QDBus::NoBlock, "Notify", args);
    return true;
  }
#endif

  if (QSystemTrayIcon::supportsMessages() && m_trayIcon) {
    m_trayIcon->showMessage(CLIENT_FANCY_NAME, summary+" "+text);
    return true;
  }
  
  qDebug() << "[NOTIFY]" << summary << text;
  return false;
}

//------------------------------------------------------------------------------

void PumpApp::errorMessage(QString msg) {
  statusMessage(tr("Error: ") + msg);
  qDebug() << "[ERROR]:" << msg;
}

//------------------------------------------------------------------------------

void PumpApp::updateTrayIcon() {
  bool useTray = m_s->useTrayIcon() && QSystemTrayIcon::isSystemTrayAvailable();

  if (useTray) {
    qApp->setQuitOnLastWindowClosed(false);
    if (!m_trayIcon)
      createTrayIcon();
    else
      m_trayIcon->show();

    if (m_trayIcon) {
      QString toolTip = CLIENT_FANCY_NAME;
      if (!m_s->userName().isEmpty())
        toolTip += " - " + siteUrlToAccountId(m_s->userName(), m_s->siteUrl());
      m_trayIcon->setToolTip(toolTip);
    }
  } else {
    qApp->setQuitOnLastWindowClosed(true);
    if (m_trayIcon)
      m_trayIcon->hide();
  }
}

//------------------------------------------------------------------------------

void PumpApp::createTrayIcon() {
  m_trayIconMenu = new QMenu(this);
  m_trayIconMenu->addAction(newNoteAction);
  // m_trayIconMenu->addAction(newPictureAction);
  m_trayIconMenu->addSeparator();
  m_trayIconMenu->addAction(m_showHideAction);
  m_trayIconMenu->addAction(exitAction);
  
  m_trayIcon = new QSystemTrayIcon(QIcon(CLIENT_ICON));
  connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
  m_trayIcon->setContextMenu(m_trayIconMenu);
  m_trayIcon->setToolTip(CLIENT_FANCY_NAME);
  m_trayIcon->show();
}

//------------------------------------------------------------------------------

void PumpApp::trayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger) {
    m_trayIcon->setIcon(QIcon(CLIENT_ICON));
    toggleVisible();
  }
}

//------------------------------------------------------------------------------

QString PumpApp::showHideText(bool visible) {
  return QString(tr("%1 &Window")).arg(visible ? tr("Hide") : tr("Show") );
}

//------------------------------------------------------------------------------

void PumpApp::toggleVisible() {
  setVisible(!isVisible());
  m_showHideAction->setText(showHideText());
}

//------------------------------------------------------------------------------

void PumpApp::createActions() {
  exitAction = new QAction(tr("E&xit"), this);
  exitAction->setShortcut(tr("Ctrl+Q"));
  connect(exitAction, SIGNAL(triggered()), this, SLOT(exit()));

  openPrefsAction = new QAction(tr("Preferences"), this);
  connect(openPrefsAction, SIGNAL(triggered()), this, SLOT(preferences()));

  reloadAction = new QAction(tr("&Reload timeline"), this);
  reloadAction->setShortcut(tr("Ctrl+R"));
  connect(reloadAction, SIGNAL(triggered()), 
          this, SLOT(reload()));

  loadOlderAction = new QAction(tr("Load older in timeline"), this);
  loadOlderAction->setShortcut(tr("Ctrl+O"));
  connect(loadOlderAction, SIGNAL(triggered()), this, SLOT(loadOlder()));

  followAction = new QAction(tr("F&ollow an account"), this);
  followAction->setShortcut(tr("Ctrl+L"));
  connect(followAction, SIGNAL(triggered()), this, SLOT(followDialog()));

  aboutAction = new QAction(tr("&About"), this);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAction = new QAction(tr("About &Qt"), this);
  connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  newNoteAction = new QAction(tr("New &Note"), this);
  newNoteAction->setShortcut(tr("Ctrl+N"));
  connect(newNoteAction, SIGNAL(triggered()), this, SLOT(newNote()));

  m_debugAction = new QAction("Debug", this);
  m_debugAction->setShortcut(tr("Ctrl+D"));
  connect(m_debugAction, SIGNAL(triggered()), this, SLOT(debugAction()));
  addAction(m_debugAction);

  m_followersAction = new QAction(tr("Followers"), this);
  connect(m_followersAction, SIGNAL(triggered()), this, SLOT(showFollowers()));

  m_followingAction = new QAction(tr("Following"), this);
  connect(m_followingAction, SIGNAL(triggered()), this, SLOT(showFollowing()));

  m_favouritesAction = new QAction(tr("Favorites"), this);
  connect(m_favouritesAction, SIGNAL(triggered()),
          this, SLOT(showFavourites()));

  m_userActivitiesAction = new QAction(tr("Activities"), this);
  connect(m_userActivitiesAction, SIGNAL(triggered()),
          this, SLOT(showUserActivities()));

  m_showHideAction = new QAction(showHideText(true), this);
  connect(m_showHideAction, SIGNAL(triggered()), this, SLOT(toggleVisible()));
}

//------------------------------------------------------------------------------

void PumpApp::createMenu() {
  fileMenu = new QMenu(tr("&Pumpa"), this);
  fileMenu->addAction(newNoteAction);
  fileMenu->addSeparator();
  fileMenu->addAction(followAction);
  fileMenu->addAction(reloadAction);
  fileMenu->addAction(loadOlderAction);
  fileMenu->addSeparator();
  fileMenu->addAction(openPrefsAction);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAction);
  menuBar()->addMenu(fileMenu);

  m_tabsMenu = new QMenu(tr("&Tabs"), this);
  m_tabsMenu->addAction(m_userActivitiesAction);
  m_tabsMenu->addAction(m_favouritesAction);
  m_tabsMenu->addAction(m_followersAction);
  m_tabsMenu->addAction(m_followingAction);
  menuBar()->addMenu(m_tabsMenu);

  helpMenu = new QMenu(tr("&Help"), this);
  helpMenu->addAction(aboutAction);
  helpMenu->addAction(aboutQtAction);
  menuBar()->addMenu(helpMenu);
}

//------------------------------------------------------------------------------

void PumpApp::preferences() {
  m_settingsDialog->exec();
}

//------------------------------------------------------------------------------

void PumpApp::exit() { 
  qApp->exit();
}

//------------------------------------------------------------------------------

void PumpApp::about() { 
  static const QString GPL = 
    tr("<p>Pumpa is free software: you can redistribute it and/or modify it "
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
       "</p>");
  static const QString credits = 
    tr("<p>The <a href=\"https://github.com/kypeli/kQOAuth\">kQOAuth library"
       "</a> is copyrighted by <a href=\"http://www.johanpaul.com/\">Johan "
       "Paul</a> and licensed under LGPL 2.1.</p>"
       "<p>The <a href=\"https://github.com/vmg/sundown\">sundown Markdown "
       "library</a> is copyrighted by Natacha Port&eacute;, Vicent Marti and "
       "others, and <a href=\"https://github.com/vmg/sundown#license\">"
       "permissively licensed</a>.</p>"
       "<p>The Pumpa logo was "
       "<a href=\"http://opengameart.org/content/fruit-and-veggie-inventory\">"
       "created by Joshua Taylor</a> for the "
       "<a href=\"http://lpc.opengameart.org/\">Liberated Pixel Cup</a>."
       "The logo is copyrighted by the artist and is dual licensed under the "
       "CC-BY-SA 3.0 license and the GNU GPL 3.0.");
  
  QString mainText = 
    QString("<p><b>%1 %2</b> - %3<br/><a href=\"%4\">%4</a><br/>"
            + tr("Copyright &copy; 2013 Mats Sj&ouml;berg.</p>")
            + tr("<p>Report bugs and feature requests at "
                 "<a href=\"%5\">%5</a>.</p>"))
    .arg(CLIENT_FANCY_NAME)
    .arg(CLIENT_VERSION)
    .arg(tr("A simple Qt-based pump.io client."))
    .arg(WEBSITE_URL)
    .arg(BUGTRACKER_URL);
  
  QMessageBox::about(this, QString(tr("About %1")).arg(CLIENT_FANCY_NAME),
                     mainText + GPL + credits);
}

//------------------------------------------------------------------------------

void PumpApp::newNote(QASObject* obj) {
  QASObject* irtObj = obj ? obj->inReplyTo() : NULL;
  if (irtObj)
    obj = irtObj;

  if (!m_messageWindow) {
    m_messageWindow = new MessageWindow(m_s, this);
    connect(m_messageWindow, SIGNAL(sendMessage(QString, int, int)),
            this, SLOT(postNote(QString, int, int)));
    connect(m_messageWindow, 
            SIGNAL(sendImage(QString, QString, QString, int, int)),
            this, SLOT(postImage(QString, QString, QString, int, int)));
    connect(m_messageWindow, SIGNAL(sendReply(QASObject*, QString)),
            this, SLOT(postReply(QASObject*, QString)));
    m_messageWindow->setCompletions(&m_completions);
  }

  m_messageWindow->newMessage(obj);
  m_messageWindow->show();
}

//------------------------------------------------------------------------------

void PumpApp::reload() {
  fetchAll(true);
  refreshTimeLabels();
}

//------------------------------------------------------------------------------

void PumpApp::fetchAll(bool all) {
  m_inboxWidget->fetchNewer();
  m_directMinorWidget->fetchNewer();
  m_directMajorWidget->fetchNewer();
  m_inboxMinorWidget->fetchNewer();
  m_firehoseWidget->fetchNewer();

  if (all || m_followersWidget->isVisible())
    m_followersWidget->fetchNewer();
  if (all || m_followingWidget->isVisible())
    m_followingWidget->fetchNewer();
  if (all || m_favouritesWidget->isVisible())
    m_favouritesWidget->fetchNewer();
  if (all || m_userActivitiesWidget->isVisible())
    m_userActivitiesWidget->fetchNewer();
}

//------------------------------------------------------------------------------

void PumpApp::loadOlder() {
  CollectionWidget* cw = 
    qobject_cast<CollectionWidget*>(m_tabWidget->currentWidget());
  cw->fetchOlder();
}

//------------------------------------------------------------------------------

QString PumpApp::inboxEndpoint(QString path) {
  if (m_s->siteUrl().isEmpty()) {
    errorMessage(tr("Site not configured yet!"));
    return "";
  }
  return m_s->siteUrl() + "/api/user/" + m_s->userName() + "/inbox/" + path;
}

//------------------------------------------------------------------------------

void PumpApp::onLike(QASObject* obj) {
  feed(obj->liked() ? "unlike" : "like", obj->toJson(),
       QAS_ACTIVITY | QAS_TOGGLE_LIKE);
}

//------------------------------------------------------------------------------

void PumpApp::onShare(QASObject* obj) {
  feed("share", obj->toJson(), QAS_ACTIVITY | QAS_REFRESH);
}

//------------------------------------------------------------------------------

void PumpApp::errorBox(QString msg) {
  QMessageBox::critical(this, CLIENT_FANCY_NAME, msg, QMessageBox::Ok);
}

//------------------------------------------------------------------------------

bool PumpApp::webFingerFromString(QString text, QString& username,
                                  QString& server) {
  if (text.startsWith("https://") || text.startsWith("http://")) {
    int slashPos = text.lastIndexOf('/');
    if (slashPos > 0)
      text = siteUrlToAccountId(text.mid(slashPos+1), text.left(slashPos));
  }

  return splitWebfingerId(text, username, server);
}

//------------------------------------------------------------------------------

void PumpApp::followDialog() {
  bool ok;

  QString defaultText = "evan@e14n.com";
  QString cbText = QApplication::clipboard()->text();
  if (cbText.contains('@') || cbText.startsWith("https://") ||
      cbText.startsWith("http://")) 
    defaultText = cbText;

  QString text =
    QInputDialog::getText(this, tr("Follow pump.io user"),
                          tr("Enter webfinger ID of person to follow: "),
                          QLineEdit::Normal, defaultText, &ok);

  if (!ok || text.isEmpty())
    return;

  QString username, server;
  QString error;

  if (!webFingerFromString(text, username, server))
    error = tr("Sorry, that doesn't even look like a webfinger ID!");

  QASObject* obj = QASObject::getObject("acct:" + username + "@" + server);
  QASActor* actor = obj ? obj->asActor() : NULL;
  if (actor && actor->followed())
    error = tr("Sorry, you are already following that person!");

  if (!error.isEmpty())
    return errorBox(error);

  testUserAndFollow(username, server);
}

//------------------------------------------------------------------------------

void PumpApp::testUserAndFollow(QString username, QString server) {
  QString fingerUrl = QString("%1/.well-known/webfinger?resource=%2@%1").
    arg(server).arg(username);
  // https://io.saz.im/.well-known/webfinger?resource=sazius@saz.im

  QNetworkRequest rec(QUrl("http://" + fingerUrl));
  QNetworkReply* reply = m_nam->head(rec);
  connect(reply, SIGNAL(finished()), this, SLOT(userTestDoneAndFollow()));
  
  // isn't this an ugly yet fancy hack? :-)
  reply->setProperty("pumpa_redirects", 0);
}

//------------------------------------------------------------------------------

void PumpApp::userTestDoneAndFollow() {
  QString error;

  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
#ifdef QT5
  QUrlQuery replyQuery(reply->url().query());
  QString userId = replyQuery.queryItemValue("resource");
#else
  QString userId = reply->url().queryItemValue("resource");
#endif
  if (reply->error() != QNetworkReply::NoError)
    return errorBox(tr("Invalid user: ") + userId);

  int redirs = reply->property("pumpa_redirects").toInt();

  QUrl loc = reply->header(QNetworkRequest::LocationHeader).toUrl();
  if (loc.isValid()) {
    if (redirs > 5)
      return errorBox(tr("Invalid user (cannot check site): ") + userId);
    reply->deleteLater();
    
    QNetworkRequest rec(loc);
    QNetworkReply* r = m_nam->head(rec);
    r->setProperty("pumpa_redirects", ++redirs);
    connect(r, SIGNAL(finished()), this, SLOT(userTestDoneAndFollow()));
    return;
  }
  
  follow("acct:" + userId, true);
}

//------------------------------------------------------------------------------

void PumpApp::onShowContext(QASObject* obj) {
  if (!m_contextWidget) {
    m_contextWidget = new ContextWidget(this);
    connectCollection(m_contextWidget);
  }
  if (m_tabWidget->indexOf(m_contextWidget) == -1)
    m_tabWidget->addTab(m_contextWidget, tr("&Context"), true, true);

  m_contextWidget->setObject(obj);
  m_tabWidget->setCurrentWidget(m_contextWidget);
}

//------------------------------------------------------------------------------

void PumpApp::showFollowers() {
  if (m_tabWidget->indexOf(m_followersWidget) == -1)
    m_tabWidget->addTab(m_followersWidget, tr("&Followers"), true, true);
  m_tabWidget->setCurrentWidget(m_followersWidget);
  m_followersWidget->fetchNewer();
}

//------------------------------------------------------------------------------

void PumpApp::showFollowing() {
  if (m_tabWidget->indexOf(m_followingWidget) == -1)
    m_tabWidget->addTab(m_followingWidget, tr("F&ollowing"), false, true);
  m_tabWidget->setCurrentWidget(m_followingWidget);
  m_followingWidget->fetchNewer();
}

//------------------------------------------------------------------------------

void PumpApp::showFavourites() {
  if (m_tabWidget->indexOf(m_favouritesWidget) == -1)
    m_tabWidget->addTab(m_favouritesWidget, tr("F&avorites"), false, true);
  m_tabWidget->setCurrentWidget(m_favouritesWidget);
  m_favouritesWidget->fetchNewer();
}

//------------------------------------------------------------------------------

void PumpApp::showUserActivities() {
  if (m_tabWidget->indexOf(m_userActivitiesWidget) == -1)
    m_tabWidget->addTab(m_userActivitiesWidget, tr("A&ctivities"), false, true);
  m_tabWidget->setCurrentWidget(m_userActivitiesWidget);
  m_userActivitiesWidget->fetchNewer();
}

//------------------------------------------------------------------------------

void PumpApp::postNote(QString content, int to, int cc) {
  if (content.isEmpty())
    return;

  QVariantMap obj;
  obj["objectType"] = "note";
  obj["content"] = addTextMarkup(content);

  feed("post", obj, QAS_OBJECT | QAS_REFRESH | QAS_POST, to, cc);
}

//------------------------------------------------------------------------------

void PumpApp::postImage(QString msg,
                        QString title,
                        QString imageFile,
                        int to,
                        int cc) {
  m_imageObject.clear();
  m_imageObject["content"] = addTextMarkup(msg);
  m_imageObject["displayName"] = title;

  m_imageTo = to;
  m_imageCc = cc;

  uploadFile(imageFile);
}

//------------------------------------------------------------------------------

void PumpApp::uploadFile(QString filename) {
  QString lcfn = filename.toLower();
  QString mimeType;
  if (lcfn.endsWith(".jpg") || lcfn.endsWith(".jpeg"))
    mimeType = "image/jpeg";
  else if (lcfn.endsWith(".png"))
    mimeType = "image/png";
  else if (lcfn.endsWith(".gif"))
    mimeType = "image/gif";
  else {
    qDebug() << "Cannot determine mime type of file" << filename;
    return;
  }

  QFile fp(filename);
  if (!fp.open(QIODevice::ReadOnly)) {
    qDebug() << "Unable to read file" << filename;
      return;
  }

  QByteArray ba = fp.readAll();
  
  KQOAuthRequest* oaRequest = initRequest(apiUrl(apiUser("uploads")),
                                          KQOAuthRequest::POST);
  oaRequest->setContentType(mimeType);
  oaRequest->setContentLength(ba.size());
  oaRequest->setRawData(ba);

  if (m_uploadDialog == NULL) {
    m_uploadDialog = new QProgressDialog("Uploading image...", "Abort", 0, 100,
                                         this);
    m_uploadDialog->setWindowModality(Qt::WindowModal);
  }
  m_uploadDialog->setValue(0);
  m_uploadDialog->show();

  const QNetworkReply* nr = executeRequest(oaRequest, QAS_IMAGE_UPLOAD);
  connect(nr, SIGNAL(uploadProgress(qint64, qint64)),
          this, SLOT(uploadProgress(qint64, qint64)));
}

//------------------------------------------------------------------------------

void PumpApp::updatePostedImage(QVariantMap) {
  feed("update", m_imageObject, QAS_ACTIVITY | QAS_REFRESH | QAS_POST);
}

//------------------------------------------------------------------------------

void PumpApp::postImageActivity(QVariantMap obj) {
  m_imageObject.unite(obj);
  feed("post", m_imageObject, QAS_IMAGE_UPDATE, m_imageTo, m_imageCc);
}

//------------------------------------------------------------------------------

void PumpApp::uploadProgress(qint64 bytesSent, qint64 bytesTotal) {
  if (!m_uploadDialog || bytesTotal <= 0)
    return;

  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

  m_uploadDialog->setValue((100*bytesSent)/bytesTotal);
  if (m_uploadDialog->wasCanceled() && reply) {
    reply->abort();
    m_uploadDialog->hide();
  }
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

  feed("post", obj, QAS_ACTIVITY | QAS_REFRESH | QAS_POST);
}

//------------------------------------------------------------------------------

void PumpApp::follow(QString acctId, bool follow) {
  QVariantMap obj;
  obj["id"] = acctId;
  obj["objectType"] = "person";

  int mode = QAS_ACTIVITY;
  if (follow)
    mode |= QAS_FOLLOW;
  else
    mode |= QAS_UNFOLLOW;

  feed(follow ? "follow" : "stop-following", obj, mode);
}

//------------------------------------------------------------------------------

void PumpApp::onDeleteObject(QASObject* obj) {
  QVariantMap json;
  json["id"] = obj->id();
  json["objectType"] = obj->type();

  feed("delete", json, QAS_ACTIVITY);
}

//------------------------------------------------------------------------------

void PumpApp::addRecipient(QVariantMap& data, QString name, int to) {
  if (to == RECIPIENT_EMPTY)
    return;

  QVariantList recList;

  QVariantMap rec;
  rec["objectType"] = "collection";
  if (to == RECIPIENT_PUBLIC)
    rec["id"] = PUBLIC_RECIPIENT_ID;
  else if (to == RECIPIENT_FOLLOWERS)
    rec["id"] = apiUrl("api/user/" + m_s->userName() + "/followers");

  recList.append(rec);

  data[name] = recList;
}

//------------------------------------------------------------------------------

void PumpApp::feed(QString verb, QVariantMap object, int response_id,
                   int to, int cc) {
  QString endpoint = "api/user/" + m_s->userName() + "/feed";

  QVariantMap data;
  data["verb"] = verb;
  data["object"] = object;

  addRecipient(data, "to", to);
  addRecipient(data, "cc", cc);

  request(endpoint, response_id, KQOAuthRequest::POST, data);
}

//------------------------------------------------------------------------------

QString PumpApp::apiUrl(QString endpoint) {
  QString ret = endpoint;
  if (!ret.startsWith("http")) {
    if (ret[0] != '/')
      ret = '/' + ret;
    ret = m_s->siteUrl() + ret;
  }
  return ret;
}

//------------------------------------------------------------------------------

QString PumpApp::apiUser(QString path) {
  return QString("api/user/%1/%2").arg(m_s->userName()).arg(path);
}

//------------------------------------------------------------------------------

KQOAuthRequest* PumpApp::initRequest(QString endpoint,
                                     KQOAuthRequest::RequestHttpMethod method) {
  KQOAuthRequest* oaRequest = new KQOAuthRequest(this);
  oaRequest->initRequest(KQOAuthRequest::AuthorizedRequest, QUrl(endpoint));
  oaRequest->setConsumerKey(m_s->clientId());
  oaRequest->setConsumerSecretKey(m_s->clientSecret());
  oaRequest->setToken(m_s->token());
  oaRequest->setTokenSecret(m_s->tokenSecret());
  oaRequest->setHttpMethod(method); 
  oaRequest->setTimeout(60000); // one minute time-out
  return oaRequest;
}

//------------------------------------------------------------------------------

void PumpApp::request(QString endpoint, int response_id,
                      KQOAuthRequest::RequestHttpMethod method,
                      QVariantMap data) {
  endpoint = apiUrl(endpoint);

  bool firehose = (endpoint == m_s->firehoseUrl());
  if (!endpoint.startsWith(m_s->siteUrl()) && !firehose) {
#ifdef DEBUG_NET
    qDebug() << "[DEBUG] dropping request for" << endpoint;
#endif
    return;
  }

#ifdef DEBUG_NET
  qDebug() << (method == KQOAuthRequest::GET ? "[GET]" : "[POST]") 
           << response_id << ":" << endpoint;
#endif

  QStringList epl = endpoint.split("?");
  KQOAuthRequest* oaRequest = initRequest(epl[0], method);

  // I have no idea why this is the only way that seems to
  // work. Incredibly frustrating and ugly :-/
  if (epl.size() > 1) {
    KQOAuthParameters params;
    QStringList parts = epl[1].split("&");
    for (int i=0; i<parts.size(); i++) {
      QStringList ps = parts[i].split("=");
      params.insert(ps[0], QUrl::fromPercentEncoding(ps[1].toLatin1()));
    }
    oaRequest->setAdditionalParameters(params);
  }
  
  if (method == KQOAuthRequest::POST) {
    QByteArray ba = serializeJson(data);
    oaRequest->setRawData(ba);
    oaRequest->setContentType("application/json");
    oaRequest->setContentLength(ba.size());
#ifdef DEBUG_NET
    qDebug() << "DATA" << oaRequest->rawData();
#endif
  }

  executeRequest(oaRequest, response_id);

  setLoading(true);
  notifyMessage(tr("Loading ..."));
}

//------------------------------------------------------------------------------

QNetworkReply* PumpApp::executeRequest(KQOAuthRequest* request,
                                       int response_id) {
  int id = m_nextRequestId++;

  if (m_nextRequestId > 32000) { // bound to be smaller than any MAX_INT
    m_nextRequestId = 0;
    while (m_requestMap.contains(m_nextRequestId))
      m_nextRequestId++;
  }

  m_requestMap.insert(id, qMakePair(request, response_id));
  oaManager->executeAuthorizedRequest(request, id);

  return oaManager->getReply(request);
}

//------------------------------------------------------------------------------

void PumpApp::followActor(QASActor* actor, bool doFollow) {
  actor->setFollowed(doFollow);

  QString dn = actor->displayName();
  QString un = actor->webFinger();
  QString from = QString("%2 (%1)").arg(dn).arg(un);
  QString md = QString("[%1](%2)").arg(dn).arg(actor->url());

  addCompletion(from, md, doFollow);
}

//------------------------------------------------------------------------------

void PumpApp::addCompletion(QString from, QString to, bool add) {
  if (from.isEmpty() || from.startsWith("http://") ||
      from.startsWith("https://"))
    return;

  if (add) {
    // if (!m_completions.contains(from))
    //   qDebug() << "addCompletion" << from << to << add;
    m_completions.insert(from, to);
  } else {
    m_completions.remove(from);
  }
}

//------------------------------------------------------------------------------

void PumpApp::onAuthorizedRequestReady(QByteArray response, int rid) {
  KQOAuthManager::KQOAuthError lastError = oaManager->lastError();

  QPair<KQOAuthRequest*, int> rp = m_requestMap.take(rid);
  KQOAuthRequest* request = rp.first;
  int id = rp.second;
  QString reqUrl = request->requestEndpoint().toString();

#ifdef DEBUG_NET
  qDebug() << "[DEBUG] request done [" << rid << id << "]" << reqUrl
           << response.count() << "bytes";
#endif
#ifdef DEBUG_NET_MOAR
  qDebug() << response;
#endif

  // FIXME: fugly quick "fix" - proper fix comming later
  // if (!lastError)
  //   request->deleteLater();

  if (m_requestMap.isEmpty()) {
    setLoading(false);
    notifyMessage(tr("Ready!"));
  } 
#ifdef DEBUG_NET_MOAR
  else {
    qDebug() << "[DEBUG] Still waiting for requests:";
    QMapIterator<int, requestInfo_t> i(m_requestMap);
    while (i.hasNext()) {
      i.next();
      requestInfo_t ri = i.value();
      qDebug() << "   " << ri.first->requestEndpoint() << ri.second;
    }    
  }
#endif

  int sid = id & 0xFF;

  if (lastError) {
    if (id & QAS_POST) {
      errorMessage(tr("Unable to post message!"));
      m_messageWindow->show();
    } else if (sid == QAS_OBJECT) {
      qDebug() << "[WARNING] unable to fetch context for object.";
    } else {
      errorMessage(QString(tr("Network or authorisation error [%1/%2] %3.")).
                   arg(oaManager->lastError()).arg(id).arg(reqUrl));
    }
#ifdef DEBUG_NET
    qDebug() << "[ERROR]" << response;
#endif
    return;
  }

  QVariantMap json = parseJson(response);
  if (sid == QAS_NULL)
    return;

  if (sid == QAS_COLLECTION) {
    QASCollection* coll = QASCollection::getCollection(json, this, id);
    if (coll) {
      bool checkFollows = (id & QAS_FOLLOW);

      for (size_t i=0; i<coll->size(); ++i) {
        QASActivity* activity = coll->at(i);
        QASObject* obj = activity->object();

        if (obj) {
          QASObject* irtObj = obj->inReplyTo();
          if (irtObj && irtObj->url().isEmpty())
            refreshObject(irtObj);
        }

        if (checkFollows) {
          QASActor* actor = activity->actor();
          if (activity->verb() == "post" && actor &&
              actor->followedJson() && !actor->followed()) 
            followActor(actor);
        }
      }
    }
  } else if (sid == QAS_ACTIVITY) {
    QASActivity* act = QASActivity::getActivity(json, this);
    QASObject* obj = act->object();

    if ((id & QAS_TOGGLE_LIKE) && obj)
      obj->toggleLiked();

    if ((id & QAS_FOLLOW) || (id & QAS_UNFOLLOW)) {
      QASActor* actor = obj ? obj->asActor() : NULL;
      if (actor) {
        bool doFollow = (id & QAS_FOLLOW);
        followActor(actor, doFollow);
        notifyMessage(QString(doFollow ? tr("Successfully followed ") :
                              tr("Successfully unfollowed ")) +
                      actor->displayNameOrWebFinger());
      }
    }
  } else if (sid == QAS_OBJECTLIST) {
    QASObjectList* ol = QASObjectList::getObjectList(json, this, id);
    if (ol && (id & QAS_FOLLOW)) {
      for (size_t i=0; i<ol->size(); ++i) {
        QASActor* actor = ol->at(i)->asActor();
        if (actor)
          followActor(actor);
      }
    }
  } else if (sid == QAS_OBJECT) {
    QASObject::getObject(json, this);
  } else if (sid == QAS_ACTORLIST) {
    QASActorList::getActorList(json, this);
  } else if (sid == QAS_SELF_PROFILE) {
    m_selfActor = QASActor::getActor(json["profile"].toMap(), this);
    m_selfActor->setYou();
  } else if (sid == QAS_IMAGE_UPLOAD) {
    m_uploadDialog->hide();
    postImageActivity(json);
  } else if (sid == QAS_IMAGE_UPDATE) {
    updatePostedImage(json);
  }

  if ((id & QAS_POST) && m_messageWindow)
    m_messageWindow->clear();

  if (id & QAS_REFRESH) { 
    fetchAll(false);
  }
}

//------------------------------------------------------------------------------
// FIXME: this shouldn't be implemented in millions of places

void PumpApp::refreshObject(QASAbstractObject* obj) {
  if (!obj)
    return;
  
  QDateTime now = QDateTime::currentDateTime();
  QDateTime lr = obj->lastRefreshed();

  if (lr.isNull() || lr.secsTo(now) > 10) {
    obj->lastRefreshed(now);
    request(obj->apiLink(), obj->asType());
  }
}
 
//------------------------------------------------------------------------------

void PumpApp::setLoading(bool on) {
  if (!m_loadIcon || m_isLoading == on)
    return;

  m_isLoading = on;

  if (!on) {
    m_loadIcon->setMovie(NULL);
    m_loadIcon->setPixmap(QPixmap(":/images/empty.gif"));
  } else if (m_loadMovie->isValid()) {
    // m_loadIcon->setPixmap(QPixmap());
    m_loadIcon->setMovie(m_loadMovie);
    m_loadMovie->start();
  }
}

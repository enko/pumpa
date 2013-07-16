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
#include <QDebug>
#include <QDesktopServices>
#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSystemTrayIcon>

#ifdef USE_DBUS
#include <QDBusInterface>
#endif

#include "QtKOAuth"

#include "pumpa_defines.h"
#include "qactivitystreams.h"
#include "collectionwidget.h"
#include "oauthwizard.h"
#include "tabwidget.h"
#include "pumpasettingsdialog.h"
#include "pumpasettings.h"
#include "contextwidget.h"
#include "objectlistwidget.h"
#include "messagewindow.h"

//------------------------------------------------------------------------------

class PumpApp : public QMainWindow {
  Q_OBJECT

public:
  PumpApp(QString settingsFile="", QWidget* parent=0);
  virtual ~PumpApp();                            

  static QString addTextMarkup(QString content);

signals:
  void userAuthorizationStarted();
                    
private slots:
  void userTestDoneAndFollow();

  void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void updateTrayIcon();
  void toggleVisible();
  void timelineHighlighted(int);

  void followDialog();
  void onLike(QASObject* obj);
  void onShare(QASObject* obj);
  void postNote(QString note, int to, int cc);
  void postReply(QASObject* replyToObj, QString content);
  void follow(QString acctId, bool follow);
  void errorMessage(QString msg);
  void notifyMessage(QString msg);
  void statusMessage(const QString& msg);
  void onShowContext(QASObject*);
  void tabSelected(int index);

  void onClientRegistered(QString, QString, QString, QString);
  void onAccessTokenReceived(QString token, QString tokenSecret);

  void onAuthorizedRequestReady(QByteArray response, int id);
  
  void request(QString endpoint, int response_id,
               KQOAuthRequest::RequestHttpMethod method = KQOAuthRequest::GET,
               QVariantMap data=QVariantMap());

  void exit();
  void about();
  void preferences();
  void newNote(QASObject* obj = NULL);
  void newPicture();
  void reload();
  void loadOlder();

  void startPumping();

  void launchOAuthWizard();

protected:
  void timerEvent(QTimerEvent*);
  virtual bool event(QEvent* e) {
    if (e->type() == QEvent::WindowActivate)
      resetNotifications();
    return QMainWindow::event(e);
  }
  void closeEvent(QCloseEvent* e) {
    m_showHideAction->setText(showHideText(false));
    QMainWindow::closeEvent(e);
  }

private:
  void errorBox(QString msg);

  void testUserAndFollow(QString username, QString server);

  QString apiUrl(QString endpoint);

  // constructs api/user/$username/$path
  QString apiUser(QString path);

  void addRecipient(QVariantMap& data, QString name, int to);

  void resetNotifications();

  void createTrayIcon();
  QString showHideText(bool);
  QString showHideText() { return showHideText(isVisible()); }

  void connectCollection(ASWidget* w, bool highlight=true);

  bool haveOAuth();

  void resetTimer();

  void refreshTimeLabels();

  void syncOAuthInfo();

  void fetchAll();
  QString inboxEndpoint(QString path);

  void feed(QString verb, QVariantMap object, int response_id,
            int to=RECIPIENT_EMPTY, int cc=RECIPIENT_EMPTY);

  bool sendNotification(QString summary, QString text);
  
  PumpaSettingsDialog* m_settingsDialog;
  PumpaSettings* m_s;

  void createActions();
  void createMenu();

  QAction* newNoteAction;
  QAction* newPictureAction;
  QAction* reloadAction;
  QAction* followAction;
  QAction* loadOlderAction;
  QAction* openPrefsAction;
  QAction* exitAction;
  QMenu* fileMenu;

  QAction* aboutAction;
  QAction* aboutQtAction;
  QMenu* helpMenu;

  KQOAuthManager *oaManager;
  KQOAuthRequest *oaRequest;

  TabWidget* m_tabWidget;
  CollectionWidget* m_inboxWidget;
  CollectionWidget* m_directMajorWidget;
  CollectionWidget* m_directMinorWidget;
  CollectionWidget* m_inboxMinorWidget;
  ContextWidget* m_contextWidget;
  ObjectListWidget* m_followersWidget;
  ObjectListWidget* m_followingWidget;

  QASActor* m_selfActor;

  OAuthWizard* m_wiz;

  MessageWindow* m_messageWindow;

  QSystemTrayIcon* m_trayIcon;
  QMenu* m_trayIconMenu;
  QAction* m_showHideAction;

  int m_timerId;
  int m_timerCount;
  int m_requests;

  QNetworkAccessManager* m_nam;

  QSignalMapper* m_notifyMap;
#ifdef USE_DBUS
  QDBusInterface* m_dbus;
#endif
};

#endif /* _PUMPAPP_H_ */

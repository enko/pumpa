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
#include <QApplication>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>

#include "QtKOAuth"

#include "collectionwidget.h"
#include "oauthwizard.h"

#define CLIENT_NAME           "pumpa"
#define CLIENT_FANCY_NAME     "Pumpa"
#define CLIENT_VERSION        "0.1"

//------------------------------------------------------------------------------

class PumpApp : public QMainWindow {
  Q_OBJECT

public:
  PumpApp(QWidget* parent=0);
  virtual ~PumpApp();                            

  static QString siteUrlFixer(QString url);

signals:
  void userAuthorizationStarted();
                    
private slots:
  void postNote(QString note);
  void postReply(QASObject* replyToObj, QString content);
  void errorMessage(QString msg);
  void notifyMessage(QString msg);
  void statusMessage(const QString& msg);

  void onFirstPageCommitted(QString, QString);
  void wizardDone();

  void onTemporaryTokenReceived(QString temporaryToken,
                                QString temporaryTokenSecret);
  void onAuthorizationReceived(QString token, QString verifier);
  void onAccessTokenReceived(QString token, QString tokenSecret);
  // void onAuthorizedRequestDone();
  void onOAuthClientRegDone();

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

protected:
  void timerEvent(QTimerEvent*);

private:
  void startPumping();
  bool haveOAuth();

  void resetTimer();

  void getOAuthAccess();
  void registerOAuthClient();
  void syncOAuthInfo();

  void fetchAll();
  void fetchInbox(int reqType);
  void feed(QString verb, QVariantMap object, int response_id);
  
  void writeSettings();
  void readSettings();

  QSettings* settings;

  void createActions();
  void createMenu();

  QAction* newNoteAction;
  QAction* newPictureAction;
  QAction* reloadAction;
  QAction* openPrefsAction;
  QAction* exitAction;
  QMenu* fileMenu;

  QAction* aboutAction;
  QAction* aboutQtAction;
  QMenu* helpMenu;

  // QSystemTrayIcon* trayIcon;
  // QMenu* trayIconMenu;
  
  QString m_siteUrl;
  QString m_userName;

  QString clientId;
  QString clientSecret;

  QString token;
  QString tokenSecret;

  int m_reloadTime;

  KQOAuthManager *oaManager;
  KQOAuthRequest *oaRequest;

  QNetworkAccessManager *netManager;

  CollectionWidget* inboxWidget;

  OAuthWizard* oaWizard;
  int timerId;
};

#endif /* _PUMPAPP_H_ */

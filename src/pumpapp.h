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

#include "QtKOAuth"

#include "pumpa_defines.h"
#include "qactivitystreams.h"
#include "collectionwidget.h"
#include "oauthwizard.h"
#include "tabwidget.h"
#include "pumpasettingsdialog.h"
#include "pumpasettings.h"

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
  void onLike(QASObject* obj);
  void onShare(QASObject* obj);
  void postNote(QString note);
  void postReply(QASObject* replyToObj, QString content);
  void errorMessage(QString msg);
  void notifyMessage(QString msg);
  void statusMessage(const QString& msg);

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

protected:
  void timerEvent(QTimerEvent*);

private:
  void connectCollection(CollectionWidget* w);

  bool haveOAuth();

  void resetTimer();

  void refreshTimeLabels();

  void syncOAuthInfo();

  void fetchAll();
  QString inboxEndpoint(QString path);

  void feed(QString verb, QVariantMap object, int response_id);
  
  PumpaSettingsDialog* m_settingsDialog;
  PumpaSettings* m_s;

  void createActions();
  void createMenu();

  QAction* newNoteAction;
  QAction* newPictureAction;
  QAction* reloadAction;
  QAction* loadOlderAction;
  QAction* openPrefsAction;
  QAction* exitAction;
  QMenu* fileMenu;

  QAction* aboutAction;
  QAction* aboutQtAction;
  QMenu* helpMenu;

  // QSystemTrayIcon* trayIcon;
  // QMenu* trayIconMenu;
  
  KQOAuthManager *oaManager;
  KQOAuthRequest *oaRequest;

  TabWidget* m_tabWidget;
  CollectionWidget* m_inboxWidget;
  CollectionWidget* m_directMajorWidget;
  CollectionWidget* m_directMinorWidget;
  CollectionWidget* m_inboxMinorWidget;

  QASActor* m_selfActor;

  OAuthWizard* m_wiz;
  int m_timerId;
  int m_timerCount;
  int m_requests;
};

#endif /* _PUMPAPP_H_ */

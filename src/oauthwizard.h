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

#ifndef _OAUTHWIZARD_H_
#define _OAUTHWIZARD_H_

#include <QWidget>
#include <QLabel>
#include <QWizard>
#include <QWizardPage>
#include <QNetworkAccessManager>

#include "QtKOAuth"

//------------------------------------------------------------------------------

class OAuthFirstPage : public QWizardPage {
  Q_OBJECT

public:
  OAuthFirstPage(QWidget* parent=0);
  void setMessage(QString msg);

signals:
  void committed(QString, QString);

protected:
  virtual bool validatePage(); 
  virtual bool isComplete() const;

private:
  bool splitAccountId(QString& username, QString& server) const;
  QLabel* m_messageLabel;
};

//------------------------------------------------------------------------------

class OAuthSecondPage : public QWizardPage {
  Q_OBJECT

public:
  OAuthSecondPage(QWidget* parent=0);

protected:
  virtual bool validatePage(); 

signals:
  void committed(QString, QString);
};

//------------------------------------------------------------------------------

class OAuthWizard : public QWizard {
Q_OBJECT

public:
  OAuthWizard(QWidget* parent=0);

signals:
  void clientRegistered(QString, QString, QString, QString);
  void accessTokenReceived(QString, QString);

private slots:
  void onFirstPageCommitted(QString, QString);
  void onSecondPageCommitted(QString, QString);

  void onOAuthClientRegDone();
  void onTemporaryTokenReceived(QString temporaryToken,
                                QString temporaryTokenSecret);
  void onAccessTokenReceived(QString token, QString tokenSecret);

private:
  void notifyMessage(QString);
  void errorMessage(QString);

  void registerOAuthClient();
  void getOAuthAccess();

  OAuthFirstPage* p1;
  OAuthSecondPage* p2;

  KQOAuthManager *m_oam;
  KQOAuthRequest *m_oar;
  QNetworkAccessManager *m_nam;

  QString m_server, m_username;
  QString m_clientId, m_clientSecret;

  int m_clientRegTryCount;
};

#endif /* _OAUTHWIZARD_H_ */

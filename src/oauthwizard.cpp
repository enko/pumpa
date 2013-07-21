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

#include <QDebug>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QRegExp>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QVariantMap>
#include <QByteArray>

#include "pumpa_defines.h"
#include "oauthwizard.h"
#include "util.h"
#include "json.h"

#define EXAMPLE_ACCOUNT_ID "username@example.com"

//------------------------------------------------------------------------------

OAuthFirstPage::OAuthFirstPage(QWidget* parent) :
  QWizardPage(parent)
{
  setTitle(tr("Welcome to Pumpa!"));

  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* infoLabel = 
    new QLabel(tr("<p>In order to use pump.io you need to first register an "
                  "account with a pump.io server. If you haven't done this yet "
                  "you can do it now by trying out one of the existing public "
                  "servers: <br /><a href=\"http://pump.io/tryit.html\">"
                  "http://pump.io/tryit.html</a>.</p>"
                  "<p>When you are done enter your new pump.io account id "
                  "below in the form of <b>username@servername</b>.</p>"),
               this);
  infoLabel->setOpenExternalLinks(true);
  infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                     Qt::LinksAccessibleByMouse);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);
  layout->addStretch();

  m_messageLabel = new QLabel(this);
  layout->addWidget(m_messageLabel);

  QLabel* accountIdLabel =
    new QLabel(tr("<b>Your pump.io account id:</b>"), this);
  QLineEdit* accountIdEdit = new QLineEdit(EXAMPLE_ACCOUNT_ID, this);
  accountIdLabel->setBuddy(accountIdEdit);
  connect(accountIdEdit, SIGNAL(textEdited(const QString&)),
          this, SIGNAL(completeChanged()));

  layout->addWidget(accountIdLabel);
  layout->addWidget(accountIdEdit);

  registerField("accountId*", accountIdEdit);

  setButtonText(QWizard::CommitButton, tr("Next"));
  setCommitPage(true);
  setLayout(layout);
}

//------------------------------------------------------------------------------

void OAuthFirstPage::setMessage(QString msg) {
  m_messageLabel->setText(msg);
}

//------------------------------------------------------------------------------

bool OAuthFirstPage::splitAccountId(QString& username, QString& server) const {
  QString accountId = field("accountId").toString().trimmed();

  if (accountId == EXAMPLE_ACCOUNT_ID)
    return false;

  return splitWebfingerId(accountId, username, server);
}

//------------------------------------------------------------------------------

bool OAuthFirstPage::isComplete() const {
  QString username, server;
  return splitAccountId(username, server);
}

//------------------------------------------------------------------------------

bool OAuthFirstPage::validatePage() {
  QString username, server;
  bool ok = splitAccountId(username, server);

  if (!ok)
    return false;

  emit committed(username, server);
  return true;
}

//------------------------------------------------------------------------------

OAuthSecondPage::OAuthSecondPage(QWidget* parent) : QWizardPage(parent) {
  setTitle(tr("Authorise Pumpa"));

  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* infoLabel = 
    new QLabel(tr("In order for Pumpa to be able to read and post new messages "
                  "to your pump.io account you need to grant Pumpa access via "
                  "the web page. Pumpa will open the web page for you - just "
                  "follow the instructions and copy &amp; paste the "
                  "<b>verifier</b> text string back into the field below. (The"
                  "token should be automatically pre-filled.)"),
               this);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  QLabel* tokenLabel = new QLabel(tr("Token:"), this);
  QLineEdit* tokenEdit = new QLineEdit(this);
  tokenLabel->setBuddy(tokenEdit);
  layout->addWidget(tokenLabel);
  layout->addWidget(tokenEdit);

  QLabel* verifierLabel = new QLabel(tr("Verifier:"), this);
  QLineEdit* verifierEdit = new QLineEdit(this);
  verifierLabel->setBuddy(verifierEdit);
  layout->addWidget(verifierLabel);
  layout->addWidget(verifierEdit);

  registerField("token*", tokenEdit);
  registerField("verifier*", verifierEdit);

  setLayout(layout);
}

//------------------------------------------------------------------------------

bool OAuthSecondPage::validatePage() {
  QString token = field("token").toString();
  QString verifier = field("verifier").toString();

  if (token.isEmpty() || verifier.isEmpty())
    return false;

  emit committed(token, verifier);
  return true;
}

//------------------------------------------------------------------------------

OAuthWizard::OAuthWizard(QNetworkAccessManager* nam, QWidget* parent) :
  QWizard(parent),
  m_nam(nam)
{
  setWindowTitle(CLIENT_FANCY_NAME);

  m_oam = new KQOAuthManager(this);
  m_oar = new KQOAuthRequest(this);

  p1 = new OAuthFirstPage(this);
  p2 = new OAuthSecondPage(this);

  connect(p1, SIGNAL(committed(QString, QString)),
          this, SLOT(onFirstPageCommitted(QString, QString)));
  connect(p2, SIGNAL(committed(QString, QString)),
          this, SLOT(onSecondPageCommitted(QString, QString)));

  addPage(p1);
  addPage(p2);
}

//------------------------------------------------------------------------------

void OAuthWizard::notifyMessage(QString msg) {
  qDebug() << "[OAuthWizard]" << msg;
}

//------------------------------------------------------------------------------

void OAuthWizard::errorMessage(QString msg) {
  p1->setMessage("<b><font color=\"red\">"+msg+"</font></b>");
  qDebug() << "[OAuthWizard ERROR]" << msg;
  back();
}

//------------------------------------------------------------------------------

void OAuthWizard::onFirstPageCommitted(QString username, QString server) {
  m_username = username;
  m_server = siteUrlFixer(server);
  m_clientRegTryCount = 0;
  registerOAuthClient();
}

//------------------------------------------------------------------------------

void OAuthWizard::registerOAuthClient() {
  notifyMessage(tr("Registering client ..."));
  m_clientRegTryCount++;

  QUrl serverUrl(m_server);
  serverUrl.setPath("/api/client/register");
  qDebug() << serverUrl;

  QNetworkRequest req;
  req.setUrl(serverUrl);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  QVariantMap post;
  post["type"] = "client_associate";
  post["application_type"] = "native";
  post["application_name"] = CLIENT_FANCY_NAME;
  post["logo_uri"] = "http://saz.im/images/pumpa.png";

  QByteArray postData = serializeJson(post);
  
  // qDebug() << "data=" << postData;

  QNetworkReply *reply = m_nam->post(req, postData);

  connect(reply, SIGNAL(finished()), this, SLOT(onOAuthClientRegDone()));
}

//------------------------------------------------------------------------------

void OAuthWizard::onOAuthClientRegDone() {
  QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
  if (reply->error() != QNetworkReply::NoError) {
    if (m_clientRegTryCount == 1 && m_server.startsWith("https://")) {
      m_server.replace("https://", "http://");
      registerOAuthClient();
    } else
      errorMessage(tr("Network error: ") + reply->errorString());
    return;
  }

  QByteArray data = reply->readAll();

  QVariantMap json = parseJson(data);
  m_clientId = json["client_id"].toString();
  m_clientSecret = json["client_secret"].toString();

  emit clientRegistered(m_username, m_server, m_clientId, m_clientSecret);

  notifyMessage(QString(tr("Registered client to [%1] successfully.")).
                arg(m_server));

  getOAuthAccess();
}

//------------------------------------------------------------------------------

void OAuthWizard::getOAuthAccess() {
  notifyMessage(tr("Authorising user ..."));

  connect(m_oam, SIGNAL(temporaryTokenReceived(QString, QString)),
          this, SLOT(onTemporaryTokenReceived(QString, QString)));
  connect(m_oam, SIGNAL(accessTokenReceived(QString, QString)),
          this, SLOT(onAccessTokenReceived(QString, QString)));

  m_oar->initRequest(KQOAuthRequest::TemporaryCredentials,
                     QUrl(m_server+"/oauth/request_token"));
  m_oar->setConsumerKey(m_clientId);
  m_oar->setConsumerSecretKey(m_clientSecret);

  m_oam->executeRequest(m_oar);
}

//------------------------------------------------------------------------------

void OAuthWizard::onTemporaryTokenReceived(QString token,
                                           QString /*tokenSecret*/) {
  setField("token", token);
  QUrl userAuthURL(m_server+"/oauth/authorize");
  if (m_oam->lastError() == KQOAuthManager::NoError)
    m_oam->getUserAuthorization(userAuthURL);
  else 
    errorMessage(tr("Network or authentication error!"));
}

//------------------------------------------------------------------------------

void OAuthWizard::onSecondPageCommitted(QString token, QString verifier) {
  m_oam->verifyToken(token, verifier);
  m_oam->getUserAccessTokens(QUrl(m_server+"/oauth/access_token"));
}

//------------------------------------------------------------------------------

void OAuthWizard::onAccessTokenReceived(QString token, QString tokenSecret) {
  emit accessTokenReceived(token, tokenSecret);
}


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

#include "oauthwizard.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>

//------------------------------------------------------------------------------

OAuthFirstPage::OAuthFirstPage(QWidget* parent) :
  QWizardPage(parent), status(0) 
{
  setTitle("Enter username and server");

  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* infoLabel = new QLabel("Some long explanation here.", this);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  QLabel* userNameLabel = new QLabel("Username:", this);
  QLineEdit* userNameEdit = new QLineEdit(this);
  userNameLabel->setBuddy(userNameEdit);
  layout->addWidget(userNameLabel);
  layout->addWidget(userNameEdit);

  QLabel* serverLabel = new QLabel("Server:", this);
  QLineEdit* serverEdit = new QLineEdit(this);
  serverLabel->setBuddy(serverEdit);
  layout->addWidget(serverLabel);
  layout->addWidget(serverEdit);

  registerField("userName*", userNameEdit);
  registerField("serverUrl*", serverEdit);

  setButtonText(QWizard::CommitButton, "Next");
  setCommitPage(true);
  setLayout(layout);
}

//------------------------------------------------------------------------------

bool OAuthFirstPage::validatePage() {
  if (status == 1)
    return false;

  if (status == 2)
    return true;

  status = 1;
  QString userName = field("userName").toString();
  QString serverUrl = field("serverUrl").toString();

  emit committed(userName, serverUrl);
  return false;
}

//------------------------------------------------------------------------------

bool OAuthFirstPage::isComplete() const {
  return (status != 1) && QWizardPage::isComplete();
}

//------------------------------------------------------------------------------

OAuthSecondPage::OAuthSecondPage(QWidget* parent) : QWizardPage(parent) {
  setTitle("User authorisation.");

  QVBoxLayout* layout = new QVBoxLayout(this);

  QLabel* infoLabel = new QLabel("Some long explanation here.", this);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  QLabel* tokenLabel = new QLabel("Token:", this);
  QLineEdit* tokenEdit = new QLineEdit(this);
  tokenLabel->setBuddy(tokenEdit);
  layout->addWidget(tokenLabel);
  layout->addWidget(tokenEdit);

  QLabel* verifierLabel = new QLabel("Verifier:", this);
  QLineEdit* verifierEdit = new QLineEdit(this);
  verifierLabel->setBuddy(verifierEdit);
  layout->addWidget(verifierLabel);
  layout->addWidget(verifierEdit);

  registerField("token*", tokenEdit);
  registerField("verifier*", verifierEdit);

  setLayout(layout);
}

//------------------------------------------------------------------------------

OAuthWizard::OAuthWizard(QWidget* parent) : QWizard(parent) {
  setWindowTitle("foo");
  p1 = new OAuthFirstPage(this);
  p2 = new OAuthSecondPage(this);

  connect(p1, SIGNAL(committed(QString, QString)),
          this, SIGNAL(firstPageCommitted(QString, QString)));
  connect(p2, SIGNAL(committed(QString, QString)),
          this, SIGNAL(secondPageCommitted(QString, QString)));

  addPage(p1);
  addPage(p2);

  setField("userName", "sazius");
  setField("serverUrl", "io.saz.im");
}

//------------------------------------------------------------------------------

void OAuthWizard::gotoSecondPage() {
  p1->userLoopDone();
  next();
}

//------------------------------------------------------------------------------


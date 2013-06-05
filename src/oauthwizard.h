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

#include <QWizard>
#include <QWizardPage>
#include <QWidget>

//------------------------------------------------------------------------------

class OAuthFirstPage : public QWizardPage {
  Q_OBJECT

public:
  OAuthFirstPage(QWidget* parent=0);
  virtual bool validatePage(); 
  void userLoopDone() { status = 2; }

signals:
  void committed(QString, QString);

protected:
  virtual bool isComplete() const;

private:
  int status;
};

//------------------------------------------------------------------------------

class OAuthSecondPage : public QWizardPage {
  Q_OBJECT

public:
  OAuthSecondPage(QWidget* parent=0);

signals:
  void committed(QString, QString);
};

//------------------------------------------------------------------------------

class OAuthWizard : public QWizard {
Q_OBJECT

public:
  OAuthWizard(QWidget* parent=0);

signals:
  void firstPageCommitted(QString, QString);
  void secondPageCommitted(QString, QString);

public slots:
  void gotoSecondPage();

private:
  OAuthFirstPage* p1;
  OAuthSecondPage* p2;
};

#endif /* _OAUTHWIZARD_H_ */

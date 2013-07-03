/*
  Copyright 2013 Mats Sjöberg
  
  This file is part of the Pumpa programme.

  Pumpa is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Pumpa is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Pumpa.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QApplication>

#include "pumpapp.h"
#include "util.h"

//------------------------------------------------------------------------------

int testMarkDown(QString str) {
  if (str.isEmpty())
    str = QString("Hello *world*!\n\nThis is unicode: ") + QChar(0x267A) +
      QString(" and straight HTML: <code>foo</code>.");
    
  qDebug() << markDown(str);
  return 0;
}

//------------------------------------------------------------------------------

int testMarkup(QString str) {
  qDebug() << PumpApp::addTextMarkup(str);
  return 0;
}

//------------------------------------------------------------------------------

int main(int argc, char** argv) {
  QApplication app(argc, argv);

  QString settingsFile;
  if (argc > 1) {
    QString arg(argv[1]);
    if (arg == "test") 
      settingsFile = "pumpa-test.conf";
    else if (arg == "testmarkdown")
      return testMarkDown(argc > 2 ? argv[2] : "");
    else if (arg == "testmarkup")
      return testMarkup(argv[2]);
    else if (arg == "testfeedint") {
      qDebug() << PumpaSettingsDialog::feedIntToComboIndex(atoi(argv[2]));
      return 0;
    }
    else if (arg == "autotestfeedint") {
      int (*f)(int) = PumpaSettingsDialog::feedIntToComboIndex;
      Q_ASSERT(f(0) == 0);
      Q_ASSERT(f(1) == 0);
      Q_ASSERT(f(2) == 1);
      Q_ASSERT(f(3) == 0);
      Q_ASSERT(f(4) == 0);
      Q_ASSERT(f(5) == 0);
      Q_ASSERT(f(6) == 2);
      Q_ASSERT(f(11) == 0);
      Q_ASSERT(f(12) == 0);
      Q_ASSERT(f(14) == 3);
      Q_ASSERT(f(15) == 4);
      Q_ASSERT(f(16) == 0);
      Q_ASSERT(f(255) == 0);
      return 0;
    }
    else if (arg == "autotestcomboindex") {
      int (*f)(int) = PumpaSettingsDialog::comboIndexToFeedInt;
      Q_ASSERT(f(0) == 0);
      Q_ASSERT(f(1) == 2);
      Q_ASSERT(f(2) == 6);
      Q_ASSERT(f(3) == 14);
      Q_ASSERT(f(4) == 15);
      Q_ASSERT(f(255) == 0);
      return 0;
    }
  }

  PumpApp papp(settingsFile);
  return app.exec();
}

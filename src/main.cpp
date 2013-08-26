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
#ifdef Q_OS_WIN32
#include <QStyleFactory>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>
#include <QtGlobal>
#include <QDateTime>
#endif

#include "pumpapp.h"
#include "util.h"
#include "pumpa_defines.h"

#include <QTranslator>
#include <QLocale>

//------------------------------------------------------------------------------

int testMarkup(QString str) {
  if (str.isEmpty()) 
    str = "Hello *world*, [Some Url](http://www.foo.bar/baz). Some\n"
      "> block quoted text\n\n"
      "A `plain` url: http://saz.im";
  addTextMarkup(str);
  return 0;
}

//------------------------------------------------------------------------------

#ifdef Q_OS_WIN32
void MessageOutputHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
  //in this function, you can write the message to any stream!
  QString logfile = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/pumpa.log";
  QFile file(logfile);
  file.open(QIODevice::Append | QIODevice::Text);
  QTextStream out(&file);
  QString logmsg = QString("[%1] %6: %2 (%3:%4, %5)\n")
      .arg((QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss")))
      .arg(msg)
      .arg(context.file)
      .arg(context.line)
      .arg(context.function)
      .arg("%1");
  switch (type) {
    case QtDebugMsg:
      out << logmsg
             .arg("Debug");
      break;
    case QtWarningMsg:
    out << logmsg
           .arg("Warning");
      break;
    case QtCriticalMsg:
      out << logmsg
             .arg("Critical");
      break;
    case QtFatalMsg:
      out << logmsg
             .arg("Fatal");
      abort();
  }
  file.close();
}
#endif


//------------------------------------------------------------------------------

int main(int argc, char** argv) {
  #ifdef Q_OS_WIN32
  qInstallMessageHandler(MessageOutputHandler);
  #endif
  QApplication app(argc, argv);

  app.setApplicationName(CLIENT_FANCY_NAME);
  app.setApplicationVersion(CLIENT_VERSION);

  QString locale = QLocale::system().name();
  #ifdef Q_OS_WIN32
  app.setStyle(QStyleFactory::create("Fusion"));
  #endif

  QString settingsFile;
  QStringList args = app.arguments();
  if (args.count() > 1) {
    QString arg(args[1]);
    if (arg == "testmarkup")
      return testMarkup(argc > 2 ? args[2] : "");
    else if (arg == "testfeedint") {
      qDebug() << PumpaSettingsDialog::feedIntToComboIndex(args[2].toInt());
      return 0;
    }
    else if (arg == "autotestfeedint") {
      int (*f)(int) = PumpaSettingsDialog::feedIntToComboIndex;
      (void) f;
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
      (void) f;
      Q_ASSERT(f(0) == 0);
      Q_ASSERT(f(1) == 2);
      Q_ASSERT(f(2) == 6);
      Q_ASSERT(f(3) == 14);
      Q_ASSERT(f(4) == 15);
      Q_ASSERT(f(255) == 0);
      return 0;
    }
    else if (arg == "-l" && argc == 3) {
      locale = args[2];
    } else if (arg == "-c" && argc == 3) {
      settingsFile = args[2];
    }
    else {
      qDebug() << "Usage: ./pumpa [-c alternative.conf] [-l locale]";
      return 0;
    }
  }
  qDebug() << "Using locale" << locale;

  QTranslator qtTranslator;
  qtTranslator.load("qt_" + locale,
                    QLibraryInfo::location(QLibraryInfo::TranslationsPath));
  app.installTranslator(&qtTranslator);

  QTranslator translator;
  bool ok = translator.load(QString("pumpa_%1").arg(locale),
                            ":/translations");
  app.installTranslator(&translator);

  if (ok) 
    qDebug() << "Successfully loaded translation";

  PumpApp papp(settingsFile, locale);
  return app.exec();
}

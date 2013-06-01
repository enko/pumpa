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

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QtCore>
#include <QtNetwork>

class FileDownloader : public QObject {
  Q_OBJECT

public:
  static FileDownloader* get(const QString& url);

  void download();

  bool downloading() const { return m_downloadStarted; }

  bool ready() const { return !m_cachedFile.isEmpty(); }
  QString fileName() const;

  static QString getCacheDir() { return m_cacheDir; }
  
  static QString urlToPath(const QString& url);
  
signals:
  void networkError(const QString&);
  void fileReady(const QString&);

private slots:
  void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);
  void replyFinished(QNetworkReply* nr);

private:
  FileDownloader();
  FileDownloader(const QString&);

  QNetworkAccessManager* m_nam;
  QString m_downloadingUrl;
  QString m_cachedFile;

  bool m_downloadStarted;

  static QString m_cacheDir;
  static QMap<QString, FileDownloader*> m_downloading;
};

#endif

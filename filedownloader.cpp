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

#include "filedownloader.h"

#ifdef QT5
#include <QStandardPaths>
#else
#include <QDesktopServices>
#endif

#include <QCryptographicHash>

//------------------------------------------------------------------------------

QString FileDownloader::m_cacheDir;
QMap<QString, FileDownloader*> FileDownloader::m_downloading;

//------------------------------------------------------------------------------

QString slashify(const QString& url) {
  QString ret = url;
  if (!ret.endsWith('/'))
    ret.append('/');
  return ret;
}

//------------------------------------------------------------------------------
FileDownloader::FileDownloader(const QString& url) :
  m_downloadingUrl(url),
  m_downloadStarted(false)
{
  QString fn = urlToPath(m_downloadingUrl);

  if (QFile::exists(fn)) {
    m_cachedFile = fn;
  } else {
    m_cachedFile = "";
    m_downloading.insert(m_downloadingUrl, this);
    m_nam = new QNetworkAccessManager(this);
    connect(m_nam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    connect(m_nam, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError>&)));
  }
}

//------------------------------------------------------------------------------

FileDownloader* FileDownloader::get(const QString& url) {
  if (m_downloading.contains(url))
    return m_downloading[url];

  return new FileDownloader(url);
}

//------------------------------------------------------------------------------

void FileDownloader::download() {
  if (m_downloadStarted)
    return;
  
  m_nam->get(QNetworkRequest(QUrl(m_downloadingUrl)));
  m_downloadStarted = true;
}

//------------------------------------------------------------------------------

QString FileDownloader::fileName() const {
  return ready() ? m_cachedFile : urlToPath(m_downloadingUrl);
}

//------------------------------------------------------------------------------

void FileDownloader::onSslErrors(QNetworkReply* reply,
                                 const QList<QSslError>& /*errors*/) {
  // for (int i=0; i<errors.size(); i++)
  //   qDebug() << "SSL Error:" << errors[i].errorString();

  reply->ignoreSslErrors();
}

//------------------------------------------------------------------------------

void FileDownloader::replyFinished(QNetworkReply* nr) {
  m_downloading.remove(m_downloadingUrl);

  if (nr->error()) {
    emit networkError("Network error: "+nr->errorString());
    return;
  }

  QString fn = urlToPath(m_downloadingUrl);
  QFile* fp = new QFile(fn);
  if (!fp->open(QIODevice::WriteOnly)) {
    emit networkError(QString("Could not open "+fn+" for writing: "+
                              fp->errorString()));
    return;
  }
  fp->write(nr->readAll());
  fp->close();
  
  nr->deleteLater();
  //  deleteLater();
  emit fileReady(fn);
}

//------------------------------------------------------------------------------

QString FileDownloader::urlToPath(const QString& url) {
  static QCryptographicHash hash(QCryptographicHash::Md5);

  if (m_cacheDir.isEmpty()) {
    m_cacheDir = 
#ifdef QT5
      QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
#else
      QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
#endif
    if (m_cacheDir.isEmpty())
      m_cacheDir = slashify(QDir::homePath())+".cache/";
    else
      m_cacheDir = slashify(m_cacheDir);
    m_cacheDir += "yaics/";
  }
  QString path = m_cacheDir;
  QDir d;
  d.mkpath(path);

  hash.reset();
  hash.addData(url.toUtf8());
    
  return path + QString(hash.result().toHex()) + "-" + url.section('/',-1);
}

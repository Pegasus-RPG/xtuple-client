/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkForUpdates.h"

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSqlError>
#include <QTranslator>
#include <QUrl>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif
#include <parameter.h>

#include "xsqlquery.h"

#define DEBUG false
#define QT_NO_URL_CAST_FROM_STRING

checkForUpdates::checkForUpdates(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl)
{
  Q_UNUSED(name);
  QString url = "http://updates.xtuple.com/updates";
  //intended http://updates.xtuple.com/updates/xTuple-4.2.0-Linux.tar.gz

  setupUi(this);
  progressDialog = new QProgressDialog(this);
  _ok = _buttonBox->button(QDialogButtonBox::Ok);
  _ignore = _buttonBox->button(QDialogButtonBox::Ignore);
  connect(_ok, SIGNAL(clicked()), this, SLOT(downloadButtonPressed()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_ignore, SIGNAL(clicked()), this, SLOT (accept()));

#ifdef Q_OS_MACX
OS = "OSX";
suffix = "tar.gz";
#endif
#ifdef Q_OS_WIN
OS = "Windows";
suffix = "exe";
#endif
#ifdef Q_OS_LINUX
OS = "Linux";
suffix = "tar.gz";
#endif

  XSqlQuery metric;
  metric.exec("SELECT fetchMetricText('ServerVersion') AS dbver,"
              "       fetchMetricBool('DisallowMismatchClientVersion') as disallowed,"
              "       fetchMetricBool('AutoVersionUpdate') as allow;");

  if(metric.first())
  {
    serverVersion = metric.value("dbver").toString();
    newurl = url + "/xTuple-" + serverVersion + "-" + OS +"."+ suffix;
    qDebug() <<"newurl=" << newurl;

    _label->setText(tr("Your client does not match the server version: %1. Would you like to update?").arg(serverVersion));
    _ignore->setEnabled(!metric.value("disallowed").toBool());
    _ok->setEnabled(metric.value("allow").toBool());
  }
  else if (metric.lastError().type() != QSqlError::NoError) {
    QMessageBox::warning(parent, tr("System Error"),
                         tr("Could not find version information: %1")
                           .arg(metric.lastError().text()));
  }
  if (DEBUG)
  {
    qDebug() << "serverVersion= " << serverVersion;
    qDebug() << "newurl= " << newurl;
  }
}
void checkForUpdates::downloadButtonPressed()
{
     // this->close();
      QUrl url(newurl);
      reply = NULL;
      filename = "xTuple-" + serverVersion + "-" + OS + "." + suffix;
      //xTuple-4.2.0-Linux.tar.gz

      if(QFile::exists(filename))
      {
          if(QMessageBox::question(this, tr("Update"),
              tr("There already exists a file called %1 in "
                  "the current directory. Overwrite?").arg(filename),
                  QMessageBox::Yes|QMessageBox::No, QMessageBox::No)
                  == QMessageBox::No)
                  return;
          QFile::remove(filename);
      }

      file = new QFile(filename);
      if(!file->open(QIODevice::WriteOnly))
      {
          QMessageBox::information(this, "Update",
              tr("Unable to save the file %1: %2.")
              .arg(filename).arg(file->errorString()));
          delete file;
          file = NULL;
          return;
      }

      downloadRequestAborted = false;
      reply = manager.get(QNetworkRequest(url));
      connect(reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
      connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
      connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
      connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
      connect(this, SIGNAL(done(int)), this, SLOT(startUpdate()));

      progressDialog->setLabelText(tr("Downloading %1...").arg(filename));
      _ok->setEnabled(false);
      progressDialog->exec();
}
void checkForUpdates::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(downloadRequestAborted)
        return;
    progressDialog->setMaximum(bytesTotal);
    filesize = bytesTotal;
    progressDialog->setValue(bytesReceived);
}
void checkForUpdates::downloadReadyRead()
{
    if(file)
        file->write(reply->readAll());
}
void checkForUpdates::cancelDownload()
{
    downloadRequestAborted = true;
    reply->abort();
    _ok->setEnabled(true);
}
void checkForUpdates::downloadFinished()
{
    int result = QDialog::Accepted;
    if(downloadRequestAborted)
    {
        if(file)
        {
            file->close();
            file->remove();
            delete file;
            file = NULL;
        }
        reply->deleteLater();
        progressDialog->hide();
        _ok->setEnabled(true);
        return;
    }

    downloadReadyRead();
    progressDialog->hide();
    _ok->setEnabled(true);
    file->flush();
    file->close();

    if(reply->error())
    {
        //Download failed
        QMessageBox::information(this, "Download failed", tr("Failed: %1").arg(reply->errorString()));
        result = QDialog::Rejected;
    }

    reply->deleteLater();
    reply = NULL;
    delete file;
    file = NULL;

    if(reply == 0)
        emit done(result);
}
void checkForUpdates::startUpdate()
{
    this->close();
    qDebug() <<"filename= " << filename;
    QFile *updater = new QFile(filename);
    if(updater->exists())
    {
        QStringList options;
        QProcess *installer = new QProcess(this);
        #ifdef Q_OS_MAC
        QProcess sh;
        sh.start("tar -xvf " + filename);
        if(sh.waitForFinished())
        {
        sh.close();
        filename = "xTuple-" + serverVersion + ".app";
        QFileInfo *path2 = new QFileInfo(filename);
        QString filepath = path2->absoluteFilePath() + "/Contents/MacOS/xtuple";
        QFile osxUpdate(filepath);
        osxUpdate.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|QFile::ReadOther|QFile::WriteOther|QFile::ExeOther);
        if(installer->startDetached(filepath, options))
            reject();
        }
        #endif
        #ifdef Q_OS_LINUX
        QProcess sh2;
        sh2.start("tar -xvf " + filename + " -C ../");
        if(sh2.waitForFinished())
        {
        sh2.close();
        filename = "../xTuple-" + serverVersion + "-" + OS + "/xtuple";
        QFile launch(filename);
        launch.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner|QFile::ReadGroup|QFile::WriteGroup|QFile::ExeGroup|QFile::ReadOther|QFile::WriteOther|QFile::ExeOther);
        QFileInfo *path = new QFileInfo(filename);
        if(installer->startDetached(path->absoluteFilePath(), options))
             reject();
        }
        #endif
        #ifdef Q_OS_WIN
        int result = (int)::ShellExecuteA(0, "open", filename.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
        qDebug() << "result= " << result;
        if (SE_ERR_ACCESSDENIED== result)
        {
            result = (int)::ShellExecuteA(0, "runas", filename.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
            reject();
        }
        if (result <= 32)
            QMessageBox::information(this, "Download failed", tr("Failed: %1").arg(result));
        #endif
    }
}

checkForUpdates::~checkForUpdates()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}


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
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QSqlError>
#include <QTranslator>
#include <QUrl>
#include <QXmlQuery>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif
#include "../guiclient/guiclient.h"
#include <parameter.h>

#define QT_NO_URL_CAST_FROM_STRING

#define DEBUG false

class Sudoable : public QProcess {
  public:
    void start(const QString password, const QString &program, OpenMode mode = ReadWrite)
    {
      if (DEBUG)
        qDebug() << "starting" << QString(password.isEmpty() ? "" : "sudo")
                 << program;
      if (password.isEmpty())
        QProcess::start(program, mode);
      else
      {
        QProcess::start("sudo -S " + program, mode);
        waitForStarted();
        write(password.toLocal8Bit());
        write("\n");
      }
      if (DEBUG) qDebug() << "started";
    };
};

class checkForUpdatesPrivate {
  public:
    checkForUpdatesPrivate(checkForUpdates *parent) { Q_UNUSED(parent); };

    QString filename() {
#ifdef Q_OS_MACX
      QString OS     = "OSX";
      QString suffix = "tar.gz";
#endif
#ifdef Q_OS_WIN
      QString OS     = "Windows";
      QString suffix = "exe";
#endif
#ifdef Q_OS_LINUX
      QString OS     = "Linux";
      QString suffix = "tar.gz";
#endif
      return "xTuple-" + _serverVersion + "-" + OS + "." + suffix;
    };

    bool ensureWritePriv(QFileInfo dirinfo) {
      if (dirinfo.isWritable())
        return true;
      return false;
    };

    QString urlString() {
      return "http://updates.xtuple.com/updates/" + filename();
    };

    QString               _serverVersion;
};

checkForUpdates::checkForUpdates(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl),
      reply(NULL)
{
  Q_UNUSED(name);

  setupUi(this);
  progressDialog = new QProgressDialog(this);
  _ok     = _buttonBox->button(QDialogButtonBox::Ok);
  _ignore = _buttonBox->button(QDialogButtonBox::Ignore);
  connect(_ok,        SIGNAL(clicked()),  this, SLOT(downloadButtonPressed()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_ignore,    SIGNAL(clicked()),  this, SLOT(accept()));

  _private = new checkForUpdatesPrivate(this);

  XSqlQuery versions, metric;
  versions.exec("SELECT metric_value AS dbver"
                "  FROM metric"
                " WHERE (metric_name = 'ServerVersion');");

  if(versions.first())
  {
    _private->_serverVersion = versions.value("dbver").toString();

    _label->setText(tr("Your client does not match the server version: %1. Would you like to update?").arg(_private->_serverVersion));

    metric.exec("SELECT fetchMetricBool('DisallowMismatchClientVersion') as disallowed;");
    metric.first();
    _ignore->setEnabled(!metric.value("disallowed").toBool());

    metric.exec("SELECT fetchMetricBool('AutoVersionUpdate') as allow;");
    metric.first();
    _ok->setEnabled(metric.value("allow").toBool());
  }
  else if (versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);
}

void checkForUpdates::downloadButtonPressed()
{
  // TODO: http://qt-project.org/doc/qt-5/qstandardpaths.html
  QUrl url(_private->urlString());
  QString filename = _private->filename();
  QString tempfile = QDir::tempPath() + QDir::separator() + filename;

  if (QFile::exists(tempfile))
  {
    QString newname = tempfile + "-%1";
    int i = 1;
    while (QFile::exists(newname.arg(i)))
      i++;
    tempfile = newname.arg(i);
  }
  if (DEBUG) qDebug() << "downloading to" << tempfile;

  file = new QFile(tempfile);
  if (!file->open(QIODevice::WriteOnly))
  {
    QMessageBox::information(this, tr("Download Failed"),
                             tr("Unable to save the file %1: %2.")
                               .arg(filename, file->errorString()));
    delete file;
    file = NULL;
    return;
  }

  downloadRequestAborted = false;
  reply = manager.get(QNetworkRequest(url));
  connect(reply, SIGNAL(finished()),  this, SLOT(downloadFinished()));
  connect(reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
  connect(progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));

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
  if (DEBUG) qDebug() << "downloadFinished() entered";

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
      QMessageBox::information(this, tr("Download Failed"),
                               tr("Failed: %1").arg(reply->errorString()));
    }
    else {
      startUpdate();
    }

    reply->deleteLater();
    reply = NULL;

    delete file;
    file = NULL;

// this->close(); // TODO: is this necessary?
  if (DEBUG) qDebug() << "downloadFinished() returning";
}

void checkForUpdates::startUpdate()
{
  if (DEBUG) qDebug() << "startUpdate() entered";

  if (file->exists())
  {
    QFileInfo   downloadinfo(*file);
    QStringList options;
    QString     subpath;

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#ifdef Q_OS_MAC
    QString destdir = ".";
    QString filename = "xTuple-" + _private->_serverVersion + ".app"
                     + "/Contents/MacOS/xtuple";
#else // Q_OS_LINUX
    QString destdir = "..";
    QString filename = "../xTuple-" + _private->_serverVersion + "-" + OS
                     + "/xtuple";
#endif
    QFileInfo destdirinfo(QDir::currentPath() + QDir::separator() + destdir);
    QString password;
    Sudoable sh;
    bool ok = false;
    if (! destdirinfo.isWritable()) {
      bool retry = false;
      do {
        password = QInputDialog::getText(0, tr("Need Password to Install"),
                                         tr("Password:"),
                                         QLineEdit::Password, password, &ok);
        if (! ok) {
          QMessageBox::information(this, tr("Download Saved"),
                                   tr("The new version has been saved to %1. "
                                      "Install it manually or delete it.")
                                   .arg(downloadinfo.absoluteFilePath()));
          reject();
          return;
        }
        sh.start(password, "echo testing");
        if (! sh.waitForFinished(1000))
          sh.terminate();
        retry = QString(sh.readAllStandardError()).contains(tr("Password:"));
      } while (retry);
    }

    QString unpack = QString("tar -xf %1 -C %2")
                            .arg(downloadinfo.absoluteFilePath())
                            .arg(destdirinfo.absoluteFilePath());
    sh.start(password, unpack);
    ok = sh.waitForFinished();
    if (ok) {
      if (DEBUG) qDebug() << "tar finished";
      sh.close();
      QFileInfo path(filename);
      QString filepath = path.absoluteFilePath();
      sh.start(password, QString("chmod a+x %1").arg(filepath));
      if (! sh.waitForFinished(1000))
      {
        sh.terminate();
        QMessageBox::critical(this, tr("Permissions Error"),
                              tr("Couldn't set execute permissions on %1: %2")
                                .arg(downloadinfo.absoluteFilePath())
                                .arg(sh.exitCode())
                                .arg(QString(sh.readAllStandardError())));
        reject();
      }
      QProcess *newver = new QProcess(this);
      if (newver->startDetached(filepath, options))
        reject();
    }
    else
    {
      sh.terminate();
      QMessageBox::critical(this, tr("Unpacking error"),
                            tr("Could not unpack %1 (%2): %3")
                              .arg(downloadinfo.absoluteFilePath())
                              .arg(sh.exitCode())
                              .arg(QString(sh.readAllStandardError())));
      reject();
      return;
    }
#endif
#ifdef Q_OS_WIN
    int result = (int)::ShellExecuteA(0, "open", filename.toUtf8().constData(), 0, 0, SW_SHOWNORMAL);
    qDebug() << "result= " << result;
    if (SE_ERR_ACCESSDENIED == result)
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
  if (progressDialog) {
    delete progressDialog;
    progressDialog = NULL;
  }
  if (_private) {
    delete _private;
    _private = NULL;
  }
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}


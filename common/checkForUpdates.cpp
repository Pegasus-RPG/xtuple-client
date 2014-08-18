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

#ifdef Q_OS_MAC
#include <DiskArbitration/DADisk.h>
#endif

#include "../guiclient/guiclient.h"
#include <parameter.h>

#define QT_NO_URL_CAST_FROM_STRING

#define DEBUG true

class Sudoable : public QProcess {
  public:
    // TODO: try sudo without -S and feed it password if it asks?
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
    }

    QString getPassword(bool *ok)
    {
      QString password;
      bool retry = false;
      do {
        password = QInputDialog::getText(0, tr("Need Password to Install"),
                                          tr("Password:"),
                                          QLineEdit::Password, password, ok);
        start(password, "echo testing");
        if (! waitForFinished(1000))
          terminate();
        retry = QString(readAllStandardError()).contains(tr("Password:"));
      } while (retry);
      return password;
    }
};

class checkForUpdatesPrivate {
  public:
    checkForUpdatesPrivate(checkForUpdates *parent, QString serverVersion)
      : _serverVersion(serverVersion)
    {
      Q_UNUSED(parent);

#ifdef Q_OS_MACX
      _newExePath   = "xTuple-" + _serverVersion + ".app/Contents/MacOS/xtuple";
      _downloadFile = "xTuple-" + _serverVersion + "-MACi386.dmg";
      _destdir = ".";
#endif
#ifdef Q_OS_WIN
      _downloadFile = "xTuple-" + _serverVersion + "-Windows.zip";
#endif
#ifdef Q_OS_LINUX
      _newExePath   = "../xTuple-" + _serverVersion + "-Linux/xtuple";
      _downloadFile = "xTuple-"    + _serverVersion + "-Linux.tar.bz2";
      _destdir = "..";
#endif
    }

    bool ensureWritePriv(QFileInfo dirinfo) {
      if (dirinfo.isWritable())
        return true;
      return false;
    }

    /* TODO: QNetworkAccessManager gets RemoteHostClosedError from SF during
             download redirects. must(?) use QTcpSocket to switch.
    return QString("http://sourceforge.net/projects/postbooks/files/")
         + "02%20PostBooks-GUIclient-only/"
         + _serverVersion + "/" + _downloadFile + "?use_mirror=autoselect";
     */
    QString urlString() {
      return "http://updates.xtuple.com/updates/" + _downloadFile;
    }

    bool makeExecutable(QString filename, QString &errmsg)
    {
      QString filepath = QFileInfo(filename).absoluteFilePath();
      Sudoable proc;
      proc.start(_password, QString("chmod a+x %1").arg(filepath));
      if (! proc.waitForFinished(1000))
      {
        proc.terminate();
        errmsg = proc.readAllStandardError();
        return false;
      }
      return true;
    }

#ifdef Q_OS_MAC
    DADiskMountCallback mountCallback(DADiskRef disk,
                                      DADissenterRef dissenter,
                                      void *context)
    {
      QString dmgpath(DADiskGetBSDName(disk));
      QFileInfo destdirinfo(QDir::currentPath() + QDir::separator() + _destdir);
      Sudoable cp;
      cp.start(_password,
               QString("cp -R ") + dmgpath + " "
             + destdirinfo.absoluteFilePath() + QDir::separator()
             + "xTuple-" + _serverVersion + ".app");
      DADiskUnmount(disk, kDADiskUnmountOptionDefault, 0, 0);
      QString errmsg;
      if (! makeExecutable(_newExePath, errmsg)) {
        QMessageBox::critical(this, tr("Permissions Error"),
                              tr("Couldn't set execute permissions on %1: %2")
                                .arg(filepath).arg(errmsg));
        reject();
      }
      QProcess::startDetached(filepath, options);
    }
#endif

    QString _destdir;
    QString _downloadFile;
    QString _newExePath;
    QString _password;
    QString _serverVersion;
};

checkForUpdates::checkForUpdates(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, modal ? (fl | Qt::Dialog) : fl),
      reply(0)
{
  Q_UNUSED(name);

  setupUi(this);
  progressDialog = new QProgressDialog(this);
  _ok     = _buttonBox->button(QDialogButtonBox::Ok);
  _ignore = _buttonBox->button(QDialogButtonBox::Ignore);
  connect(_ok,        SIGNAL(clicked()),  this, SLOT(downloadButtonPressed()));
  connect(_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
  connect(_ignore,    SIGNAL(clicked()),  this, SLOT(accept()));

  XSqlQuery versions, metric;
  versions.exec("SELECT metric_value AS dbver"
                "  FROM metric"
                " WHERE (metric_name = 'ServerVersion');");

  QString serverVersion;
  if(versions.first())
  {
    serverVersion = versions.value("dbver").toString();

    _label->setText(tr("Your client does not match the server version: %1. Would you like to update?").arg(serverVersion));

    metric.exec("SELECT fetchMetricBool('DisallowMismatchClientVersion') as disallowed;");
    metric.first();
    _ignore->setEnabled(!metric.value("disallowed").toBool());

    metric.exec("SELECT fetchMetricBool('AutoVersionUpdate') as allow;");
    metric.first();
    _ok->setEnabled(metric.value("allow").toBool());
  }
  else if (versions.lastError().type() != QSqlError::NoError)
    systemError(this, versions.lastError().text(), __FILE__, __LINE__);

  _private = new checkForUpdatesPrivate(this, serverVersion);
}

void checkForUpdates::downloadButtonPressed()
{
  // TODO: http://qt-project.org/doc/qt-5/qstandardpaths.html
  QUrl url(_private->urlString());
  QString filename = _private->_downloadFile;
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
    file = 0;
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
            file = 0;
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
    reply = 0;

    delete file;
    file = 0;

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
    QFileInfo destdirinfo(QDir::currentPath() + QDir::separator() + _private->_destdir);
    Sudoable sh;
    if (! destdirinfo.isWritable())
    {
      bool ok;
      _private->_password = sh.getPassword(&ok);
      if (! ok) {
        QMessageBox::information(this, tr("Download Saved"),
                                 tr("The new version has been saved to %1. "
                                    "Install it manually or delete it.")
                                 .arg(downloadinfo.absoluteFilePath()));
        reject();
      }
    }
    if (DEBUG) qDebug() << "empty pw?" << _private->_password.isEmpty();

#ifdef Q_OS_MAC
    CFAllocatorRef allocator = kCFAllocatorDefault;
    DASessionRef   session   = DASessionCreate(allocator);
    const char    *name      = downloadinfo.absoluteFilePath().toLatin1.data();
    DADiskRef      dmg       = DADiskCreateFromBSDName(allocator, session, name);
    DASessionScheduleWithRunLoop(session, CSRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    DADiskMount(dmg, 0, kDADiskMountOptionDefault, _private->mountCallback, 0);
#endif  // MAC
#ifdef Q_OS_LINUX
    QString filename = _private->_newExePath;
    QString unpack = QString("tar -xf %1 -C %2")
                            .arg(downloadinfo.absoluteFilePath())
                            .arg(destdirinfo.absoluteFilePath());
    sh.start(_private->_password, unpack);
    if (! sh.waitForFinished())
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
    if (DEBUG) qDebug() << "tar finished";
    sh.close();
    QString errmsg;
    if (! _private->makeExecutable(filename, errmsg)) {
      QMessageBox::critical(this, tr("Permissions Error"),
                            tr("Couldn't set execute permissions on %1: %2")
                              .arg(filename)
                              .arg(errmsg));
      reject();
    }
    if (QProcess::startDetached(filename, options))
      reject();
#endif // LINUX
#endif  // both MAC & LINUX
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
    progressDialog = 0;
  }
  if (_private) {
    delete _private;
    _private = 0;
  }
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}


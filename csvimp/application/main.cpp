/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QApplication>
#include <QDir>
#include <QMainWindow>
#include <QMessageBox>
#include <QPluginLoader>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QStyleFactory>
#include <QProxyStyle>
#include <QtDebug>

#ifdef Q_OS_MACX
#include <qmacstyle_mac.h>
#endif

#include <xsqlquery.h>
#include <dbtools.h>
#include <login.h>

#include <parameter.h>

#include "csvimpdata.h"
#include "csvimpplugininterface.h"

CSVImpPluginInterface *csvimpInterface = 0;

bool loadPlugin()
{
#ifdef HARDCODE_APPLICATION_DIR
  QDir pluginsDir(HARDCODE_APPLICATION_DIR);
#else
  QDir pluginsDir(QApplication::applicationDirPath());
#endif

  while (! pluginsDir.exists("plugins") && pluginsDir.cdUp())
    ;
  if (! pluginsDir.cd("plugins"))
    return false;

  foreach (QString fileName, pluginsDir.entryList(QDir::Files))
  {
    QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
    QObject *plugin = pluginLoader.instance();
    if (plugin)
    {
      csvimpInterface = qobject_cast<CSVImpPluginInterface*>(plugin);
      if (csvimpInterface)
        return true;
    }
    else 
    {
	  qDebug() << "\nerror: " << pluginLoader.errorString();
	}
  }

  return false;
}

int main(int argc, char *argv[])
{
  QString username  = "";
  bool    haveUsername    = false;
  bool    haveDatabaseURL = false;
  bool    loggedIn        = false;

  QString databaseURL = "";

  QApplication app(argc, argv);
  app.addLibraryPath(".");

#ifndef Q_OS_MACX
  QApplication::setWindowIcon(QIcon(":/images/CSVimpIcon-32x32.png"));
#endif

#ifdef Q_OS_WIN
  if (QSysInfo::WindowsVersion == QSysInfo::WV_XP)
    app.setStyle(QProxyStyle::create("windowsxpstyle"));
  else
    app.setStyle(new QProxyStyle);
#elif defined Q_OS_MACX
  app.setStyle(new QMacStyle);
#else
  app.setStyle(new QProxyStyle);
#endif

  if (!loadPlugin())
  {
    QMessageBox::critical(0, QObject::tr("Plugin Error"),
                             QObject::tr("Could not load the CSVImp plugin"));
    exit(1);
  }

  if (argc > 1)
  {
    haveUsername        = false;
    bool    havePasswd          = false;
    QString passwd              = "";

    for (int intCounter = 1; intCounter < argc; intCounter++)
    {
      QString argument(argv[intCounter]);

      if (argument.contains("-databaseURL=")) {
        haveDatabaseURL = true;
        databaseURL    = argument.right(argument.length() - 13);
      }
      else if (argument.contains("-username="))
      {
        haveUsername = true;
        username     = argument.right(argument.length() - 10);
      }
      else if (argument.contains("-passwd="))
      {
        havePasswd = true;
        passwd     = argument.right(argument.length() - 8);
      }
      else if (argument.contains("-noAuth"))
      {
        haveUsername = true;
        havePasswd   = true;
      }

    }

    if ( (haveDatabaseURL) && (haveUsername) && (havePasswd) )
    {
      QSqlDatabase db;
      QString      hostName;
      QString      dbName;
      QString      port;

      db = QSqlDatabase::addDatabase("QPSQL7");
      if (!db.isValid())
      {
        QMessageBox::critical(0, QObject::tr("Can not load database driver"),
                              QObject::tr("<p>Unable to load the database "
                                          "driver. Please contact your systems "
                                          "adminstrator."));
        QApplication::exit(-1);
      }

      QString protocol;
      parseDatabaseURL(databaseURL, protocol, hostName, dbName, port);
      db.setDatabaseName(dbName);
      db.setUserName(username);
      db.setPassword(passwd);
      db.setHostName(hostName);
      bool valport = false;
      int iport = port.toInt(&valport);
      if(!valport) iport = 5432;
      db.setPort(iport);
      
      if (!db.open())
      {
        QMessageBox::critical(0, QObject::tr("Unable to connect to database"),
                              QObject::tr("<p>Unable to connect to the database "
                                          "with the given information."));
        QApplication::exit(-1);
      }
      else
        loggedIn = true;
    }
  }

  if(!loggedIn)
  {
    ParameterList params;
    params.append("name",      CSVImp::name);
    params.append("copyright", CSVImp::copyright);
    params.append("version",   CSVImp::version);
    params.append("build",     CSVImp::build);

    if (haveUsername)
      params.append("username", username);

    if (haveDatabaseURL)
      params.append("databaseURL", databaseURL);

    login newdlg(0, "", true);
    QPixmap tmpPixmap(":/images/splashCSVimp.png");
    newdlg._logo->setPixmap(tmpPixmap);
    QSplashScreen splash(tmpPixmap);
    newdlg.set(params, &splash);

    if (newdlg.exec() == QDialog::Rejected)
      return -1;
  }

  QMainWindow *mainwin = csvimpInterface->getCSVToolWindow();
  mainwin->show();

  QObject::connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));
  return app.exec();
}

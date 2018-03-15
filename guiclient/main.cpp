/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2016 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

/*! \mainpage xTuple ERP Programmer Reference Index Page

  \section intro_sec Introduction

  \brief xTuple ERP is an application written in C++ using the %Qt cross-platform application and %user interface framework.

  This document provides reference information for people who want to
  either understand the core application code or develop %scripts
  and %packages to extend the xTuple ERP core.

  To navigate this documentation you should know the following:
  <ul>
    <li>Most xTuple classes are derived from %Qt classes. Much of the
        behavior that is available to you is provided by the %Qt
        classes.
    </li>
    <li>Most classes with names starting with the letter 'Q'
        are %Qt classes while the others are xTuple classes.
        The documentation of %Qt classes comes directly from %Qt and
        so has a different visual style than the xTuple documentation.
    </li>
    <li>Most windows and widgets implemented by xTuple use
	multiple inheritance.  An application window class might
	inherit properties and behavior from QMainWindow (probably
        indirectly through XMainWindow) and its collection of
        widgets from a related class in the Ui namespace.
        Look for the following xTuple conventions:
        <ul>
          <li>Widget names typically begin with an underscore ('_').
              If a widget name does not begin with this character then
              it's probably not important for manipulation (e.g. it's a
              layout or spacer).
          </li>
	  <li>Widget names that end with 'Lit' are usually given
	      to the labels for widgets that do something interesting.
	      For example, if you see a QLabel named '_currencyLit',
	      then there is probably another widget named '_currency'
	      that you might want to manipulate. If you hide a
	      widget then you should probably hide the corresponding
	      label, too.
          </li>
        </ul>
    </li>
    <li>Because of these inheritance relationships, you should use
        the 'List of all members' link frequently. The page documenting
        an xTuple class only shows the slots, signals, methods, and
        properties unique to or overridden by that class. The inherited
        features are only displayed if you look up the inheritance hierarchy
        either directly or through the 'List of all members.'
    </li>
    <li>If you are writing %scripts, you can use any slots, signals,
        and properties displayed on these pages. You can also use
        any methods marked Q_INVOKABLE. Methods which are <i>not</i>
        Q_INVOKABLE are not available for scripting.
    </li>
    <li>The documentation for xTuple classes is sparse at the moment.
	The addition of details is a work in progress.  If you would
	like to help with this effort, please send email to
        <a href="mailto:devdocs@xtuple.com?subject=xTuple%20ERP%20Programmer%20Reference%20contributions">devdocs@xtuple.com</a>.
	You can attach %file diffs with %comment changes to the email
	for us to review and hopefully merge into the core.

        See the
        <a href="http://www.stack.nl/~dimitri/doxygen/commands.html">Special Commands</a>
        chapter in the
        <a href="http://www.stack.nl/~dimitri/doxygen/manual.html">Doxygen Manual</a>
        for information on how to tag the documentation.
    </li>
    <li>The application code is continually changing as we improve
	the product. Classes and object and method names <i>will</i>
	change over time. Each page in this documentation is marked
	with the xTuple ERP version for which it applies. The fact
	that it is documented is not meant to imply that it will
	remain unchanged in future versions.
    </li>
  </ul>
*/

#include <stdlib.h>

#include <QApplication>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>
#include <QSplashScreen>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTranslator>
#if QT_VERSION < 0x050000
#include <QHttp>
#else
#include <QUrlQuery>
#endif
#include <QUrl>

#include <dbtools.h>
#include <parameter.h>
#include <widgets.h>
#include "xtupleplugin.h"
#include "xtupleproductkey.h"

#include "createfiscalyear.h"
#include "errorReporter.h"
#include "login2.h"
#include "currenciesDialog.h"
#include "registrationKeyDialog.h"
#include "guiclient.h"
#include "version.h"
#include "metrics.h"
#include "metricsenc.h"
#include "scripttoolbox.h"
#include "xmainwindow.h"
#include "checkForUpdates.h"
#include "salesOrderSimple.h"
#include "userPreferences.h"
#include "xtNetworkRequestManager.h"

#include "sysLocale.h"

#include "splashconst.h"
#include "xtsettings.h"

#include <QtPlugin>
Q_IMPORT_PLUGIN(xTuplePlugin)

QString __password;

#define DEBUG false

#if QT_VERSION >= 0x050000
extern void xTupleMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
#else
extern void xTupleMessageOutput(QtMsgType type, const char *msg);
#endif

int main(int argc, char *argv[])
{
  XSqlQuery main;
  Q_INIT_RESOURCE(guiclient);

  QString username;
  QString databaseURL;
  QString passwd;
  bool    haveUsername    = false;
  bool    haveDatabaseURL = false;
  bool    loggedIn        = false;
  bool    haveEnhancedAuth= false;
  bool    _enhancedAuth   = false;
  bool    havePasswd      = false;
#if QT_VERSION >= 0x050000
  qInstallMessageHandler(xTupleMessageOutput);
#else
  qInstallMsgHandler(xTupleMessageOutput);
#endif
  QApplication app(argc, argv);
  app.setOrganizationDomain("xTuple.com");
  app.setOrganizationName("xTuple");
  app.setApplicationName("xTuple");
  app.setApplicationVersion(_Version);

  QCoreApplication::addLibraryPath(QString("."));

#ifndef Q_OS_MAC
  QApplication::setWindowIcon(QIcon(":/images/icon32x32.png"));
#endif

  app.processEvents();

  if (argc > 1)
  {
    for (int intCounter = 1; intCounter < argc; intCounter++)
    {
      QString argument(argv[intCounter]);

      if (argument.contains("-databaseURL=", Qt::CaseInsensitive))
      {
        haveDatabaseURL = true;
        databaseURL = argument.right(argument.length() - 13);
      }
      else if (argument.contains("-username=", Qt::CaseInsensitive))
      {
        haveUsername = true;
        username = argument.right(argument.length() - 10);
      }
      else if (argument.contains("-passwd=", Qt::CaseInsensitive))
      {
        havePasswd = true;
        passwd     = argument.right(argument.length() - 8);
      }
      else if (argument.contains("-noAuth", Qt::CaseInsensitive))
      {
        haveUsername = true;
        havePasswd   = true;
      }
      else if (argument.contains("-enhancedAuth", Qt::CaseInsensitive))
      {
        haveEnhancedAuth = true;
        _enhancedAuth = true;
        if(argument.contains("=no", Qt::CaseInsensitive) || argument.contains("=false", Qt::CaseInsensitive))
          _enhancedAuth = false;
      }
    }
  }


  _splash = new QSplashScreen();
  _splash->setPixmap(QPixmap(":/images/splashEmpty.png"));

  _evaluation = false;

  QTranslator *translator = new QTranslator(&app);

  QStringList lang;
  QLocale sysl = QLocale::system();
  qDebug()<<sysl.name().toLower();
  if (sysl.language() != QLocale::C && sysl.language() != QLocale::English)
  {
    lang.append(sysl.name().toLower());

    if (translator->load(translationFile(lang.first(), "xTuple")))
      app.installTranslator(translator);
  }

  if (!loggedIn)
  {
    ParameterList params;
    params.append("copyright", _Copyright);
    params.append("version",   _Version);
    params.append("build",     _Build.arg(__DATE__).arg(__TIME__));
    params.append("applicationName", _ConnAppName);
    params.append("setSearchPath", true);

    if (haveUsername)
      params.append("username", username);

    if (havePasswd)
      params.append("password", passwd);

    if (haveDatabaseURL)
      params.append("databaseURL", databaseURL);

    if (haveEnhancedAuth)
      params.append("enhancedAuth", _enhancedAuth);

    if (_evaluation)
      params.append("evaluation");

    if ( (haveDatabaseURL) && (haveUsername) && (havePasswd) )
      params.append("login");

    login2 newdlg(0, "", true);
    newdlg.set(params, _splash);

    if(newdlg.result() != QDialog::Accepted)
    {
      if (newdlg.exec() == QDialog::Rejected)
        return -1;
      else
      {
        databaseURL = newdlg._databaseURL;
        username = newdlg.username();
        __password = newdlg.password();
      }
    }
  }
//{
  _splash->showMessage(QObject::tr("Loading Translations"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  XSqlQuery langq("SELECT lang_abbr2,   lang_qt_number,"
                  "       country_abbr, country_qt_number"
                  "  FROM usr"
                  "  JOIN locale  ON usr_locale_id     = locale_id"
                  "  JOIN lang    ON locale_lang_id    = lang_id"
                  "  LEFT OUTER JOIN country ON locale_country_id = country_id"
                  " WHERE usr_username = getEffectiveXtUser();");
  if (langq.first())
  {
    QString langAbbr    = langq.value("lang_abbr2").toString();
    QString countryAbbr = langq.value("country_abbr").toString().toUpper();

    if (! langAbbr.isEmpty() && ! countryAbbr.isEmpty())
      lang.prepend(langAbbr + "_" + countryAbbr.toLower());
    else if (! langAbbr.isEmpty())
      lang.prepend(langAbbr);

    /* set the locale to langabbr_countryabbr, langabbr, {lang# country#}, or
       lang#, depending on what information is available
     */
    if (countryAbbr == "UK")
      countryAbbr = "GB";

    if (! langAbbr.isEmpty() && ! countryAbbr.isEmpty())
      QLocale::setDefault(QLocale(langAbbr + "_" + countryAbbr));
    else if (! langAbbr.isEmpty())
      QLocale::setDefault(QLocale(langAbbr));
    else if (langq.value("lang_qt_number").toInt() &&
             langq.value("country_qt_number").toInt())
      QLocale::setDefault(
          QLocale(QLocale::Language(langq.value("lang_qt_number").toInt()),
                  QLocale::Country(langq.value("country_qt_number").toInt())));
    else
      QLocale::setDefault(sysl);

    qDebug() << "Locale set to language" << QLocale();
  }
  ErrorReporter::error(QtCriticalMsg, 0, QObject::tr("Error Getting Locale"),
                       langq, __FILE__, __LINE__);

  (void)lang.removeDuplicates();

  QList<QPair<QString, QString> > transfile;
  transfile << qMakePair(QString("xTuple"), QString()) << qMakePair(QString("openrpt"), QString()) << qMakePair(QString("reports"), QString());
  XSqlQuery pkglist("SELECT pkghead_name, pkghead_version "
                    "  FROM pkghead"
                    " WHERE packageIsEnabled(pkghead_name);");
  while (pkglist.next())
    transfile << qMakePair(pkglist.value("pkghead_name").toString(), pkglist.value("pkghead_version").toString());
  ErrorReporter::error(QtCriticalMsg, 0, QObject::tr("Error Getting Extension Names"),
                       pkglist, __FILE__, __LINE__);

  translator = new QTranslator(&app);
  QPair<QString, QString> f;
  foreach (f, transfile)
  {
    foreach (QString l, lang)
    {
      if (translator->load(translationFile(l, f.first, f.second)))
      {
        app.installTranslator(translator);
        qDebug() << "installed" << l << f.first;
        translator = new QTranslator(&app);
        break;
      }
    }
  }
//}

  _splash->showMessage(QObject::tr("Loading Database Metrics"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _metrics = new Metrics();

  // TODO: we should compose the splash screen on the fly from parts
  QString edition("PostBooks");
  QHash<QString, QString> splashMap;
  splashMap.insert("Distribution",  ":/images/splashDistEdition.png");
  splashMap.insert("Enterprise",    ":/images/splashEnterprise.png");
  splashMap.insert("Manufacturing", ":/images/splashMfgEdition.png");
  splashMap.insert("PostBooks",     ":/images/splashPostBooks.png");

  XSqlQuery q("SELECT getEdition() AS result;");
  if (q.first())
  {
    edition = q.value("result").toString();
  }
  else
  {
    edition = _metrics->value("Application");
  }

  qDebug() << edition;
  _splash->setPixmap(QPixmap(splashMap[q.value("result").toString()]));

  _Name = _Name.arg(edition);

  _splash->showMessage(QObject::tr("Checking License Key"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();

  int cnt = 50000;
  int tot = 50000;

  XSqlQuery metric;
  metric.prepare("SELECT numOfDatabaseUsers(:appName) AS xt_client_count,"
                 "       numOfServerUsers() as total_client_count;");
  metric.bindValue(":appName", _ConnAppName);
  metric.exec();
  if(metric.first())
  {
    cnt = metric.value("xt_client_count").toInt();
    tot = metric.value("total_client_count").toInt();
  }
  else
  {
    ErrorReporter::error(QtCriticalMsg, 0, QObject::tr("Error Counting Users"),
                         metric, __FILE__, __LINE__);
  }
  metric.exec("SELECT packageIsEnabled('drupaluserinfo') AS result;");
  bool xtweb = false;
  if(metric.first())
    xtweb = metric.value("result").toBool();
  bool forceLimit = _metrics->boolean("ForceLicenseLimit");
  bool forced = false;
  bool checkPass = true;
  bool checkLock = false;
  bool expired   = false;
  bool invalid   = false;
  QString checkPassReason;
  QString rkey = _metrics->value("RegistrationKey");
  XTupleProductKey pkey(rkey);
  QString application = _metrics->value("Application");
  if(pkey.valid() && (pkey.version() == 1 || pkey.version() == 2 || pkey.version() == 3))
  {
    if(pkey.expiration() < QDate::currentDate())
    {
      checkPass = false;
      checkPassReason = QObject::tr("<p>Your license has expired.");
      if(!pkey.perpetual())
      {
        int daysTo = pkey.expiration().daysTo(QDate::currentDate());
        if(daysTo > 30)
        {
          checkLock = true;
          expired = true;
          checkPassReason = QObject::tr("<p>Your xTuple license expired over 30 days ago, and this software will no longer function. Please contact xTuple immediately to reinstate your software.");
        }
        else
          checkPassReason = QObject::tr("<p>Attention:  Your xTuple license has expired, and in %1 days this software will cease to function.  Please contact xTuple to arrange for license renewal.").arg(30 - daysTo);
      }
      else
        expired = true;
    }
    else if(application == "PostBooks" && pkey.users() == 1)
    {
      if(pkey.users() < cnt)
        {
        checkPass = false;
        checkPassReason = QObject::tr("<p>Multiple concurrent users of xTuple PostBooks require a license key. Please contact key@xtuple.com to request a free license key for your local installation, or sales@xtuple.com to purchase additional users in the xTuple Cloud Service. <p>Thank you.");
        checkLock = forced = forceLimit;
        }
    }
    else if(pkey.users() != 0 && (pkey.users() < cnt || (!xtweb && (pkey.users() * 2 < tot))))
    {
      if (pkey.users() < cnt) {
          checkPassReason = QObject::tr("<p>You have exceeded the number of allowed concurrent xTuple users for your license.</p>");
      } else {
          checkPassReason = QObject::tr("<p>You have exceeded the number of allowed concurrent database connections for your license.</p>");
      }

      checkPass = false;
      checkLock = forced = forceLimit;
    }
    else
    {
      int daysTo = QDate::currentDate().daysTo(pkey.expiration());
      if(!pkey.perpetual() && daysTo <= 15)
      {
        checkPass = false;
        checkPassReason = QObject::tr("<p>Please note: your xTuple license will expire in %1 days.  You should already have received your renewal invoice; please contact xTuple at your earliest convenience.").arg(daysTo);
      }
    }
  }
  else
  {
    checkPass = false;
    invalid   = true;
    checkPassReason = QObject::tr("<p>The Registration key installed for this system does not appear to be valid.");
  }
  if (! checkPass)
  {
    _splash->hide();
    if (invalid || expired)
    {
      ParameterList params;
      params.append("message", checkPassReason);
      if (invalid)
        params.append("invalid");

      registrationKeyDialog newdlg;
      newdlg.set(params);
      if (newdlg.exec() == QDialog::Rejected)
        return 0;
    }
    else if(checkLock)
    {
      QMessageBox::critical(0, QObject::tr("Registration Key"), checkPassReason);
      if(!forced)
        return 0;
    }
    else
    {
      if(QMessageBox::critical(0, QObject::tr("Registration Key"), QObject::tr("%1\n<p>Would you like to continue anyway?").arg(checkPassReason), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        return 0;
    }

    if(forced)
      checkPassReason.append(" FORCED!");

    metric.exec("SELECT current_database() AS db;");
    QString db = "";
    QString dbname = _metrics->value("DatabaseName");
    QString name   = _metrics->value("remitto_name");
    if(metric.first())
    {
      db = metric.value("db").toString();
    }
#if QT_VERSION >= 0x050000
    QUrlQuery urlQuery("https://www.xtuple.org/api/regviolation.php?");
    urlQuery.addQueryItem("key", rkey);
    urlQuery.addQueryItem("error", checkPassReason);
    urlQuery.addQueryItem("name", name);
    urlQuery.addQueryItem("dbname", dbname);
    urlQuery.addQueryItem("db", db);
    urlQuery.addQueryItem("cnt", QString::number(cnt));
    urlQuery.addQueryItem("tot", QString::number(tot));
    urlQuery.addQueryItem("ver", _Version);
    QUrl url = urlQuery.query();
#else
    QUrl url;
    url.setUrl("https://www.xtuple.org/api/regviolation.php");
    url.addQueryItem("key", QUrl::toPercentEncoding(rkey));
    url.addQueryItem("error", checkPassReason);
    url.addQueryItem("name", name);
    url.addQueryItem("dbname", dbname);
    url.addQueryItem("db", QUrl::toPercentEncoding(db));
    url.addQueryItem("cnt", QString::number(cnt));
    url.addQueryItem("tot", QString::number(tot));
    url.addQueryItem("ver", _Version);
#endif
    QMutex wait;
    xtNetworkRequestManager _networkManager(url, wait);
    if(forced)
      return 0;

    _splash->show();
  }

  QString _serverVersion = _metrics->value("ServerVersion");
  if (_serverVersion != _dbVersion) {
    bool shouldCheckForUpdates = _metrics->boolean("CheckForUpdates");
    bool disallowMismatch      = _metrics->boolean("DisallowMismatchClientVersion");
    int result = 0;
    if (shouldCheckForUpdates) {
      _splash->hide();
      checkForUpdates newdlg(0,"", true);
      result = newdlg.exec();
      if (result == QDialog::Rejected) {
        return 0;
      }
    }
    else if (!shouldCheckForUpdates && disallowMismatch) {
      _splash->hide();
      result = QMessageBox::warning( 0, QObject::tr("Version Mismatch"),
      QObject::tr("<p>The %1 client cannot connect to a %2 database. Please use a %3 client.").arg(_Version, _serverVersion, _serverVersion),
                  QMessageBox::Ok | QMessageBox::Escape | QMessageBox::Default );
      return 0;
    }
    else {
     _splash->hide();
     result = QMessageBox::warning( 0, QObject::tr("Version Mismatch"),
     QObject::tr("<p>This client was designed to work against database version %1 "
                 "and your database is currently at version %2. If you continue some functionality may "
                 "not work properly or at all. You may also cause other "
                 "problems on the database.<p>Do you want to continue "
                 "anyway?").arg(_Version, _serverVersion), QMessageBox::Yes,
                 QMessageBox::No | QMessageBox::Escape | QMessageBox::Default );
      if (result != QMessageBox::Yes) {
        return 0;
      }
      else {
        _splash->show();
      }
    }
  }

  _splash->showMessage(QObject::tr("Loading User Preferences"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _preferences = new Preferences(username);

  _splash->showMessage(QObject::tr("Loading User Privileges"), SplashTextAlignment, SplashTextColor);
  qApp->processEvents();
  _privileges = new Privileges();

  qApp->processEvents();

  QString key;
  QString keypath;
  QString keyname;
  QString keytogether;

#ifdef Q_OS_WIN
  keypath = _metrics->value("CCWinEncKey");
#elif defined Q_OS_MAC
  keypath = _metrics->value("CCMacEncKey");
#elif defined Q_OS_LINUX
  keypath = _metrics->value("CCLinEncKey");
#endif

  if (keypath.isEmpty())
    keypath = app.applicationDirPath();

  if (! keypath.endsWith(QDir::separator()))
    keypath += QDir::separator();

  keyname = _metrics->value("CCEncKeyName");
  if (keyname.isEmpty())
  {
    keyname = "xTuple.key";
    keytogether = keypath + keyname;
    QFile kn(keytogether);
    if(!kn.exists())
      keyname = "OpenMFG.key";
  }

  keytogether = keypath + keyname;

  QFile keyFile(keytogether);

  if(keyFile.exists())
  {
    if(keyFile.open(QIODevice::ReadOnly))
    {
      key = keyFile.readLine(1024);
      // strip off any newline characters
      key = key.trimmed();
    }
  }

  omfgThis = new GUIClient(databaseURL, username);
  omfgThis->_key = key;

  if (key.length() > 0) {
	_splash->showMessage(QObject::tr("Loading Database Encryption Metrics"), SplashTextAlignment, SplashTextColor);
	qApp->processEvents();
	_metricsenc = new Metricsenc(key);
  }

  initializePlugin(_preferences, _metrics, _privileges, omfgThis->username(), omfgThis->workspace());

// START code for updating the locale settings if they haven't been already
  XSqlQuery lc;
  lc.exec("SELECT count(*) FROM metric WHERE metric_name='AutoUpdateLocaleHasRun';");
  lc.first();
  if(lc.value(0).toInt() == 0)
  {
    lc.exec("INSERT INTO metric (metric_name, metric_value) values('AutoUpdateLocaleHasRun', 't');");
    lc.exec("SELECT locale_id from locale;");
    while(lc.next())
    {
      ParameterList params;
      params.append("mode","edit");
      params.append("locale_id", lc.value(0));
      sysLocale lcdlg;
      lcdlg.set(params);
      lcdlg.sSave();
    }
  }
// END code for updating locale settings

  QObject::connect(&app, SIGNAL(aboutToQuit()), &app, SLOT(closeAllWindows()));
  if (omfgThis->_singleWindow.isEmpty())
  {
    omfgThis->setAttribute(Qt::WA_DeleteOnClose);
    omfgThis->show();
  }
  // keep this synchronized with GUIClient and user.ui.h
  else if (omfgThis->_singleWindow == "woTimeClock")
  {
    ScriptToolbox sb(0);
    QWidget* newdlg = sb.openWindow("woTimeClock");
    if(newdlg)
    {
      XMainWindow *mw = qobject_cast<XMainWindow*>(newdlg);
      if(mw)
      {
        ParameterList params;
        params.append("captive");
        mw->set(params);
      }
      newdlg->setAttribute(Qt::WA_DeleteOnClose);
      QObject::connect(omfgThis, SIGNAL(destroyed(QObject*)), &app, SLOT(quit()));
      newdlg->show();
    }
    else
    {
      qDebug("Failed to initialize woTimeClock window.");
      return -2;
    }
  }
  else if (omfgThis->_singleWindow == "salesOrderSimple")
  {
    ParameterList params;
    params.append("mode", "new");

    salesOrderSimple *newdlg = new salesOrderSimple();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }

  // Check for the existance of a base currency, if none, one needs to
  // be selected or created
  XSqlQuery baseCurrency;
  baseCurrency.prepare("SELECT COUNT(*) AS count FROM curr_symbol WHERE curr_base=TRUE;");
  baseCurrency.exec();
  if(baseCurrency.first())
  {
    if(baseCurrency.value("count").toInt() != 1)
    {
      currenciesDialog newdlg(0, "", true);
      newdlg.exec();
      baseCurrency.exec();
      if(baseCurrency.first())
      {
        if(baseCurrency.value("count").toInt() != 1)
          return -1;
      }
      else
      {
        ErrorReporter::error(QtCriticalMsg, omfgThis, QObject::tr("Error Retrieving Base Currency Information"),
                             baseCurrency, __FILE__, __LINE__);
        // need to figure out appropriate return code for this...unusual error
        return -1;
      }
    }
  }
  else
  {
      ErrorReporter::error(QtCriticalMsg, omfgThis, QObject::tr("Error Retrieving Base Currency Information"),
                           baseCurrency, __FILE__, __LINE__);
    // need to figure out appropriate return code for this...unusual error
    return -1;
  }

  if(!omfgThis->singleCurrency() &&
     _metrics->value("GLCompanySize").toInt() == 0)
  {
    // Check for the gain/loss and discrep accounts
    main.prepare("SELECT COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((CAST(accnt_id AS text)=metric_value)"
              "                    AND  (metric_name='CurrencyGainLossAccount'))), FALSE)"
              "   AND COALESCE((SELECT TRUE"
              "                   FROM accnt, metric"
              "                  WHERE ((CAST(accnt_id AS text)=metric_value)"
              "                    AND  (metric_name='GLSeriesDiscrepancyAccount'))), FALSE) AS result; ");
    main.exec();
    if(main.first() && main.value("result").toBool() != true)
      QMessageBox::warning( omfgThis, QObject::tr("Additional Configuration Required"),
        QObject::tr("<p>Your system is configured to use multiple Currencies, "
                    "but the Currency Gain/Loss Account and/or the G/L Series "
                    "Discrepancy Account does not appear to be configured "
                    "correctly. You should define these Accounts in 'System | "
                    "Configure Modules | Configure G/L...' before posting any "
                    "transactions in the system.") );
  }

//  Check for valid current Fiscal period
  XSqlQuery periodCheck;
  periodCheck.prepare("SELECT EXISTS(SELECT 1 FROM period "
            "     WHERE ((current_date BETWEEN period_start AND period_end) "
            "       AND (NOT period_closed))) AS found; ");
  periodCheck.exec();
  if(periodCheck.first() && ! periodCheck.value("found").toBool())
  {
    createFiscalYear newdlg(NULL);
    (void)newdlg.exec();
  }

//  Check for valid current exchange rates
  XSqlQuery xrateCheck("SELECT curr_abbr"
	          "  FROM curr_symbol s JOIN curr_rate r ON s.curr_id = r.curr_id"
	          "  GROUP BY curr_abbr"
	          "  HAVING NOT BOOL_OR(current_date BETWEEN curr_effective AND curr_expires);");
  if (xrateCheck.first())
    QMessageBox::warning( omfgThis, QObject::tr("Additional Configuration Required"),
      QObject::tr("<p>Your system has alternate currencies without exchange rates "
                  "entered for the current date. "
                  "You should define the exchange rates for these currencies in 'System | "
                  "Setup | Exchange Rates...' before posting any "
                  "transactions in the system.") );

// Check for presence of password reset requirement and user last reset days
  XSqlQuery resetCheck("SELECT fetchmetricbool('EnforcePasswordReset') as passreset, "
                       "       fetchmetricvalue('PasswordResetDays')::TEXT as resetdays, "
                       "(SELECT current_date - fetchmetricvalue('PasswordResetDays')::INTEGER > "
                       "(SELECT usrpref_value FROM usrpref WHERE ((usrpref_username = geteffectivextuser()) "
                       " AND (usrpref_name = 'PasswordResetDate')))::DATE) AS lastreset;");
  resetCheck.exec();
  if(resetCheck.first())
  {
    if(resetCheck.value("passreset").toBool() && resetCheck.value("lastreset").toBool())
    {
      QMessageBox::warning( omfgThis, QObject::tr("New Password Required"),
        QObject::tr("<p>Your company has a policy of updating passwords every %1 days.  "
                  "Please change your password before logging out.").arg(resetCheck.value("resetdays").toString()));
      if (_privileges->check("MaintainPreferencesSelf"))
      {
        ParameterList params;
        params.append("passwordReset");
        userPreferences newdlg(0, "", true);
        newdlg.set(params);
        newdlg.exec();
      }
    }
  }

  app.exec();

//  Clean up
  delete _metrics;
  delete _preferences;
  delete _privileges;
  if (0 != _metricsenc)
    delete _metricsenc;

  return 0;
}

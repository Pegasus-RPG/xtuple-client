/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "checkForUpdates.h"

#include <QWebView>

#include "guiclient.h"
#include "version.h"

checkForUpdates::checkForUpdates(QWidget* parent, Qt::WFlags fl)
    : XWidget(parent, fl)
{
  setupUi(this);

  QStringList entries;
  entries.append("xtuple="+_Version);

  XSqlQuery versions;
  versions.exec("SELECT version() as pgver, fetchMetricText('ServerVersion') AS dbver, fetchMetricText('Application') AS dbprod;");
  if(versions.first())
  {
    entries.append("postgres="+versions.value("pgver").toString().replace(" ", "%20"));
    entries.append(versions.value("dbprod").toString()+"="+versions.value("dbver").toString());
  }
  versions.exec("select pkghead_name, pkghead_version from pkghead where packageisenabled(pkghead_id);");
  while(versions.next())
    entries.append(versions.value("pkghead_name").toString()+"="+versions.value("pkghead_version").toString());

  _view->load("http://updates.xtuple.com/checkForUpdates.php?"+entries.join("&"));
}

checkForUpdates::~checkForUpdates()
{
  // no need to delete child widgets, Qt does it all for us
}

void checkForUpdates::languageChange()
{
  retranslateUi(this);
}


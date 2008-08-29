/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "packages.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <dbtools.h>
#include <openreports.h>

#include "package.h"
#include "storedProcErrorLookup.h"

packages::packages(QWidget* parent, const char* name, Qt::WFlags fl)
    : XMainWindow(parent, name, fl)
{
  setupUi(this);

  connect(_autoUpdate, SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_close,   SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete,  SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_edit,    SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_load,    SIGNAL(clicked()), this, SLOT(sLoad()));
  connect(_new,     SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_package, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_print,   SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view,    SIGNAL(clicked()), this, SLOT(sView()));

  _package->addColumn(tr("Name"),    _itemColumn, Qt::AlignLeft, true, "pkghead_name");
  _package->addColumn(tr("Description"),      -1, Qt::AlignLeft, true, "pkghead_descrip");
  _package->addColumn(tr("Version"), _itemColumn, Qt::AlignRight,true, "pkghead_version");

  _load->setEnabled(package::userHasPriv(cNew));
  // TODO: spec says no editing of packages, only loading
  // It would be nice to be able to create new packages, fix 'em, and export 'em
  //_new->setEnabled(package::userHasPriv(cNew));
  //_edit->setEnabled(package::userHasPriv(cEdit));
  _new->setVisible(false);
  _edit->setVisible(false);

  if (package::userHasPriv(cEdit))
  {
    disconnect(_package, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    connect(_package,      SIGNAL(valid(bool)), _edit,  SLOT(setEnabled(bool)));
    connect(_package,SIGNAL(itemSelected(int)), _view,  SLOT(animateClick()));
  }
  else
    connect(_package, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

  if (package::userHasPriv(cNew))
    connect(_package, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));

  sHandleAutoUpdate(_autoUpdate->isChecked());
}

packages::~packages()
{
  // no need to delete child widgets, Qt does it all for us
}

void packages::languageChange()
{
  retranslateUi(this);
}

void packages::sFillList()
{
  q.prepare( "SELECT * "
             "FROM pkghead "
             "ORDER BY pkghead_name, pkghead_version DESC;" );
  q.bindValue(":days", tr("Days"));
  q.bindValue(":proximo", tr("Proximo"));
  q.exec();
  _package->populate(q);
}

void packages::sDelete()
{
  if (QMessageBox::question(this, tr("Delete Package?"),
                            tr("<p>Are you sure you want to delete the package "
                               "%1?<br>If you answer 'Yes' then you should "
                               "have backed up your database first.")
                            .arg(_package->currentItem()->text(0)),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::No)
    return;

  q.prepare( "SELECT deletePackage(:pkghead_id) AS result;" );
  q.bindValue(":pkghead_id", _package->id());
  q.exec();
  if (q.first())
  {
    int result = q.value("result").toInt();
    if (result < 0)
    {
      systemError(this, storedProcErrorLookup("deletePackage", result),
                  __FILE__, __LINE__);
      return;
    }
  }
  else if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

// TODO: implement the updater as a Qt application plugin and call it directly
// for now, launch it as an external program but try to preset the db connection info
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QProcess>
#include <QSqlDatabase>
void packages::sLoad()
{
  QProcess proc(this);
#ifdef Q_WS_MACX
  QString proc_path = qApp->applicationDirPath() +
                      "/../../../updater.app/Contents/MacOS/updater" ;
#else
  QString proc_path = qApp->applicationDirPath() + QDir::separator() + "updater";
#endif
  if (! QFile::exists(proc_path))
  {
#ifdef Q_WS_MACX
    if (QMessageBox::question(this, tr("Could Not Find Updater"),
                              tr("<p>xTuple ERP could not find the Updater "
                                 "application. Would you like to look for it?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::No)
      return;
#endif
    proc_path = QFileDialog::getOpenFileName(this,
                                             tr("Find Updater Application"));
    if (proc_path.isEmpty())
      return;
#ifdef Q_WS_MACX
    proc_path += "/Contents/MacOS/updater";
#endif
  }

  QStringList proc_args;
  QSqlDatabase db = QSqlDatabase::database();
  QString dbURL;
  buildDatabaseURL(dbURL, "QPSQL", db.hostName(), db.databaseName(),
                   QString::number(db.port()));
  proc_args << "-databaseURL=" + dbURL;
  if (! db.userName().isEmpty())
    proc_args << "-username=" + db.userName();
  if (! db.password().isEmpty())
    proc_args << "-passwd=" + db.password();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  proc.start(proc_path, proc_args);
  if (proc.waitForStarted() &&
      proc.waitForFinished(-1) &&
      proc.exitStatus() == QProcess::NormalExit &&
      proc.exitCode() == 0)
  {
    QApplication::restoreOverrideCursor();
    sFillList();
  }
  else
  {
    if (! db.password().isEmpty())
    {
      proc_args.removeLast();
      proc_args << "-passwd=XXXXX";
    }
    QApplication::restoreOverrideCursor();
    systemError(this,
                tr("<p>There was an error running the Updater program: "
                   "<br>%1 %2<br><br><pre>%3</pre>")
		  .arg(proc_path)
                  .arg(proc_args.join(" "))
		  .arg(QString(proc.readAllStandardError())));
    return;
  }

  sFillList();
}

void packages::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  package newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void packages::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("pkghead_id", _package->id());

  package newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void packages::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("pkghead_id", _package->id());

  package newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void packages::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("View..."), this, SLOT(sView()), 0);
  pMenu->setItemEnabled(menuItem, package::userHasPriv(cView));

  menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
  pMenu->setItemEnabled(menuItem, package::userHasPriv(cNew));
}

void packages::sPrint()
{
  orReport report("PackageMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void packages::sHandleAutoUpdate(const bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  sFillList();
}

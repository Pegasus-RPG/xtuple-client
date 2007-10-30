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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#include "importXML.h"

#include <QDirIterator>
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "configureIE.h"
#include "storedProcErrorLookup.h"

bool importXML::userHasPriv()
{
  return _privleges->check("ImportXML");
}

void importXML::setVisible(bool visible)
{
  if (! visible)
    QMainWindow::setVisible(false);

  else if (! userHasPriv())
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    close();
  }
  else if (_metrics->value("XMLSuccessTreatment").isEmpty() ||
	   _metrics->value("XSLTLibrary").isEmpty()) // not configured properly
  {
    if (! configureIE::userHasPriv())
    {
      systemError(this,
		  tr("The application is not set up to perform XML Import. "
		     "Have an administrator configure XML Import before "
		     "trying to import data."),
		  __FILE__, __LINE__);
      close();
    }
    else if (QMessageBox::question(this, tr("Setup required"),
			      tr("<p>You must set up the application to "
				 "import XML data before trying to import "
				 "data. Would you like to do this now?"),
			      QMessageBox::Yes | QMessageBox::Default,
			      QMessageBox::No) == QMessageBox::Yes &&
	     configureIE(this, "", true).exec() == QDialog::Accepted)
      QMainWindow::setVisible(true);
    else
      close();
  }
  else
    QMainWindow::setVisible(true);
}

importXML::importXML(QWidget* parent, Qt::WindowFlags fl)
    : QMainWindow(parent, fl)
{
  setupUi(this);

  (void)statusBar();

  connect(_add,            SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_file, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_importAll,      SIGNAL(clicked()), this, SLOT(sImportAll()));
  connect(_importSelected, SIGNAL(clicked()), this, SLOT(sImportSelected()));

  _file->addColumn(tr("File Name"),     -1, Qt::AlignLeft   );
  _file->addColumn(tr("Status"), _ynColumn, Qt::AlignCenter );

  _defaultDir = _metrics->value(
#if defined Q_WS_MACX
				      "XMLDefaultDirMac"
#elif defined Q_WS_WIN
				      "XMLDefaultDirWindows"
#elif defined Q_WS_X11
				      "XMLDefaultDirLinux"
#endif
      );

  sFillList();
}

importXML::~importXML()
{
  // no need to delete child widgets, Qt does it all for us
}

void importXML::languageChange()
{
  retranslateUi(this);
}

void importXML::sFillList()
{
  if (! _defaultDir.isEmpty())
  {
    QStringList filters;
    filters << "*.xml" << "*.XML";
    QDirIterator iterator(_defaultDir, filters);
    XTreeWidgetItem *last = 0;
    for (int i = 0; iterator.hasNext(); i++)
      last = new XTreeWidgetItem(_file, last, i, QVariant(iterator.next()));
  }
}

void importXML::sAdd()
{
  QFileDialog newdlg(this, tr("Select File(s) to Import"), _defaultDir);
  newdlg.setFileMode(QFileDialog::ExistingFiles);
  QStringList filters;
  filters << tr("XML files (*.xml *.XML)") << tr("Any Files (*)");
  newdlg.setFilters(filters);
  if (newdlg.exec())
  {
    QStringList files = newdlg.selectedFiles();
    XTreeWidgetItem *last = 0;
    for (int i = 0; i < files.size(); i++)
      last = new XTreeWidgetItem(_file, last, i, QVariant(files[i]));
  }
}

void importXML::sDelete()
{
  QList<QTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    _file->takeTopLevelItem(_file->indexOfTopLevelItem(selected[i]));
}

void importXML::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Import Selected"), this, SLOT(sImportSelected()), 0);
  menuItem = pMenu->insertItem(tr("Delete From List"), this, SLOT(sDelete()), 0);
}

void importXML::sImportAll()
{
  for (int i = 0; i < _file->topLevelItemCount(); i++)
    importOne(_file->topLevelItem(i));
}

void importXML::sImportSelected()
{
  QList<QTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = 0; i < selected.size(); i++)
    importOne(selected[i]);
}

bool importXML::importOne(QTreeWidgetItem *pItem)
{
  QString suffix = _metrics->value("XMLSuccessSuffix");
  if (suffix.isEmpty())
    suffix = ".done";
  bool remove = (_metrics->value("XMLSuccessTreatment") == "Delete");

  QString filename = pItem->data(0, Qt::DisplayRole).toString();

  if ( QMessageBox::information(this, "", tr("import %1").arg(filename)) )
  {
    pItem->setData(1, Qt::DisplayRole, tr("Done"));
    if (remove)
      return QFile::remove(filename);
    else
      return QFile::rename(filename, filename + suffix);
  }
  else
  {
    pItem->setData(1, Qt::DisplayRole, tr("Error"));
    return false;
  }
}

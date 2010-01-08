/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureIE.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "storedProcErrorLookup.h"
#include "xsltMap.h"

bool configureIE::userHasPriv()
{
  return _privileges->check("ConfigureImportExport");
}

int configureIE::exec()
{
  if (userHasPriv())
    return XDialog::exec();
  else
  {
    systemError(this,
		tr("You do not have sufficient privilege to view this window"),
		__FILE__, __LINE__);
    return XDialog::Rejected;
  }
}

configureIE::configureIE(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_deleteMap,	SIGNAL(clicked()), this, SLOT(sDeleteMap()));
  connect(_editMap,	SIGNAL(clicked()), this, SLOT(sEditMap()));
  connect(_map, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(sEditMap()));
  connect(_map,	  SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_newMap,	SIGNAL(clicked()), this, SLOT(sNewMap()));
  connect(_save,	SIGNAL(clicked()), this, SLOT(sSave()));

  _map->addColumn(tr("Name"),		  -1, Qt::AlignLeft, true, "xsltmap_name");
  _map->addColumn(tr("Document Type"),	  -1, Qt::AlignLeft, true, "xsltmap_doctype");
  _map->addColumn(tr("System Identifier"),-1, Qt::AlignLeft, true, "xsltmap_system");
  _map->addColumn(tr("Import XSLT File"), -1, Qt::AlignLeft, true, "xsltmap_import");
  _map->addColumn(tr("Export XSLT File"), -1, Qt::AlignLeft, true, "xsltmap_export");

  // TODO: fix these when support for an internal XSLT processor is enabled
  _internal->setEnabled(false);
  _internal->setVisible(false);
  _external->setVisible(false);
  _tabs->removePage(_tabs->page(2));
  
  adjustSize();

  sPopulate();
}

configureIE::~configureIE()
{
  // no need to delete child widgets, Qt does it all for us
}

void configureIE::languageChange()
{
  retranslateUi(this);
}

void configureIE::sSave()
{
  if (_renameFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Rename"));
  else if (_deleteFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Delete"));
  else if (_moveFiles->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("Move"));
  else if (_doNothing->isChecked())
    _metrics->set("XMLSuccessTreatment",  QString("None"));
  else
  {
    QMessageBox::critical(this, tr("Incomplete Data"),
			  tr("<p>Please choose whether to delete or rename "
			     "XML files after they have been successfully "
			     "imported."));
    _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
    _renameFiles->setFocus();
    return;
  }

  if (_internal->isChecked())
    _metrics->set("XSLTLibrary",	  _internal->isChecked());
  else if (_external->isChecked())
  {
    if (_linuxCmd->text().isEmpty() && _macCmd->text().isEmpty() &&
	_windowsCmd->text().isEmpty())
    {
      QMessageBox::critical(this, tr("Incomplete Data"),
			    tr("<p>Please enter the XSLT processor command "
			       "line for at least one platform."));
      _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
      _linuxCmd->setFocus();
      return;
    }
    else
      _metrics->set("XSLTLibrary",	  ! _external->isChecked());
  }
  else
  {
    QMessageBox::critical(this, tr("Incomplete Data"),
			  tr("<p>Please choose whether to use the internal "
			     "XSLT processor or an external XSLT processor."));
    _tabs->setCurrentIndex(_tabs->indexOf(_importTab));
    _internal->setFocus();
    return;
  }

  _metrics->set("XSLTDefaultDirLinux",	  _xsltLinuxDir->text());
  _metrics->set("XSLTDefaultDirMac",	  _xsltMacDir->text());
  _metrics->set("XSLTDefaultDirWindows",  _xsltWindowsDir->text());


  _metrics->set("XSLTProcessorLinux",	  _linuxCmd->text());
  _metrics->set("XSLTProcessorMac",	  _macCmd->text());
  _metrics->set("XSLTProcessorWindows",	  _windowsCmd->text());

  _metrics->set("XMLDefaultDirLinux",	  _linuxDir->text());
  _metrics->set("XMLDefaultDirMac",	  _macDir->text());
  _metrics->set("XMLDefaultDirWindows",	  _windowsDir->text());

  _metrics->set("XMLSuccessSuffix",	  _renameSuffix->text());
  _metrics->set("XMLSuccessDir",	  _moveDir->text());

  accept();
}

void configureIE::sPopulate()
{
  _xsltLinuxDir->setText(_metrics->value("XSLTDefaultDirLinux"));
  _xsltMacDir->setText(_metrics->value("XSLTDefaultDirMac"));
  _xsltWindowsDir->setText(_metrics->value("XSLTDefaultDirWindows"));

  /*  Will implement this if/when Xalan is embedded
  if (! _metrics->value("XSLTLibrary").isEmpty())
  {
    _internal->setChecked(_metrics->boolean("XSLTLibrary"));
    _external->setChecked(! _metrics->boolean("XSLTLibrary"));
  }
  */

  _external->setChecked(TRUE);

  _linuxCmd->setText(_metrics->value("XSLTProcessorLinux"));
  _macCmd->setText(_metrics->value("XSLTProcessorMac"));
  _windowsCmd->setText(_metrics->value("XSLTProcessorWindows"));

  _linuxDir->setText(_metrics->value("XMLDefaultDirLinux"));
  _macDir->setText(_metrics->value("XMLDefaultDirMac"));
  _windowsDir->setText(_metrics->value("XMLDefaultDirWindows"));

  if (_metrics->value("XMLSuccessTreatment") == "Rename")
    _renameFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "Delete")
    _deleteFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "Move")
    _moveFiles->setChecked(true);
  else if (_metrics->value("XMLSuccessTreatment") == "None")
    _doNothing->setChecked(true);

  _renameSuffix->setText(_metrics->value("XMLSuccessSuffix"));
  _moveDir->setText(_metrics->value("XMLSuccessDir"));

  sFillList();
}

void configureIE::sFillList()
{
  q.prepare("SELECT * FROM xsltmap ORDER BY xsltmap_name;");
  q.exec();
  _map->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void configureIE::sDeleteMap()
{
  if (QMessageBox::question(this, tr("Delete Map?"),
		    tr("Are you sure you want to delete this XSLT Map?"),
		    QMessageBox::Yes,
		    QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    q.prepare("DELETE FROM xsltmap WHERE (xsltmap_id=:mapid);");
    q.bindValue(":mapid", _map->id());
    q.exec();
    if (q.first())
    {
      int result = q.value("result").toInt();
      if (result < 0)
      {
	systemError(this, storedProcErrorLookup("deleteXSLTMap", result),
		    __FILE__, __LINE__);
	return;
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    sFillList();
  }
}

void configureIE::sEditMap()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("xsltmap_id", _map->id());
  params.append("defaultLinuxDir",   _xsltLinuxDir->text());
  params.append("defaultMacDir",     _xsltMacDir->text());
  params.append("defaultWindowsDir", _xsltWindowsDir->text());

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sNewMap()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("defaultLinuxDir",   _xsltLinuxDir->text());
  params.append("defaultMacDir",     _xsltMacDir->text());
  params.append("defaultWindowsDir", _xsltWindowsDir->text());

  xsltMap newdlg(this);
  if (newdlg.set(params) == NoError && newdlg.exec() == XDialog::Accepted)
    sFillList();
}

void configureIE::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("New..."),  this, SLOT(sNewMap()),    0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());

  menuItem = pMenu->insertItem(tr("Edit..."), this, SLOT(sEditMap()),   0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());

  menuItem = pMenu->insertItem(tr("Delete"),  this, SLOT(sDeleteMap()), 0);
  pMenu->setItemEnabled(menuItem, xsltMap::userHasPriv());
}

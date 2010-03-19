/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "importXML.h"

#include <QDirIterator>
#include <QMessageBox>
#include <QVariant>

#include "configureIE.h"
#include "importhelper.h"
#include "storedProcErrorLookup.h"

#define DEBUG false

bool importXML::userHasPriv()
{
  return _privileges->check("ImportXML");
}

void importXML::setVisible(bool visible)
{
  if (! visible)
    XWidget::setVisible(false);

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
      deleteLater();
    }
    else if (QMessageBox::question(this, tr("Setup required"),
                              tr("<p>You must set up the application to "
                                 "import XML data before trying to import "
                                 "data. Would you like to do this now?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::Yes &&
             configureIE(this, "", true).exec() == XDialog::Accepted)
      XWidget::setVisible(true);
    else
      deleteLater();
  }
  else
    XWidget::setVisible(true);
}

importXML::importXML(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_add,            SIGNAL(clicked()), this, SLOT(sAdd()));
  connect(_autoUpdate,  SIGNAL(toggled(bool)), this, SLOT(sHandleAutoUpdate(bool)));
  connect(_clearStatus,    SIGNAL(clicked()), this, SLOT(sClearStatus()));
  connect(_delete,         SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_file, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*)), this, SLOT(sPopulateMenu(QMenu*,QTreeWidgetItem*)));
  connect(_importAll,      SIGNAL(clicked()), this, SLOT(sImportAll()));
  connect(_importSelected, SIGNAL(clicked()), this, SLOT(sImportSelected()));
  connect(_resetList,      SIGNAL(clicked()), this, SLOT(sFillList()));

  _file->addColumn(tr("File Name"),     -1, Qt::AlignLeft   );
  _file->addColumn(tr("Status"), _ynColumn, Qt::AlignCenter );

  _defaultXMLDir = _metrics->value(
#if defined Q_WS_MACX
                                      "XMLDefaultDirMac"
#elif defined Q_WS_WIN
                                      "XMLDefaultDirWindows"
#elif defined Q_WS_X11
                                      "XMLDefaultDirLinux"
#endif
      );
  if (_defaultXMLDir.isEmpty())
    _defaultXMLDir = ".";

  _defaultXSLTDir = _metrics->value(
#if defined Q_WS_MACX
                                      "XSLTDefaultDirMac"
#elif defined Q_WS_WIN
                                      "XSLTDefaultDirWindows"
#elif defined Q_WS_X11
                                      "XSLTDefaultDirLinux"
#endif
      );
  _externalCmd = _metrics->value(
#if defined Q_WS_MACX
                                      "XSLTProcessorMac"
#elif defined Q_WS_WIN
                                      "XSLTProcessorWindows"
#elif defined Q_WS_X11
                                      "XSLTProcessorLinux"
#endif
      );

  sFillList();
  sHandleAutoUpdate(_autoUpdate->isChecked());
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
  _file->clear();
  if (! _defaultXMLDir.isEmpty())
  {
    QStringList filters;
    filters << "*.xml" << "*.XML";
    QDirIterator iterator(_defaultXMLDir, filters);
    XTreeWidgetItem *last = 0;
    for (int i = 0; iterator.hasNext(); i++)
      last = new XTreeWidgetItem(_file, last, i, QVariant(iterator.next()));
  }
}

void importXML::sAdd()
{
  QFileDialog newdlg(this, tr("Select File(s) to Import"), _defaultXMLDir);
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

void importXML::sClearStatus()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    selected[i]->setData(1, Qt::DisplayRole, tr(""));
}

void importXML::sDelete()
{
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = selected.size() - 1; i >= 0; i--)
    _file->takeTopLevelItem(_file->indexOfTopLevelItem(selected[i]));
}

void importXML::sPopulateMenu(QMenu* pMenu, QTreeWidgetItem* /* pItem */)
{
  int menuItem;

  menuItem = pMenu->insertItem(tr("Import Selected"),  this, SLOT(sImportSelected()), 0);
  menuItem = pMenu->insertItem(tr("Clear Status"),     this, SLOT(sClearStatus()), 0);
  menuItem = pMenu->insertItem(tr("Delete From List"), this, SLOT(sDelete()), 0);
}

void importXML::sImportAll()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  for (int i = 0; i < _file->topLevelItemCount(); i++)
  {
    QTreeWidgetItem* pItem = _file->topLevelItem(i);
    if (pItem->data(1, Qt::DisplayRole).toString().isEmpty())
    {
      if (importOne(pItem->data(0, Qt::DisplayRole).toString()))
        pItem->setData(1, Qt::DisplayRole, tr("Done"));
      else
        pItem->setData(1, Qt::DisplayRole, tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

void importXML::sImportSelected()
{
  bool oldAutoUpdate = _autoUpdate->isChecked();
  sHandleAutoUpdate(false);
  QList<XTreeWidgetItem*> selected = _file->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (selected[i]->data(1, Qt::DisplayRole).toString().isEmpty())
    {
      if (importOne(selected[i]->data(0, Qt::DisplayRole).toString()))
        selected[i]->setData(1, Qt::DisplayRole, tr("Done"));
      else
        selected[i]->setData(1, Qt::DisplayRole, tr("Error"));
    }
  }
  if (oldAutoUpdate)
    sHandleAutoUpdate(true);
}

bool importXML::importOne(const QString &pFileName)
{
  QString errmsg;
  if (! ImportHelper::importXML(pFileName, errmsg))
  {
    systemError(this, errmsg);
    return false;
  }

  return true;
}

void importXML::sHandleAutoUpdate(const bool pAutoUpdate)
{
  if (pAutoUpdate)
    connect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
  else
    disconnect(omfgThis, SIGNAL(tick()), this, SLOT(sFillList()));
}

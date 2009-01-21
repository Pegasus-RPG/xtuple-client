/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureAccountingSystem.h"

#include <qvariant.h>
#include <q3filedialog.h>
#include "guiclient.h"

/*
 *  Constructs a configureAccountingSystem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureAccountingSystem::configureAccountingSystem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_importPathList, SIGNAL(clicked()), this, SLOT(sImportPathList()));
    connect(_exportPathList, SIGNAL(clicked()), this, SLOT(sExportPathList()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_external, SIGNAL(toggled(bool)), _externalGroup, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureAccountingSystem::~configureAccountingSystem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureAccountingSystem::languageChange()
{
    retranslateUi(this);
}


void configureAccountingSystem::init()
{
#ifndef Q_WS_MAC
  _importPathList->setMaximumWidth(25);
  _exportPathList->setMaximumWidth(25);
#endif

  _accountingSystem->insertItem("PassPort RW2000 v6.7B");
  _accountingSystem->insertItem("RealWorld v9.1");
  _accountingSystem->insertItem("Other");

  if (_metrics->value("AccountingSystem") == "Native")
    _native->setChecked(TRUE);
  else
  {
    _external->setChecked(TRUE);

    if (_metrics->value("AccountingSystem") == "RW2000")
      _accountingSystem->setCurrentIndex(0);
    else if (_metrics->value("AccountingSystem") == "RealWorld91")
      _accountingSystem->setCurrentIndex(1);
    else if (_metrics->value("AccountingSystem") == "Other")
      _accountingSystem->setCurrentIndex(2);

    _importPath->setText(_metrics->value("AccountingSystemImportPath"));
    _exportPath->setText(_metrics->value("AccountingSystemExportPath"));
  }
}

void configureAccountingSystem::sSave()
{
  if (_native->isChecked())
    _metrics->set("AccountingSystem", QString("Native"));
  else
  {
    switch (_accountingSystem->currentIndex())
    {
      case 0:
        _metrics->set("AccountingSystem", QString("RW2000"));
        break;
  
      case 1:
        _metrics->set("AccountingSystem", QString("RealWorld91"));
        break;
  
      default:
        _metrics->set("AccountingSystem", QString("Other"));
        break;
    }

    _metrics->set("AccountingSystemImportPath", _importPath->text().trimmed());
    _metrics->set("AccountingSystemExportPath", _exportPath->text().trimmed());
  }

  _metrics->load();

  accept();
}

void configureAccountingSystem::sImportPathList()
{
  QString path = Q3FileDialog::getExistingDirectory( _importPath->text(), this, "selectImportPath",
                                                    tr("Select Accounting System Import Directory") );
  if (path.length())
    _importPath->setText(path);
}

void configureAccountingSystem::sExportPathList()
{
  QString path = Q3FileDialog::getExistingDirectory( _exportPath->text(), this, "selectExportPath",
                                                    tr("Select Accounting System Export Directory") );
  if (path.length())
    _exportPath->setText(path);
}


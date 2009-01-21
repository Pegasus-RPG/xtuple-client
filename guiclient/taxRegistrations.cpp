/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxRegistrations.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QSqlError>
#include <parameter.h>
#include <qworkspace.h>
#include "taxRegistration.h"

/*
 *  Constructs a taxRegistrations as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
taxRegistrations::taxRegistrations(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

//  statusBar()->hide();
  
  if (_privileges->check("MaintainChartOfAccounts"))
  {
    connect(_taxreg, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_taxreg, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_taxreg, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_taxreg, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _taxreg->addColumn(tr("Tax Authority"),  100,  Qt::AlignCenter, true,  "taxauth_code" );
  _taxreg->addColumn(tr("Registration #"),  -1,  Qt::AlignLeft,   true,  "taxreg_number"   );

  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxRegistrations::~taxRegistrations()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxRegistrations::languageChange()
{
  retranslateUi(this);
}

void taxRegistrations::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxRegistrations::sEdit()
{
  ParameterList params;
  params.append("taxreg_id", _taxreg->id());
  params.append("mode", "edit");

  taxRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  if(newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void taxRegistrations::sView()
{
  ParameterList params;
  params.append("taxreg_id", _taxreg->id());
  params.append("mode", "view");

  taxRegistration newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxRegistrations::sDelete()
{
  q.prepare("DELETE FROM taxreg"
            " WHERE (taxreg_id=:taxreg_id);");
  q.bindValue(":taxreg_id", _taxreg->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  sFillList();
}

void taxRegistrations::sFillList()
{
  q.prepare("SELECT taxreg_id, taxreg_taxauth_id, "
            "       taxauth_code, taxreg_number "
            "  FROM taxreg, taxauth "
            " WHERE ((taxreg_rel_type IS NULL) "
            "   AND  (taxreg_taxauth_id=taxauth_id));");
  q.exec();
  _taxreg->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}


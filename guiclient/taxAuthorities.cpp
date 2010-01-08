/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAuthorities.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <openreports.h>
#include "taxAuthority.h"

/*
 *  Constructs a taxAuthorities as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
taxAuthorities::taxAuthorities(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_taxauth, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(omfgThis, SIGNAL(taxAuthsUpdated(int)), this, SLOT(sFillList(int)));

  _taxauth->addColumn(tr("Code"), 70, Qt::AlignLeft,   true,  "taxauth_code" );
  _taxauth->addColumn(tr("Name"), -1, Qt::AlignLeft,   true,  "taxauth_name" );

  if (_privileges->check("MaintainTaxAuthorities"))
  {
    connect(_taxauth, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_taxauth, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_taxauth, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_taxauth, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  sFillList(-1);
}

/*
 *  Destroys the object and frees any allocated resources
 */
taxAuthorities::~taxAuthorities()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void taxAuthorities::languageChange()
{
    retranslateUi(this);
}

void taxAuthorities::sDelete()
{
  q.prepare("SELECT deleteTaxAuthority(:taxauth_id) AS result;");
  q.bindValue(":taxauth_id", _taxauth->id());
  q.exec();
  if (q.first())
  {
    if(q.value("result").toInt() < 0)
    {
      QMessageBox::warning( this, tr("Cannot Delete Tax Authority"),
                            tr( "You cannot delete the selected Tax Authority because there are currently items assigned to it.\n"
                                "You must first re-assign these items before deleting the selected Tax Authority." ) );
      return;
    }

    omfgThis->sTaxAuthsUpdated(_taxauth->id());
    sFillList(-1);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void taxAuthorities::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxAuthorities::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("taxauth_id", _taxauth->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void taxAuthorities::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("taxauth_id", _taxauth->id());

  taxAuthority newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void taxAuthorities::sPrint()
{
  orReport report("TaxAuthoritiesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void taxAuthorities::sFillList(int pId)
{
  _taxauth->populate( "SELECT taxauth_id, taxauth_code, taxauth_name "
                      "FROM taxauth "
                      "ORDER BY taxauth_code;", pId );
}


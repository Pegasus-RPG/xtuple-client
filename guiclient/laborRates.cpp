/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "laborRates.h"

#include <qvariant.h>
#include <qmessagebox.h>
//#include <qstatusbar.h>
#include <parameter.h>
#include <openreports.h>
#include "laborRate.h"

/*
 *  Constructs a laborRates as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
laborRates::laborRates(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_lbrrate, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
laborRates::~laborRates()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void laborRates::languageChange()
{
    retranslateUi(this);
}


void laborRates::init()
{
//  statusBar()->hide();
  
  _lbrrate->addColumn(tr("Labor Rate"),  _itemColumn, Qt::AlignLeft,   true,  "lbrrate_code"  );
  _lbrrate->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "lbrrate_descrip"  );
  _lbrrate->addColumn(tr("Rate"),        _costColumn, Qt::AlignRight,  true,  "lbrrate_rate" );

  if (_privileges->check("MaintainLaborRates"))
  {
    connect(_lbrrate, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_lbrrate, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_lbrrate, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_lbrrate, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  sFillList();
}

void laborRates::sPrint()
{
  orReport report("StdLaborRatesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void laborRates::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  laborRate newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void laborRates::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("lbrrate_id", _lbrrate->id());

  laborRate newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void laborRates::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("lbrrate_id", _lbrrate->id());

  laborRate newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void laborRates::sDelete()
{
  q.prepare( "SELECT wrkcnt_id "
             "FROM wrkcnt "
             "WHERE ( (wrkcnt_run_lbrrate_id=:lbrrate_id)"
             " OR (wrkcnt_setup_lbrrate_id=:lbrrate_id) ) "
             "LIMIT 1;" );
  q.bindValue(":lbrrate_id", _lbrrate->id());
  q.exec();
  if (q.first())
  {
    QMessageBox::information( this, tr("Cannot Delete Standard Labor Rate"),
                              tr( "The selected Standard Labor Rate cannot be deleted as it is in use at one or more Work Centers.\n"
                                  "You must first remove all references of the selected Standard Labor Rate before you may delete it.\n" ));
    return;
  }

  q.prepare( "DELETE FROM lbrrate "
             "WHERE (lbrrate_id=:lbrrate_id);" );
  q.bindValue(":lbrrate_id", _lbrrate->id());
  q.exec();

  sFillList();
}

void laborRates::sFillList()
{
  _lbrrate->populate( "SELECT lbrrate_id, lbrrate_code, lbrrate_descrip,"
                      "       lbrrate_rate, 'curr' AS lbrrate_rate_xtnumericrole "
                      "FROM lbrrate "
                      "ORDER BY lbrrate_code" );
}




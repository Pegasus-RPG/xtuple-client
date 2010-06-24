/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspSummarizedBankrecHistory.h"

#include <QVariant>
//#include <QStatusBar>
#include <openreports.h>
#include <parameter.h>
#include <metasql.h>

#include "guiclient.h"
#include "mqlutil.h"

/*
 *  Constructs a dspSummarizedBankrecHistory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspSummarizedBankrecHistory::dspSummarizedBankrecHistory(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_bankaccnt, SIGNAL(newID(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _bankrec->addColumn(tr("Posted"),             _ynColumn, Qt::AlignLeft,   true,  "bankrec_posted"   );
  _bankrec->addColumn(tr("Post Date"),        _dateColumn, Qt::AlignCenter, true,  "bankrec_postdate" );
  _bankrec->addColumn(tr("User"),                      -1, Qt::AlignLeft,   true,  "bankrec_username"   );
  _bankrec->addColumn(tr("Start Date"),       _dateColumn, Qt::AlignCenter, true,  "bankrec_opendate" );
  _bankrec->addColumn(tr("End Date"),         _dateColumn, Qt::AlignCenter, true,  "bankrec_enddate" );
  _bankrec->addColumn(tr("Opening Bal."), _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_openbal"  );
  _bankrec->addColumn(tr("Ending Bal."),  _bigMoneyColumn, Qt::AlignRight,  true,  "bankrec_endbal"  );
  
  _bankaccnt->populate("SELECT bankaccnt_id,"
                       "       (bankaccnt_name || '-' || bankaccnt_descrip) "
                       "FROM bankaccnt "
                       "ORDER BY bankaccnt_name;");
  
  sFillList();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspSummarizedBankrecHistory::~dspSummarizedBankrecHistory()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspSummarizedBankrecHistory::languageChange()
{
  retranslateUi(this);
}

void dspSummarizedBankrecHistory::sPrint()
{
  ParameterList params;
  params.append("bankaccnt_id", _bankaccnt->id());

  orReport report("SummarizedBankrecHistory", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspSummarizedBankrecHistory::sFillList()
{
  _bankrec->clear();
  ParameterList params;
  if(! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("summarizedBankrecHistory", "detail");
  q = mql.toQuery(params);
  _bankrec->populate(q);
}

bool dspSummarizedBankrecHistory::setParams(ParameterList & params)
{
  params.append("bankaccntid", _bankaccnt->id());
  return true;
}

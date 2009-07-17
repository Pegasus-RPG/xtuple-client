/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printAnnodizingPurchaseRequests.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>

printAnnodizingPurchaseRequests::printAnnodizingPurchaseRequests(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_cancel, SIGNAL(clicked()), this, SLOT(accept()));
}

printAnnodizingPurchaseRequests::~printAnnodizingPurchaseRequests()
{
  // no need to delete child widgets, Qt does it all for us
}

void printAnnodizingPurchaseRequests::languageChange()
{
  retranslateUi(this);
}

void printAnnodizingPurchaseRequests::sPrint()
{
  if(!_dates->allValid())
  {
    QMessageBox::warning( this, tr("Must Specify Dates"),
                          tr("One or both of the dates are either missing or invalid.") );
    return;
  }
  
  ParameterList params;
  _dates->appendValue(params);

  orReport report("PrintAnnodizingPurchaseRequests", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }
}

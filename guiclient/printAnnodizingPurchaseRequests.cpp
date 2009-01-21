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

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include <parameter.h>

/*
 *  Constructs a printAnnodizingPurchaseRequests as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printAnnodizingPurchaseRequests::printAnnodizingPurchaseRequests(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_cancel, SIGNAL(clicked()), this, SLOT(accept()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
printAnnodizingPurchaseRequests::~printAnnodizingPurchaseRequests()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
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

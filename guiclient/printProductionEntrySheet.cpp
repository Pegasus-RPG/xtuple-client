/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printProductionEntrySheet.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"
#include "inputManager.h"

printProductionEntrySheet::printProductionEntrySheet(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));

  _form->populate( "SELECT form_id, form_name "
                   "FROM form "
                   "WHERE (form_key='PES') "
                   "ORDER BY form_name;" );
}

printProductionEntrySheet::~printProductionEntrySheet()
{
  // no need to delete child widgets, Qt does it all for us
}

void printProductionEntrySheet::languageChange()
{
  retranslateUi(this);
}

void printProductionEntrySheet::sPrint()
{
  q.prepare( "SELECT form_report_name AS report_name "
             "FROM form "
             "WHERE ( (form_id=:form_id) );");
  q.bindValue(":form_id", _form->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    _warehouse->appendValue(params);
    _dates->appendValue(params);

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }
  }
  else
    QMessageBox::warning( this, tr("Cannot Print Production Entry Sheet"),
                          tr("Could not locate the report definition the form '%1'.")
                          .arg(_form->currentText()) );
}

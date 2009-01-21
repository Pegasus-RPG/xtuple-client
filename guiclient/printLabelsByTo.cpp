/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printLabelsByTo.h"

#include <QVariant>
#include <QMessageBox>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

printLabelsByTo::printLabelsByTo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_so, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
  connect(_from, SIGNAL(valueChanged(int)), this, SLOT(sSetToMin(int)));

  _so->setType(cToOpen | cToClosed);

  _report->populate( "SELECT labelform_id, labelform_name "
                     "FROM labelform "
                     "ORDER BY labelform_name;" );
}

printLabelsByTo::~printLabelsByTo()
{
  // no need to delete child widgets, Qt does it all for us
}

void printLabelsByTo::languageChange()
{
  retranslateUi(this);
}

void printLabelsByTo::sPrint()
{
  q.prepare( "SELECT report_name "
             "FROM labelform, report "
             "WHERE ( (labelform_id=:labelform_id) "
             " AND (report_id=labelform_report_id) );");
  q.bindValue(":labelform_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("tohead_id", _so->id());
    params.append("labelFrom", _from->value());
    params.append("labelTo", _to->value());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    _so->setId(-1);
    _so->setFocus();
  }
  else
    QMessageBox::warning(this, tr("Could not locate report"),
                         tr("Could not locate the report definition the form \"%1\"")
                         .arg(_report->currentText()) );
}

void printLabelsByTo::sSetToMin(int pValue)
{
  _to->setMinValue(pValue);
}

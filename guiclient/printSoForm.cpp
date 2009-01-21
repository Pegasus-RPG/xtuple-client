/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printSoForm.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include <parameter.h>
#include "guiclient.h"

/*
 *  Constructs a printSoForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printSoForm::printSoForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

 
    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_so, SIGNAL(valid(bool)), _print, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printSoForm::~printSoForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printSoForm::languageChange()
{
    retranslateUi(this);
}


enum SetResponse printSoForm::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _print->setFocus();
  }

  return NoError;
}

void printSoForm::init()
{

   _captive = FALSE; 

  _so->setType(cSoOpen | cSoClosed);

  _report->populate( "SELECT form_id, form_name "
                     "FROM form "
                     "WHERE (form_key='SO') "
                     "ORDER BY form_name;" );
}

void printSoForm::sPrint()
{
  q.prepare( "SELECT report_name "
             "  FROM report, form "
             " WHERE ( (form_id=:form_id) "
             "   AND (report_id=form_report_id) );");
  q.bindValue(":form_id", _report->id());
  q.exec();
  if (q.first())
  {
    ParameterList params;
    params.append("sohead_id", _so->id());

    orReport report(q.value("report_name").toString(), params);
    if (report.isValid())
      report.print();
    else
    {
      report.reportError(this);
      reject();
    }

    if (_captive)
      accept();
    else
    {
    _so->setId(-1);
    _so->setFocus();
    }
  }
  else
    QMessageBox::warning( this, tr("Could not locate report"),
                          tr("Could not locate the report definition the form \"%1\"")
                          .arg(_report->currentText()) );
}


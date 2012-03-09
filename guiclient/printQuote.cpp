/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printQuote.h"

#include <metasql.h>

#include "errorReporter.h"

printQuote::printQuote(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : printSinglecopyDocument(parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(optionsWidget()->windowTitle());
  
  setDoctype("QT");
  setReportKey("quhead_id");

  _docinfoQueryString = "SELECT quhead_id AS docid, quhead_id,"
                        "       quhead_number AS docnumber,"
                        "       false         AS printed,"
                        "<? if exists('reportname') ?>"
                        "       <? value('reportname') ?>"
                        "<? else ?>"
                        "       findCustomerForm(quhead_cust_id, 'Q')"
                        " <? endif ?> AS reportname"
                        "  FROM quhead"
                        " WHERE (quhead_id=<? value('docid') ?>);" ;

  connect(_quote,          SIGNAL(newId(int)), this, SLOT(sHandleNewQuoteId()));
  connect(_report,         SIGNAL(newID(int)), this, SLOT(sHandleButtons()));
  connect(this,     SIGNAL(finishedWithAll()), this, SLOT(sFinishedWithAll()));
  connect(this, SIGNAL(populated(XSqlQuery*)), this, SLOT(sPopulate(XSqlQuery*)));
}

printQuote::~printQuote()
{
}

void printQuote::languageChange()
{
  retranslateUi(this);
}

ParameterList printQuote::getParamsDocList()
{
  ParameterList params = printSinglecopyDocument::getParamsDocList();
  if (_report->isValid())
    params.append("reportname", _report->code());

  return params;
}

void printQuote::sFinishedWithAll()
{
  _quote->setId(-1);
}

void printQuote::sHandleButtons()
{
  setPrintEnabled(_quote->isValid() && _report->isValid());
}

void printQuote::sHandleNewQuoteId()
{
  if (_quote->isValid())
  {
    ParameterList params;
    params.append("docid", _quote->id());
    MetaSQLQuery mql(_docinfoQueryString);
    XSqlQuery    qry = mql.toQuery(params);
    if (qry.first())
      _report->setCode(qry.value("reportname").toString());
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Getting Quote Form"),
                                  qry, __FILE__, __LINE__))
      return;
  }
  sHandleButtons();
}

void printQuote::sPopulate(XSqlQuery *docq)
{
  _quote->setId(docq->value("docid").toInt());
  _report->setCode(docq->value("reportname").toString());
}

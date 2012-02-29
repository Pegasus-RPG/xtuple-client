/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "printCreditMemo.h"

#include <QDebug>
#include <QSqlRecord>
#include <QVariant>

printCreditMemo::printCreditMemo(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : printMulticopyDocument("CreditMemoCopies",     "CreditMemoWatermark",
                             "CreditMemoShowPrices", "PostARDocuments",
                             parent, name, modal, fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Print Credit Memo"));

  _doctypefull = tr("Credit Memo");
  _reportKey   = "cmhead_id";
  _distributeInventory = true;

  _docinfoQueryString =
             "SELECT cmhead_id      AS docid,"
             "       cmhead_number  AS docnumber,"
             "       cmhead_printed AS printed,"
             "       cmhead_posted  AS posted,"
             "       cmhead_cust_id,"
             "       findCustomerForm(cmhead_cust_id, 'C') AS reportname "
             "FROM cmhead "
             "WHERE (cmhead_id=<? value('docid') ?>);" ;

  _markOnePrintedQry = "UPDATE cmhead "
                       "   SET cmhead_printed=TRUE "
                       " WHERE (cmhead_id=<? value('docid') ?>);" ;
  _postFunction = "postCreditMemo";
  _postQuery    = "SELECT postCreditMemo(<? value('docid') ?>, 0) AS result;" ;

  connect(this, SIGNAL(docUpdated(int)),        this, SLOT(sHandleDocUpdated(int)));
  connect(this, SIGNAL(populated(QSqlRecord*)), this, SLOT(sHandlePopulated(QSqlRecord*)));
}

printCreditMemo::~printCreditMemo()
{
}

void printCreditMemo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse printCreditMemo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("cmhead_id", &valid);
  if (valid)
    setId(param.toInt());

  return printMulticopyDocument::set(pParams);
}

void printCreditMemo::sHandlePopulated(QSqlRecord *record)
{
  if (record)
  {
    _number->setText(record->value(record->indexOf("docnumber")).toString());
    _cust->setId(record->value(record->indexOf("cmhead_cust_id")).toInt());
  }
}

void printCreditMemo::sHandleDocUpdated(int docid)
{
  Q_UNUSED(docid);
  omfgThis->sCreditMemosUpdated();
}

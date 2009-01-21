/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "cmcluster.h"

CmCluster::CmCluster(QWidget *pParent, const char *pName) :
  VirtualCluster(pParent, pName)
{
  addNumberWidget(new CmLineEdit(this, pName));
  _info->hide();
}

void CmCluster::setCustId(int pcustid)
{
  return (static_cast<CmLineEdit*>(_number))->setCustId(pcustid);
}

CmLineEdit::CmLineEdit(QWidget *pParent, const char *pName) :
  VirtualClusterLineEdit(pParent, "aropen", "aropen_id", "aropen_docnumber", 
          "formatmoney(aropen_amount-aropen_paid- "
          
          //Subtract amount on existing checks
          "(SELECT COALESCE(SUM(checkhead_amount),0) "
          " FROM checkhead,checkitem "
          " WHERE ((checkhead_id=checkitem_checkhead_id) "
          " AND (NOT checkhead_posted) "
          " AND (NOT checkhead_void) "
          " AND (checkitem_aropen_id=aropen_id))) "
          
          ")"
          
          , 0, 
          " AND (aropen_doctype='C') AND (aropen_open) "
          " AND aropen_amount - aropen_paid - "
          "(SELECT COALESCE(SUM(checkhead_amount),0) "
          " FROM checkhead,checkitem "
          " WHERE ((checkhead_id=checkitem_checkhead_id) "
          " AND (NOT checkhead_posted) "
          " AND (NOT checkhead_void) "
          " AND (checkitem_aropen_id=aropen_id))) > 0 "
          , pName)
{
  setTitles(tr("Credit Memo"), tr("Credit Memos"));
}

void CmLineEdit::setCustId(int pCust)
{
  {
    _custId = pCust;
    setExtraClause(QString(" (aropen_cust_id='%1') "
                  " AND (aropen_doctype='C') AND (aropen_open) "
                  " AND aropen_amount - aropen_paid - "
                  "(SELECT COALESCE(SUM(checkhead_amount),0) "
                  " FROM checkhead,checkitem "
                  " WHERE ((checkhead_id=checkitem_checkhead_id) "
                  " AND (NOT checkhead_posted) "
                  " AND (NOT checkhead_void) "
                  " AND (checkitem_aropen_id=aropen_id))) > 0 ").arg(pCust));
  }
}


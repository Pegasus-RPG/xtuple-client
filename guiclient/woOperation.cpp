/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "woOperation.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include "inputManager.h"

woOperation::woOperation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_stdopn, SIGNAL(newID(int)), this, SLOT(sHandleStdopn(int)));
  connect(_fixedFont, SIGNAL(toggled(bool)), this, SLOT(sHandleFont(bool)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_runTime, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
  connect(_invProdUOMRatio, SIGNAL(textChanged(const QString&)), this, SLOT(sCalculateInvRunTime()));
  connect(_wo, SIGNAL(newId(int)), this, SLOT(sPopulateWoInfo(int)));

  _qtyOrdered->setPrecision(omfgThis->qtyVal());
  _setupTime->setValidator(omfgThis->runTimeVal());
  _runTime->setValidator(omfgThis->runTimeVal());
  _invProdUOMRatio->setValidator(omfgThis->ratioVal());
  _invRunTime->setPrecision(omfgThis->runTimeVal());
  _invPerMinute->setPrecision(omfgThis->runTimeVal());
  _setupTimeConsumed->setPrecision(omfgThis->runTimeVal());
  _runTimeConsumed->setPrecision(omfgThis->runTimeVal());
  _setupTimeRemaining->setPrecision(omfgThis->runTimeVal());
  _runTimeRemaining->setPrecision(omfgThis->runTimeVal());

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  sHandleFont(_fixedFont->isChecked());
  
  _wo->setType(cWoExploded | cWoReleased | cWoIssued);

  _prodUOM->setType(XComboBox::UOMs);
  _wrkcnt->setType(XComboBox::WorkCenters);

  _stdopn->populate( "SELECT -1, TEXT('None') AS stdopn_number "
                     "UNION SELECT stdopn_id, stdopn_number "
                     "FROM stdopn "
                     "ORDER BY stdopn_number" );
}

woOperation::~woOperation()
{
  // no need to delete child widgets, Qt does it all for us
}

void woOperation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse woOperation::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _wo->setId(param.toInt());
    _wo->setReadOnly(TRUE);
  }

  param = pParams.value("wooper_id", &valid);
  if (valid)
  {
    _wooperid = param.toInt();
    _wo->setReadOnly(TRUE);

    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      _setupTimeConsumedLit->setEnabled(FALSE);
      _setupTimeRemainingLit->setEnabled(FALSE);
      _setupComplete->setEnabled(FALSE);
      _runTimeConsumedLit->setEnabled(FALSE);
      _runTimeRemainingLit->setEnabled(FALSE);
      _runComplete->setEnabled(FALSE);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _wo->setReadOnly(TRUE);
      _executionDay->setEnabled(FALSE);
      _stdopn->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _toolingReference->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _setupTime->setEnabled(FALSE);
      _runTime->setEnabled(FALSE);
      _reportSetup->setEnabled(FALSE);
      _reportRun->setEnabled(FALSE);
      _receiveStock->setEnabled(FALSE);
      _issueComp->setEnabled(FALSE);
      _setupComplete->setEnabled(FALSE);
      _runComplete->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
      _fixedFont->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void woOperation::sSave()
{
  if (_receiveStock->isChecked())
  {
    q.prepare( "UPDATE wooper "
               "SET wooper_rcvinv=FALSE "
               "WHERE (wooper_wo_id=:wo_id);" );
    q.bindValue(":wo_id", _wo->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('wooper_wooper_id_seq') AS wooper_id;");
    if (q.first())
      _wooperid = q.value("wooper_id").toInt();
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO wooper "
               "( wooper_id, wooper_wo_id,"
               "  wooper_seqnumber,"
               "  wooper_wrkcnt_id, wooper_stdopn_id,"
               "  wooper_descrip1, wooper_descrip2, wooper_toolref,"
               "  wooper_produom, wooper_invproduomratio,"
               "  wooper_sutime, wooper_surpt, wooper_sucomplete, wooper_suconsumed,"
               "  wooper_rntime, wooper_rnqtyper,"
               "  wooper_rnrpt, wooper_rncomplete, wooper_rnconsumed,"
               "  wooper_rcvinv, wooper_issuecomp,"
               "  wooper_scheduled,"
               "  wooper_qtyrcv,"
               "  wooper_instruc )"
               "SELECT :wooper_id, :wo_id,"
               "       (COALESCE(MAX(wooper_seqnumber), 0) + 10),"
               "       :wooper_wrkcnt_id, :wooper_stdopn_id,"
               "       :wooper_descrip1, :wooper_descrip2, :wooper_toolref,"
               "       :wooper_produom, :wooper_invproduomratio,"
               "       :wooper_sutime, :wooper_surpt, :wooper_sucomplete, 0,"
               "       :wooper_rntime, :wooper_rnqtyper,"
               "       :wooper_rnrpt, :wooper_rncomplete, 0,"
               "       :wooper_rcvinv, :wooper_issuecomp,"
               "       (wo_startdate + :executionDay),"
               "       0.0,"
               "       :wooper_instruc "
               "FROM wo LEFT OUTER JOIN wooper ON (wooper_wo_id=wo_id) "
               "WHERE (wo_id=:wo_id) "
               "GROUP BY wo_startdate;" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE wooper "
               "SET wooper_wrkcnt_id=:wooper_wrkcnt_id, wooper_stdopn_id=:wooper_stdopn_id,"
               "    wooper_descrip1=:wooper_descrip1, wooper_descrip2=:wooper_descrip2, wooper_toolref=:wooper_toolref,"
               "    wooper_produom=:wooper_produom, wooper_invproduomratio=:wooper_invproduomratio,"
               "    wooper_sutime=:wooper_sutime, wooper_surpt=:wooper_surpt, wooper_sucomplete=:wooper_sucomplete,"
               "    wooper_rntime=:wooper_rntime, wooper_rnqtyper=:wooper_rnqtyper,"
               "    wooper_rnrpt=:wooper_rnrpt, wooper_rncomplete=:wooper_rncomplete,"
               "    wooper_rcvinv=:wooper_rcvinv, wooper_issuecomp=:wooper_issuecomp,"
               "    wooper_scheduled=(wo_startdate + :executionDay - 1),"
               "    wooper_instruc=:wooper_instruc "
               "FROM wo "
               "WHERE ( (wooper_wo_id=wo_id)"
               " AND (wooper_id=:wooper_id) );" );

  q.bindValue(":wooper_id", _wooperid);
  q.bindValue(":wo_id", _wo->id());
  q.bindValue(":wooper_wrkcnt_id", _wrkcnt->id());
  q.bindValue(":wooper_stdopn_id", _stdopn->id());
  q.bindValue(":wooper_descrip1", _description1->text());
  q.bindValue(":wooper_descrip2", _description2->text());
  q.bindValue(":wooper_toolref", _toolingReference->text());
  q.bindValue(":wooper_produom", _prodUOM->currentText());
  q.bindValue(":wooper_invproduomratio", _invProdUOMRatio->toDouble());
  q.bindValue(":wooper_sutime", _setupTime->toDouble());
  q.bindValue(":wooper_rntime", _runTime->toDouble());
  q.bindValue(":wooper_rnqtyper", (_runTime->toDouble() / _cachedQtyOrdered));
  q.bindValue(":executionDay", _executionDay->value());
  q.bindValue(":wooper_instruc", _instructions->toPlainText());
  q.bindValue(":wooper_surpt", QVariant(_reportSetup->isChecked()));
  q.bindValue(":wooper_sucomplete", QVariant(_setupComplete->isChecked()));
  q.bindValue(":wooper_rnrpt", QVariant(_reportRun->isChecked()));
  q.bindValue(":wooper_rncomplete", QVariant(_runComplete->isChecked()));
  q.bindValue(":wooper_rcvinv", QVariant(_receiveStock->isChecked()));
  q.bindValue(":wooper_issuecomp", QVariant(_issueComp->isChecked()));
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  omfgThis->sWorkOrderOperationsUpdated(_wo->id(), _wooperid, TRUE);

  done(_wooperid);
}

void woOperation::sHandleFont(bool pFixed)
{
  if (pFixed)
    _instructions->setFont(omfgThis->fixedFont());
  else
    _instructions->setFont(omfgThis->systemFont());
}

void woOperation::sHandleStdopn(int pStdopnid)
{
  if (_stdopn->id() != -1)
  {
    q.prepare( "SELECT * "
               "FROM stdopn "
               "WHERE (stdopn_id=:stdopn_id);" );
    q.bindValue(":stdopn_id", pStdopnid);
    q.exec();
    if (q.first())
    {
      _description1->setText(q.value("stdopn_descrip1"));
      _description2->setText(q.value("stdopn_descrip2"));
      _toolingReference->setText(q.value("stdopn_toolref"));
      _instructions->setText(q.value("stdopn_instructions").toString());

      _wrkcnt->setId(q.value("stdopn_wrkcnt_id").toInt());

      if (q.value("stdopn_stdtimes").toBool())
      {
        _setupTime->setDouble(q.value("stdopn_sutime").toDouble());
        _prodUOM->setText(q.value("stdopn_produom"));
        _invProdUOMRatio->setDouble(q.value("stdopn_invproduomratio").toDouble());
      }
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void woOperation::sCalculateInvRunTime()
{
  if ((_runTime->toDouble() != 0.0) && (_invProdUOMRatio->toDouble() != 0.0))
  {
    _invRunTime->setDouble(_runTime->toDouble() / _cachedQtyOrdered / _invProdUOMRatio->toDouble());
    _invPerMinute->setDouble((1 / (_runTime->toDouble() / _cachedQtyOrdered / _invProdUOMRatio->toDouble())));
  }
  else
  {
    _invRunTime->setDouble(0.0);
    _invPerMinute->setDouble(0.0);
  }
}

void woOperation::sPopulateWoInfo(int pWoid)
{
  if(pWoid == -1)
    return;

  q.prepare( "SELECT wo_qtyord, uom_name "
             "FROM wo, itemsite, item, uom "
             "WHERE ( (wo_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=item_id)"
             " AND (item_inv_uom_id=uom_id)"
             " AND (wo_id=:wo_id) );" );
  q.bindValue(":wo_id", pWoid);
  q.exec();
  if (q.first())
  {
    _cachedQtyOrdered = q.value("wo_qtyord").toDouble();

    _invUOM1->setText(q.value("uom_name").toString());
    _invUOM2->setText(q.value("uom_name").toString());
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}


void woOperation::populate()
{
  XSqlQuery wooper;
  wooper.prepare( "SELECT wooper_wo_id, wooper_seqnumber, wooper_wrkcnt_id, wooper_stdopn_id,"
                  "       wooper_descrip1, wooper_descrip2, wooper_toolref,"
                  "       wooper_produom, wooper_invproduomratio,"
                  "       wooper_sutime, wooper_surpt,"
                  "       wooper_rntime, wooper_rnrpt,"
                  "       wooper_suconsumed, wooper_sucomplete,"
                  "       wooper_rnconsumed, wooper_rnconsumed, wooper_rncomplete,"
                  "       noNeg(wooper_sutime - wooper_suconsumed) AS suremaining,"
                  "       noNeg(wooper_rntime - wooper_rnconsumed) AS rnremaining,"
                  "       wooper_rcvinv, wooper_issuecomp,"
                  "       (DATE(wooper_scheduled) - wo_startdate + 1) AS executionday,"
                  "       wooper_instruc "
                  "FROM wooper, wo "
                  "WHERE ( (wooper_wo_id=wo_id)"
                  " AND (wooper_id=:wooper_id) );" );
  wooper.bindValue(":wooper_id", _wooperid);
  wooper.exec();
  if (wooper.first())
  {
    _wo->setId(wooper.value("wooper_wo_id").toInt());
    _stdopn->setId(wooper.value("wooper_stdopn_id").toInt());
    _operSeqNum->setText(wooper.value("wooper_seqnumber").toString());
    _description1->setText(wooper.value("wooper_descrip1"));
    _description2->setText(wooper.value("wooper_descrip2"));
    _prodUOM->setText(wooper.value("wooper_produom"));
    _invProdUOMRatio->setDouble(wooper.value("wooper_invproduomratio").toDouble());
    _toolingReference->setText(wooper.value("wooper_toolref"));
    _wrkcnt->setId(wooper.value("wooper_wrkcnt_id").toInt());

    _setupTime->setText(wooper.value("wooper_sutime").toDouble());
    _setupTimeConsumed->setText(wooper.value("wooper_suconsumed").toDouble());
    _setupComplete->setChecked(wooper.value("wooper_sucomplete").toBool());
    _setupTimeRemaining->setText(wooper.value("suremaining").toDouble());
    _reportSetup->setChecked(wooper.value("wooper_surpt").toBool());

    _runTime->setText(wooper.value("wooper_rntime").toDouble());
    _runTimeConsumed->setText(wooper.value("wooper_rnconsumed").toDouble());
    _runComplete->setChecked(wooper.value("wooper_rncomplete").toBool());
    _runTimeRemaining->setText(wooper.value("rnremaining").toDouble());
    _reportRun->setChecked(wooper.value("wooper_rnrpt").toBool());

    _receiveStock->setChecked(wooper.value("wooper_rcvinv").toBool());
    _issueComp->setChecked(wooper.value("wooper_issuecomp").toBool());
    _executionDay->setValue(wooper.value("executionday").toInt());
    _instructions->setText(wooper.value("wooper_instruc").toString());

    if ((wooper.value("wooper_suconsumed").toDouble() > 0) || (wooper.value("wooper_rnconsumed").toDouble() > 0))
    {
      _stdopn->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _toolingReference->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
    }
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

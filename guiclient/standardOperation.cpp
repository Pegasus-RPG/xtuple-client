/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "standardOperation.h"

#include <QVariant>
#include <QMessageBox>

standardOperation::standardOperation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_stdTimes, SIGNAL(toggled(bool)), _setupTime, SLOT(setEnabled(bool)));
  connect(_stdTimes, SIGNAL(toggled(bool)), _runTime, SLOT(setEnabled(bool)));
  connect(_stdTimes, SIGNAL(toggled(bool)), _runQtyPer, SLOT(setEnabled(bool)));
  connect(_stdTimes, SIGNAL(toggled(bool)), _setupReport, SLOT(setEnabled(bool)));
  connect(_stdTimes, SIGNAL(toggled(bool)), _runReport, SLOT(setEnabled(bool)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_wrkcnt, SIGNAL(newID(int)), this, SLOT(sHandleWorkCenter()));
  connect(_stdTimes, SIGNAL(toggled(bool)), _reportSetup, SLOT(setEnabled(bool)));
  connect(_stdTimes, SIGNAL(toggled(bool)), _reportRun, SLOT(setEnabled(bool)));
  connect(_number, SIGNAL(lostFocus()), this, SLOT(sCheck()));
    
  _invProdUOMRatio->setValidator(omfgThis->ratioVal());
  _setupTime->setValidator(omfgThis->runTimeVal());
  _runTime->setValidator(omfgThis->runTimeVal());
  _runQtyPer->setValidator(omfgThis->qtyPerVal());
    

  _prodUOM->setType(XComboBox::UOMs);

  _wrkcnt->populate( "SELECT -1 AS wrkcnt_id, 'Any' AS wrkcnt_code, 1 AS orderby "
                     "UNION SELECT wrkcnt_id, wrkcnt_code, 2 AS orderby "
                     "FROM wrkcnt "
                     "ORDER BY orderby, wrkcnt_code;" );

  _setupReport->insertItem(tr("Direct Labor"));
  _setupReport->insertItem(tr("Overhead"));
  _setupReport->insertItem(tr("None"));
  _setupReport->setCurrentIndex(-1);

  _runReport->insertItem(tr("Direct Labor"));
  _runReport->insertItem(tr("Overhead"));
  _runReport->insertItem(tr("None"));
  _runReport->setCurrentIndex(-1);
}

standardOperation::~standardOperation()
{
  // no need to delete child widgets, Qt does it all for us
}

void standardOperation::languageChange()
{
  retranslateUi(this);
}

enum SetResponse standardOperation::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("stdopn_id", &valid);
  if (valid)
  {
    _stdopnid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _number->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _number->setEnabled(FALSE);
      _description1->setEnabled(FALSE);
      _description2->setEnabled(FALSE);
      _wrkcnt->setEnabled(FALSE);
      _prodUOM->setEnabled(FALSE);
      _invProdUOMRatio->setEnabled(FALSE);
      _toolReference->setEnabled(FALSE);
      _stdTimes->setEnabled(FALSE);
      _setupTime->setEnabled(FALSE);
      _setupReport->setEnabled(FALSE);
      _reportSetup->setEnabled(FALSE);
      _runTime->setEnabled(FALSE);
      _runReport->setEnabled(FALSE);
      _reportRun->setEnabled(FALSE);
      _runQtyPer->setEnabled(FALSE);
      _instructions->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();

      _close->setFocus();
    }
  }

  return NoError;
}

void standardOperation::sCheck()
{
  if ((_mode == cNew) && (_number->text().length() != 0))
  {
    q.prepare( "SELECT stdopn_id "
               "FROM stdopn "
               "WHERE (UPPER(stdopn_number)=UPPER(:stdopn_number));" );
    q.bindValue(":stdopn_number", _number->text().trimmed());
    q.exec();
    if (q.first())
    {
      _stdopnid = q.value("stdopn_id").toInt();
      _mode = cEdit;
      populate();

      _number->setEnabled(FALSE);
    }
  }
}

void standardOperation::sSave()
{
  struct {
    bool	condition;
    QString	msg;
    QWidget*	widget;
  } error[] = {
    { _number->text().trimmed().isEmpty(),
      tr("You must supply a Std. Oper. #."), _number },
    { _stdTimes->isChecked() &&
      _setupTime->toDouble() <= 0 && _runTime->toDouble() <= 0,
      tr("If you select Use Standard Times then please enter at least a Setup "
	 "or a Run Time."), _setupTime },
    { true, "", NULL }
  }; // error[]

  int errIndex;
  for (errIndex = 0; ! error[errIndex].condition; errIndex++)
    ;
  if (! error[errIndex].msg.isEmpty())
  {
    QMessageBox::critical(this, tr("Cannot Save Standard Operation"),
			  error[errIndex].msg);
    error[errIndex].widget->setFocus();
    return;
  }


  if (_mode == cNew)
  {
    q.exec("SELECT NEXTVAL('stdopn_stdopn_id_seq') AS _stdopn_id;");
    if (q.first())
      _stdopnid = q.value("_stdopn_id").toInt();
    else
    {
      systemError( this, tr("A System Error occurred at standardOperation::%1.")
                         .arg(__LINE__) );
      reject();
      return;
    }
    
    q.prepare( "INSERT INTO stdopn "
               "( stdopn_id, stdopn_number,"
               "  stdopn_descrip1, stdopn_descrip2,"
               "  stdopn_wrkcnt_id, stdopn_toolref, stdopn_stdtimes,"
               "  stdopn_produom, stdopn_invproduomratio,"
               "  stdopn_sutime, stdopn_sucosttype, stdopn_reportsetup,"
               "  stdopn_rntime, stdopn_rncosttype, stdopn_reportrun,"
               "  stdopn_rnqtyper, stdopn_instructions ) "
               "VALUES "
               "( :stdopn_id, :stdopn_number,"
               "  :stdopn_descrip1, :stdopn_descrip2,"
               "  :stdopn_wrkcnt_id, :stdopn_toolref, :stdopn_stdtimes,"
               "  :stdopn_produom, :stdopn_invproduomratio,"
               "  :stdopn_sutime, :stdopn_sucosttype, :stdopn_reportsetup,"
               "  :stdopn_rntime, :stdopn_rncosttype, :stdopn_reportrun,"
               "  :stdopn_rnqtyper, :stdopn_instructions );" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE stdopn "
               "SET stdopn_number=:stdopn_number, stdopn_wrkcnt_id=:stdopn_wrkcnt_id,"
               "    stdopn_descrip1=:stdopn_descrip1, stdopn_descrip2=:stdopn_descrip2,"
               "    stdopn_toolref=:stdopn_toolref, stdopn_stdtimes=:stdopn_stdtimes,"
               "    stdopn_produom=:stdopn_produom, stdopn_invproduomratio=:stdopn_invproduomratio,"
               "    stdopn_sutime=:stdopn_sutime, stdopn_sucosttype=:stdopn_sucosttype, stdopn_reportsetup=:stdopn_reportsetup,"
               "    stdopn_rntime=:stdopn_rntime, stdopn_rncosttype=:stdopn_rncosttype, stdopn_reportrun=:stdopn_reportrun,"
               "    stdopn_rnqtyper=:stdopn_rnqtyper, stdopn_instructions=:stdopn_instructions "
               "WHERE (stdopn_id=:stdopn_id);" );

  q.bindValue(":stdopn_id", _stdopnid);
  q.bindValue(":stdopn_wrkcnt_id", _wrkcnt->id());
  q.bindValue(":stdopn_number", _number->text());
  q.bindValue(":stdopn_descrip1", _description1->text());
  q.bindValue(":stdopn_descrip2", _description2->text());
  q.bindValue(":stdopn_toolref", _toolReference->text());
  q.bindValue(":stdopn_instructions", _instructions->toPlainText());
  q.bindValue(":stdopn_produom", _prodUOM->currentText().trimmed().toUpper());
  q.bindValue(":stdopn_invproduomratio", _invProdUOMRatio->toDouble());
  q.bindValue(":stdopn_stdtimes", QVariant(_stdTimes->isChecked()));
  q.bindValue(":stdopn_sutime", _setupTime->toDouble());
  q.bindValue(":stdopn_reportsetup", QVariant(_reportSetup->isChecked()));

  if (_setupReport->currentIndex() == 0)
    q.bindValue(":stdopn_sucosttype", "D");
  else if (_setupReport->currentIndex() == 1)
    q.bindValue(":stdopn_sucosttype", "O");
  else if (_setupReport->currentIndex() == 2)
    q.bindValue(":stdopn_sucosttype", "N");

  q.bindValue(":stdopn_rntime", _runTime->toDouble());
  q.bindValue(":stdopn_reportrun", QVariant(_reportRun->isChecked()));

  if (_runReport->currentIndex() == 0)
    q.bindValue(":stdopn_rncosttype", "D");
  else if (_runReport->currentIndex() == 1)
    q.bindValue(":stdopn_rncosttype", "O");
  else if (_runReport->currentIndex() == 2)
    q.bindValue(":stdopn_rncosttype", "N");

  q.bindValue(":stdopn_rnqtyper", _runQtyPer->toDouble());
  q.exec();

  done(_stdopnid);
}

void standardOperation::populate()
{
  XSqlQuery stdopn;
  stdopn.prepare( "SELECT stdopn_number, stdopn_descrip1, stdopn_instructions,"
                  "       stdopn_descrip2, stdopn_toolref,"
                  "       stdopn_wrkcnt_id, stdopn_stdtimes,"
                  "       stdopn_produom, stdopn_sucosttype, stdopn_rncosttype,"
                  "       formatQty(stdopn_sutime) AS sutime, stdopn_reportsetup,"
                  "       formatQty(stdopn_rntime) AS rntime, stdopn_reportrun,"
                  "       formatQty(stdopn_rnqtyper) AS rnqtyper,"
                  "       formatUOMRatio(stdopn_invproduomratio) AS invproduomratio "
                  "FROM stdopn "
                  "WHERE (stdopn_id=:stdopn_id);" );
  stdopn.bindValue(":stdopn_id", _stdopnid);
  stdopn.exec();
  if (stdopn.first())
  {
    _number->setText(stdopn.value("stdopn_number"));
    _description1->setText(stdopn.value("stdopn_descrip1"));
    _description2->setText(stdopn.value("stdopn_descrip2"));
    _toolReference->setText(stdopn.value("stdopn_toolref"));
    _wrkcnt->setId(stdopn.value("stdopn_wrkcnt_id").toInt());
    _prodUOM->setText(stdopn.value("stdopn_produom"));
    _setupTime->setText(stdopn.value("sutime"));
    _reportSetup->setChecked(stdopn.value("stdopn_reportsetup").toBool());
    _runTime->setText(stdopn.value("rntime"));
    _reportRun->setChecked(stdopn.value("stdopn_reportrun").toBool());
    _runQtyPer->setText(stdopn.value("rnqtyper"));
    _invProdUOMRatio->setText(stdopn.value("invproduomratio"));
    _instructions->setText(stdopn.value("stdopn_instructions").toString());

    bool field = stdopn.value("stdopn_stdtimes").toBool();
    _stdTimes->setChecked(field);
    _setupTime->setEnabled(field);
    _setupReport->setEnabled(field);
    _runTime->setEnabled(field);
    _runReport->setEnabled(field);
    _runQtyPer->setEnabled(field);

    if (stdopn.value("stdopn_sucosttype").toString() == "D")
      _setupReport->setCurrentIndex(0);
    else if (stdopn.value("stdopn_sucosttype").toString() == "O")
      _setupReport->setCurrentIndex(1);
    else if (stdopn.value("stdopn_sucosttype").toString() == "N")
      _setupReport->setCurrentIndex(2);

    if (stdopn.value("stdopn_rncosttype").toString() == "D")
      _runReport->setCurrentIndex(0);
    else if (stdopn.value("stdopn_rncosttype").toString() == "O")
      _runReport->setCurrentIndex(1);
    else if (stdopn.value("stdopn_rncosttype").toString() == "N")
      _runReport->setCurrentIndex(2);
  }
}

void standardOperation::sHandleWorkCenter()
{
  if (_wrkcnt->id() != -1)
  {
    q.prepare( "SELECT formatTime(wrkcnt_avgsutime) AS setup "
               "FROM wrkcnt "
               "WHERE (wrkcnt_id=:wrkcnt_id);" );
    q.bindValue(":wrkcnt_id", _wrkcnt->id());
    q.exec();
    if (q.first())
      _setupTime->setText(q.value("setup").toString());
  }
}


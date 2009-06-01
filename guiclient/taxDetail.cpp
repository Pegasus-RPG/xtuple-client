/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxDetail.h"

#include <QSqlError>
#include <QVariant>
#include <QMessageBox>
#include "metasql.h"
#include "taxAdjustment.h"

taxDetail::taxDetail(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    _taxcodes->addColumn(tr("Code"),	 -1,  Qt::AlignLeft,   true,  "taxdetail_tax_code");
    _taxcodes->addColumn(tr("Description"),      100,  Qt::AlignLeft,   true,  "taxdetail_tax_descrip");
    _taxcodes->addColumn(tr("Amount"),      100,  Qt::AlignLeft,   true,  "taxdetail_tax");
    _taxcodes->addColumn(tr("Sequence"),      100,  Qt::AlignLeft,   true,  "taxdetail_taxclass_sequence");
    _taxcodes->setIndentation(10);
    
	connect(_taxcodes, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_cancel,	SIGNAL(clicked()),	this, SLOT(sCancel()));
	connect(_taxType,	SIGNAL(newID(int)),	this, SLOT(sCalculateTax()));
	//connect(_taxType,	SIGNAL(newID(int)),	this, SLOT(sPopulate()));
	connect(_new,	SIGNAL(clicked()),	this, SLOT(sNew()));
	connect(_delete, SIGNAL(clicked()),	this, SLOT(sDelete()));
	/* comment out until new code is written
    connect(_amountA,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_amountB,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_amountC,	SIGNAL(valueChanged()),	this, SLOT(sCalculateTotal()));
    connect(_calculate, SIGNAL(clicked()),	this, SLOT(sCalculateTax()));
    connect(_cancel,	SIGNAL(clicked()),	this, SLOT(sCancel()));
    connect(_taxCode,	SIGNAL(newID(int)),	this, SLOT(sCalculateTax()));
    connect(_taxCode,	SIGNAL(newID(int)),	this, SLOT(sPopulate()));

    _blankDetailDescriptions = false;
    clear();
    _taxCodeInitialized = false;
    
    _prcntA->setPrecision(omfgThis->percentVal());
    _prcntB->setPrecision(omfgThis->percentVal());
    _prcntC->setPrecision(omfgThis->percentVal());
    
    */
    //sCalculateTotal();
}

taxDetail::~taxDetail()
{
    // no need to delete child widgets, Qt does it all for us
}

void taxDetail::languageChange()
{
    retranslateUi(this);
}

enum SetResponse taxDetail::set(const ParameterList & pParams )
{
  QVariant param;
  bool     valid;
  
   _readonly = pParams.inList("readOnly");

   _new->hide();
   _delete->hide();

   param = pParams.value("taxzone_id", &valid);
   if (valid)
	   _taxzoneId = param.toInt();

   param = pParams.value("taxtype_id", &valid);
   if (valid)
   	   _taxType->setId(param.toInt());
   else
    clear();

   param = pParams.value("date", &valid);
   if (valid)
      _subtotal->setEffective(param.toDate());

   param = pParams.value("subtotal", &valid);
   if (valid)
	   _subtotal->setLocalValue(param.toDouble());
    
   param = pParams.value("curr_id", &valid);
   if (valid)
	   _subtotal->setId(param.toInt());

   param = pParams.value("order_id", &valid);
   if (valid)
    _orderid = param.toInt();

   param = pParams.value("order_type", &valid);
   if (valid)
    _ordertype = param.toString();

    param = pParams.value("display_type", &valid);
   if (valid)
    _displayType = param.toString();
	
    
   _adjustment = pParams.inList("adjustment"); 

   if (_readonly)
   {
     _taxType->setEnabled(FALSE);
     _save->hide();
     _cancel->setText(tr("&Close"));
   }

   if(_adjustment && !_readonly)
   {	
       _taxType->setEnabled(FALSE);
	   _new->show();
	   _delete->show();
	   _save->show();
   }

   sPopulate();
   //sCalculateTax();

   
  /* comment out until new code is written
  _readonly = pParams.inList("readOnly") ||
	(!pParams.inList("readOnly") && !_privileges->check("OverrideTax"));

  // tax_id first so signal/slot cascade doesn't overwrite passed-in A,B,C
  param = pParams.value("tax_id", &valid);
  _blankDetailDescriptions = !valid;
  if (valid)
    _taxCode->setId(param.toInt());
  else
    clear();
  _taxCodeInitialized = true;

  param = pParams.value("curr_id", &valid);
  if (valid)
    _total->setId(param.toInt());

  param = pParams.value("date", &valid);
  if (valid)
    _total->setEffective(param.toDate());

  param = pParams.value("valueA", &valid);
  if (valid)
  {
    _aCache = param.toDouble();
    _amountA->setLocalValue(_aCache);
  }

  param = pParams.value("valueB", &valid);
  if (valid)
  {
    _bCache = param.toDouble();
    _amountB->setLocalValue(_bCache);
  }

  param = pParams.value("valueC", &valid);
  if (valid)
  {
    _cCache = param.toDouble();
    _amountC->setLocalValue(_cCache);
  }

  param = pParams.value("pctA", &valid);
  if (valid)
  {
    _aPctCache = param.toDouble();
    _prcntA->setDouble(_aPctCache * 100);
  }

  param = pParams.value("pctB", &valid);
  if (valid)
  {
    _bPctCache = param.toDouble();
    _prcntB->setDouble(_bPctCache * 100);
  }

  param = pParams.value("pctC", &valid);
  if (valid)
  {
    _cPctCache = param.toDouble();
    _prcntC->setDouble(_cPctCache * 100);
  }
  
  param = pParams.value("subtotal", &valid);
  if (valid)
    _subtotal->setLocalValue(param.toDouble());

  sCalculateTotal();
  
  if (_taxCode->id() == -1 || _subtotal->isZero())
  {
    _calculate->setEnabled(FALSE);
    _calculate->hide();
  }

  if (_readonly)
  {
    _taxCode->setEnabled(FALSE);
    _calculate->setEnabled(FALSE);
    _calculate->hide();
    _save->hide();
    _cancel->setText(tr("&Close"));
  }
  */
  return NoError;
}

void taxDetail::sCancel()
{
  /*
    _amountA->setLocalValue(_aCache);
    _amountB->setLocalValue(_bCache);
    _amountC->setLocalValue(_cCache);
    _prcntA->setText(QString::number(_aPctCache));
    _prcntB->setText(QString::number(_bPctCache));
    _prcntC->setText(QString::number(_cPctCache));*/
    reject();
 }

void taxDetail::sCalculateTotal()
{
  //_total->setLocalValue(_amountA->localValue() + _amountB->localValue() + _amountC->localValue());
}

double taxDetail::amountA() const
{
  //return _amountA->localValue();
}

double taxDetail::amountB() const
{
  //return _amountB->localValue();
}

double taxDetail::amountC() const
{
  //return _amountC->localValue();
}

double taxDetail::pctA() const
{
  //return _prcntA->text().toDouble();
}

double taxDetail::pctB() const
{
  //return _prcntB->text().toDouble();
}

double taxDetail::pctC() const
{
  //return _prcntC->text().toDouble();
}

int taxDetail::tax() const
{
  //return _taxCode->id();
}


int taxDetail::taxtype() const
{
  return _taxType->id();
} 

void taxDetail::sCalculateTax()
{
   ParameterList params;
   params.append("taxzone_id", _taxzoneId);  
   params.append("taxtype_id", _taxType->id());
   params.append("date", _subtotal->effective());
   params.append("subtotal", _subtotal->localValue());
   params.append("curr_id", _subtotal->id());
  

   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "taxdetail_tax, taxdetail_taxclass_sequence, 0 AS xtindentrole "
              "FROM calculateTaxDetail(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>) "
			  "UNION ALL "
			  "SELECT -1 AS taxdetail_tax_id, 'Total' AS taxdetail_tax_code, NULL AS taxdetail_tax_descrip, "
			  "(select calculateTax(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>)) AS taxdetail_tax, NULL AS taxdetail_taxclass_sequence, "
			  "0 AS  xtindentrole;"); 
			  
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  _taxcodes->clear();
  _taxcodes->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  /*ParameterList params;
  
  if(_ordertype == "S" || _ordertype == "Q")
  {
   params.append("order_id", _orderid);
   params.append("order_type", _ordertype);
   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "taxdetail_tax, taxdetail_taxclass_sequence, 0 AS xtindentrole "
              "FROM calculateTaxDetailSummary(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>);");
    MetaSQLQuery mql(sql);
   q = mql.toQuery(params);
  
   _taxcodes->clear();
   _taxcodes->populate(q);
   if (q.lastError().type() != QSqlError::NoError)
   {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
   }

  }
  else
  { 
   params.append("taxzone_id", _taxzoneId);  
   params.append("taxtype_id", _taxType->id());
   params.append("date", _subtotal->effective());
   params.append("subtotal", _subtotal->localValue());
   params.append("curr_id", _subtotal->id());
  

   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "taxdetail_tax, taxdetail_taxclass_sequence, 0 AS xtindentrole "
              "FROM calculateTaxDetail(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>) "
			  "UNION ALL "
			  "SELECT -1 AS taxdetail_tax_id, 'Total' AS taxdetail_tax_code, NULL AS taxdetail_tax_descrip, "
			  "(select calculateTax(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>)) AS taxdetail_tax, NULL AS taxdetail_taxclass_sequence, "
			  "0 AS  xtindentrole;"); 
			  //"ORDER BY taxassign_taxzone_id, taxassign_taxtype_id, dummy_seq, xtindentrole;");
  
  MetaSQLQuery mql(sql);
  q = mql.toQuery(params);
  //_taxcodes->populate("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, taxdetail_tax, taxdetail_taxclass_sequence, taxdetail_level AS xtindentrole FROM calculateTaxDetail(2, 3, current_date, 300, 3) ");
  _taxcodes->clear();
  _taxcodes->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }*/
 


  /* comment out until new code is written
  XSqlQuery calcq;
  calcq.prepare("SELECT calculateTax(:tax_id, :subtotal, 0, 'A') AS ratea,"
            "       calculateTax(:tax_id, :subtotal, 0, 'B') AS rateb,"
            "       calculateTax(:tax_id, :subtotal, 0, 'C') AS ratec;" );
  calcq.bindValue(":tax_id", _taxCode->id());
  calcq.bindValue(":subtotal", _subtotal->localValue());
  calcq.exec();
  if(calcq.first())
  {
    _amountA->setLocalValue(calcq.value("ratea").toDouble());
    _amountB->setLocalValue(calcq.value("rateb").toDouble());
    _amountC->setLocalValue(calcq.value("ratec").toDouble());
    sCalculateTotal();
  }
  else if (calcq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, calcq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  */
}

void taxDetail::clear()
{
  /* comment out until new code is written
  if (_blankDetailDescriptions)
  {
    _description->setText("");
    _cumulative->setText("");
    _accntA->setText("");
    _accntB->setText("");
    _accntC->setText("");
    _taxCode->setNullStr("");
  }
  else
  {
    _description->setText(tr("Unspecified"));
    _cumulative->setText(tr("Unspecified"));
    _accntA->setText(tr("unassigned"));
    _accntB->setText(tr("unassigned"));
    _accntC->setText(tr("unassigned"));
    _taxCode->setNullStr(tr("Unspecified"));
  }

  _aPctCache = 0;
  _bPctCache = 0;
  _cPctCache = 0;
  _prcntA->clear();
  _prcntB->clear();
  _prcntC->clear();
  _amountA->setEnabled(false);
  _amountB->setEnabled(false);
  _amountC->setEnabled(false);
  */
   _taxType->setNullStr(tr("Unspecified"));
}

void taxDetail::sPopulate()
{
  XSqlQuery popq;

  popq.prepare("SELECT taxtype_descrip from taxtype where taxtype_id=:taxtype_id;");
  popq.bindValue(":taxtype_id", _taxType->id());
  popq.exec();
  if(popq.first())
	_descrip->setText(popq.value("taxtype_descrip").toString());
  else
    _descrip->setText("Unspecified");


  ParameterList params;

  if(_ordertype == "S" || _ordertype == "Q" || _ordertype == "I" || _ordertype == "B")
  {
   params.append("order_id", _orderid);
   params.append("order_type", _ordertype);
   params.append("display_type", _displayType);
   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "sum(taxdetail_tax) as taxdetail_tax, taxdetail_taxclass_sequence, 0 AS xtindentrole "
              "FROM calculateTaxDetailSummary(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>, <? value(\"display_type\") ?>) "
			  "GROUP BY taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, taxdetail_level, taxdetail_taxclass_sequence;");
   MetaSQLQuery mql(sql);
   q = mql.toQuery(params);
  
   _taxcodes->clear();
   _taxcodes->populate(q);
   if (q.lastError().type() != QSqlError::NoError)
   {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
   }
  }
  else if( _ordertype == "II" || _ordertype == "BI")
  {
   params.append("order_id", _orderid);
   params.append("order_type", _ordertype);
   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "taxdetail_tax, taxdetail_taxclass_sequence, taxdetail_level AS xtindentrole "
              "FROM calculateTaxDetailLine(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>); ");
   MetaSQLQuery mql(sql);
   q = mql.toQuery(params);
  
   _taxcodes->clear();
   _taxcodes->populate(q);
   if (q.lastError().type() != QSqlError::NoError)
   {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
   }
  }
  else
  {  
   params.append("taxzone_id", _taxzoneId);  
   params.append("taxtype_id", _taxType->id());
   params.append("date", _subtotal->effective());
   params.append("subtotal", _subtotal->localValue());
   params.append("curr_id", _subtotal->id());
  

   QString sql("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
              "taxdetail_tax, taxdetail_taxclass_sequence, taxdetail_level AS xtindentrole "
              "FROM calculateTaxDetail(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>) "
			  "UNION ALL "
			  "SELECT -1 AS taxdetail_tax_id, 'Total' AS taxdetail_tax_code, NULL AS taxdetail_tax_descrip, "
			  "(select calculateTax(<? value(\"taxzone_id\") ?>, <? value(\"taxtype_id\") ?>, "
			  " <? value(\"date\") ?>, <? value(\"curr_id\") ?>,  "
			  " <? value(\"subtotal\") ?>)) AS taxdetail_tax, NULL AS taxdetail_taxclass_sequence, "
			  "0 AS  xtindentrole;"); 
			  //"ORDER BY taxassign_taxzone_id, taxassign_taxtype_id, dummy_seq, xtindentrole;");
  
   MetaSQLQuery mql(sql);
   q = mql.toQuery(params);
  //_taxcodes->populate("SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, taxdetail_tax, taxdetail_taxclass_sequence, taxdetail_level AS xtindentrole FROM calculateTaxDetail(2, 3, current_date, 300, 3) ");
   _taxcodes->clear();
   _taxcodes->populate(q);
   if (q.lastError().type() != QSqlError::NoError)
   {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
   }
  }



  /* comment out until new code is written
  clear();
  XSqlQuery popq;
  popq.prepare("SELECT tax_descrip,"
	    "       formatBoolYN(tax_cumulative) AS f_cumulative,"
	    "       tax_ratea * 100 AS ratea,"
	    "       formatGLAccountLong(tax_sales_accnt_id) AS accnta,"
	    "       COALESCE(tax_sales_accnt_id,-1) AS accnta_id,"
	    "       tax_rateb * 100 AS rateb,"
	    "       formatGLAccountLong(tax_salesb_accnt_id) AS accntb,"
	    "       COALESCE(tax_salesb_accnt_id,-1) AS accntb_id,"
	    "       tax_ratec * 100 AS ratec,"
	    "       formatGLAccountLong(tax_salesc_accnt_id) AS accntc,"
	    "       COALESCE(tax_salesc_accnt_id,-1) AS accntc_id"
	    "  FROM tax"
	    " WHERE (tax_id=:tax_id); ");
  popq.bindValue(":tax_id", _taxCode->id());
  popq.exec();
  if(popq.first())
  {
    _description->setText(popq.value("tax_descrip").toString());
    _cumulative->setText(popq.value("f_cumulative").toString());

    if (! _taxCodeInitialized)
    {
      _aPctCache = popq.value("ratea").toDouble();
      _bPctCache = popq.value("rateb").toDouble();
      _cPctCache = popq.value("ratec").toDouble();
    }

    if(popq.value("accnta_id").toInt() != -1)
    { 
      _prcntA->setText(popq.value("ratea").toDouble());
      _accntA->setText(popq.value("accnta").toString());
      _amountA->setEnabled(! _readonly);
    }

    if(popq.value("accntb_id").toInt() != -1)
    { 
      _prcntB->setText(popq.value("rateb").toDouble());
      _accntB->setText(popq.value("accntb").toString());
      _amountB->setEnabled(! _readonly);
    }

    if(popq.value("accntc_id").toInt() != -1)
    { 
      _prcntC->setText(popq.value("ratec").toDouble());
      _accntC->setText(popq.value("accntc").toString());
      _amountC->setEnabled(! _readonly);
    }
  }
  else if (popq.lastError().type() != QSqlError::NoError)
  {
    systemError(this, popq.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  */
}

void taxDetail::sNew()
{
  taxAdjustment newdlg(this, "", true);
  ParameterList params;
  params.append("order_id", _orderid);
  params.append("order_type", _ordertype);
  params.append("date", _subtotal->effective());
  params.append("curr_id", _subtotal->id());
  params.append("mode", "new");
  if (newdlg.set(params) == NoError)
    newdlg.exec();
  sPopulate();
}

void taxDetail::sDelete()
{
    
  if(_taxcodes->id() == -1)
  {
    if (QMessageBox::question(this, tr("Delete All Tax Adjustments?"),
                              tr("<p>Are you sure that you want to delete all tax adjustments?"),
                              QMessageBox::Yes,
                              QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
	{
	 if(_ordertype == "I")
       q.prepare( "DELETE FROM invcheadtax WHERE taxhist_parent_id=:parent_id AND taxhist_taxtype_id=getadjustmenttaxtypeid();");
          
     if(_ordertype == "B")
         q.prepare( "DELETE FROM cobmisctax WHERE taxhist_parent_id=:parent_id AND taxhist_taxtype_id=getadjustmenttaxtypeid();");
	  
	 q.bindValue(":parent_id", _orderid);
     q.exec();
	}
  }
  else
  {
   if(_ordertype == "I")
    q.prepare( "DELETE FROM invcheadtax WHERE taxhist_parent_id=:parent_id AND taxhist_taxtype_id=getadjustmenttaxtypeid() AND taxhist_tax_id=:tax_id;"); 
   if(_ordertype == "B")
    q.prepare( "DELETE FROM cobmisctax WHERE taxhist_parent_id=:parent_id AND taxhist_taxtype_id=getadjustmenttaxtypeid() AND taxhist_tax_id=:tax_id;"); 
   q.bindValue(":parent_id", _orderid);
   q.bindValue(":tax_id", _taxcodes->id());
   q.exec();
  }
  sPopulate();
  return;
}


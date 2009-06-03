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

  _taxcodes->addColumn(tr("Code"),	       -1,  Qt::AlignLeft,   true,  "taxdetail_tax_code");
  _taxcodes->addColumn(tr("Description"), 100,  Qt::AlignLeft,   true,  "taxdetail_tax_descrip");
  _taxcodes->addColumn(tr("Amount"),      100,  Qt::AlignLeft,   true,  "taxdetail_tax");
  _taxcodes->addColumn(tr("Sequence"),    100,  Qt::AlignLeft,   true,  "taxdetail_taxclass_sequence");
  _taxcodes->setIndentation(10);
    
	connect(_taxcodes, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_cancel,	SIGNAL(clicked()),	this, SLOT(sCancel()));
	connect(_taxType,	SIGNAL(newID(int)),	this, SLOT(sCalculateTax()));
	connect(_new,	SIGNAL(clicked()),	this, SLOT(sNew()));
	connect(_delete, SIGNAL(clicked()),	this, SLOT(sDelete()));
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

  return NoError;
}

void taxDetail::sCancel()
{
    reject();
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
}

void taxDetail::clear()
{
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
   
  QString sql;
  ParameterList params;
  params.append("order_id", _orderid);
  params.append("order_type", _ordertype);
  if(_ordertype == "S" || _ordertype == "Q" || _ordertype == "I" || _ordertype == "B" || _ordertype == "RA")
  {
   params.append("display_type", _displayType);
   sql = "SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
         "  sum(taxdetail_tax) as taxdetail_tax, taxdetail_taxclass_sequence, 0 AS xtindentrole "
         "FROM calculateTaxDetailSummary(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>, <? value(\"display_type\") ?>) "
			   "GROUP BY taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, taxdetail_level, taxdetail_taxclass_sequence;";
  }
  else if( _ordertype == "II" || _ordertype == "BI")
   sql = "SELECT taxdetail_tax_id, taxdetail_tax_code, taxdetail_tax_descrip, "
         "  taxdetail_tax, taxdetail_taxclass_sequence, taxdetail_level AS xtindentrole "
         "FROM calculateTaxDetailLine(<? value(\"order_type\") ?>, <? value(\"order_id\") ?>); ";
  else
  {  
    sCalculateTax();
    return;
  }
   
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


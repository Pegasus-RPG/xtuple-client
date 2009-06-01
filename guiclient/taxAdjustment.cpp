/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "taxAdjustment.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QCloseEvent>

#include "storedProcErrorLookup.h"

taxAdjustment::taxAdjustment(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
	connect(_taxcode, SIGNAL(newID(int)), this, SLOT(sCheck()));
}

taxAdjustment::~taxAdjustment()
{
  // no need to delete child widgets, Qt does it all for us
}

void taxAdjustment::languageChange()
{
  retranslateUi(this);
}

enum SetResponse taxAdjustment::set(const ParameterList &pParams)  
{
  QVariant param;
  bool     valid;

  param = pParams.value("order_id", &valid);
  if (valid)
    _orderid = param.toInt();
   
   
  param = pParams.value("order_type", &valid);
  if (valid)
    _ordertype = param.toString();
    
   
  param = pParams.value("mode", &valid);
  if (valid)
    if (param.toString() == "new")
    {
		_mode = cNew;      
		_taxcode->setFocus();
    }
 
  param = pParams.value("date", &valid);
   if (valid)
     _amount->setEffective(param.toDate());

  param = pParams.value("curr_id", &valid);
   if (valid)
     _amount->setId(param.toInt());

  return NoError;
}

void taxAdjustment::sSave()
{ 
  if(_ordertype == "I")
   q.prepare( "SELECT taxhist_tax_id, taxhist_tax "
              "FROM invcheadtax " 
              "WHERE ((taxhist_parent_id=:order_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_tax_id=:tax_id));" );
  
  else if(_ordertype == "B")
   q.prepare( "SELECT taxhist_tax_id, taxhist_tax "
              "FROM cobmisctax " 
              "WHERE ((taxhist_parent_id=:order_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_tax_id=:tax_id));" );
  
  q.bindValue(":order_id", _orderid);
  q.bindValue(":tax_id", _taxcode->id()); 
 
  q.exec();
  if (q.first())
    _mode=cEdit;
  
  if (_mode == cNew && _ordertype == "I")
    q.prepare( "INSERT into invcheadtax(taxhist_basis,taxhist_percent,taxhist_amount,taxhist_docdate, taxhist_tax_id, taxhist_tax, taxhist_taxtype_id, taxhist_parent_id  ) "
               "VALUES (0, 0, 0, :date, :taxcode_id, :amount, getadjustmenttaxtypeid(), :order_id) ");    
  
  else if (_mode == cEdit && _ordertype == "I")
    q.prepare( "UPDATE invcheadtax "
               "SET taxhist_tax=:amount, taxhist_docdate=:date "
               "WHERE ((taxhist_tax_id=:taxcode_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_parent_id=:order_id)); ");
		     
  else if (_mode == cNew && _ordertype == "B")
    q.prepare( "INSERT into cobmisctax( taxhist_basis,taxhist_percent,taxhist_amount,taxhist_docdate, taxhist_tax_id, taxhist_tax, taxhist_taxtype_id, taxhist_parent_id  ) "
               "VALUES (0, 0, 0, :date, :taxcode_id, :amount, getadjustmenttaxtypeid(), :order_id) ");    
    
  else if (_mode == cEdit && _ordertype == "B")
    q.prepare( "UPDATE cobmisctax "
                "SET taxhist_tax=:amount, taxhist_docdate=:date "
               "WHERE ((taxhist_tax_id=:taxcode_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_parent_id=:order_id));");
  
  q.bindValue(":taxcode_id", _taxcode->id());
  q.bindValue(":amount", _amount->localValue());          
  q.bindValue(":order_id", _orderid);
  q.bindValue(":date", _amount->effective());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  done(_orderid);
}

void taxAdjustment::sCheck()     
{
  if(_ordertype == "I")
   q.prepare( "SELECT taxhist_tax_id, taxhist_tax "
              "FROM invcheadtax " 
              "WHERE ((taxhist_parent_id=:order_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_tax_id=:tax_id));" );
  
  else if(_ordertype == "B")
   q.prepare( "SELECT taxhist_tax_id, taxhist_tax "
              "FROM cobmisctax " 
              "WHERE ((taxhist_parent_id=:order_id) AND (taxhist_taxtype_id=getadjustmenttaxtypeid()) AND (taxhist_tax_id=:tax_id));" );
  
  q.bindValue(":order_id", _orderid);
  q.bindValue(":tax_id", _taxcode->id()); 
 
  q.exec();
  if (q.first())
  {
    _amount->setLocalValue(q.value("taxhist_tax").toDouble());
	_taxcode->setEnabled(FALSE);
	_amount->setFocus();
	_mode=cEdit;
  }
}



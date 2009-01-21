/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountSearch.h"

#include <QVariant>

#include <parameter.h>

#include "glcluster.h"
#include "xsqlquery.h"

accountSearch::accountSearch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : QDialog(parent, name, modal, fl)
{
  setupUi(this);

  _type->addItem(tr("ALL"), QVariant(""));
  _type->addItem(tr("Asset"), QVariant("A"));
  _type->addItem(tr("Liability"), QVariant("L"));
  _type->addItem(tr("Expense"), QVariant("E"));
  _type->addItem(tr("Revenue"), QVariant("R"));
  _type->addItem(tr("Equity"), QVariant("Q"));

  // signals and slots connections
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _accnt, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _clear, SIGNAL( clicked() ), this, SLOT( sClear() ) );
  connect(_type, SIGNAL(currentIndexChanged(int)), this, SLOT(sFillList()));
  connect(_descrip, SIGNAL(textChanged(const QString &)), this, SLOT(sFillList()));

  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _accnt->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _accnt->addColumn(tr("Profit"), 50, Qt::AlignCenter, true, "accnt_profit");
  }

  _accnt->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _accnt->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  }

  _accnt->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");

  _accnt->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");

  _accnt->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");

  _accnt->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");

  _showExternal = false;
}

accountSearch::~accountSearch()
{
  // no need to delete child widgets, Qt does it all for us
}

void accountSearch::languageChange()
{
  retranslateUi(this);
}

void accountSearch::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
  {
    _typeval = param.toUInt();
    if(_typeval > 0)
    {
      if(!(_typeval & GLCluster::cEquity))
        _type->removeItem(5);
      if(!(_typeval & GLCluster::cRevenue))
        _type->removeItem(4);
      if(!(_typeval & GLCluster::cExpense))
        _type->removeItem(3);
      if(!(_typeval & GLCluster::cLiability))
        _type->removeItem(2);
      if(!(_typeval & GLCluster::cAsset))
        _type->removeItem(1);
    }
  }

  _showExternal = pParams.inList("showExternal");

  sFillList();
}

void accountSearch::sClose()
{
  done(_accntid);
}

void accountSearch::sSelect()
{
  done(_accnt->id());
}

void accountSearch::sClear()
{
  done(-1);
}

void accountSearch::sFillList()
{
  QString sql("SELECT accnt_id, *,"
              "       CASE WHEN(accnt_type='A') THEN 'Asset'"
              "            WHEN(accnt_type='E') THEN 'Expense'"
              "            WHEN(accnt_type='L') THEN 'Liability'"
              "            WHEN(accnt_type='Q') THEN 'Equity'"
              "            WHEN(accnt_type='R') THEN 'Revenue'"
              "            ELSE accnt_type"
              "       END AS accnt_type_qtdisplayrole "
              "FROM (accnt LEFT OUTER JOIN"
              "     company ON (accnt_company=company_number)) "
              "     LEFT OUTER JOIN subaccnttype ON (accnt_type=subaccnttype_accnt_type AND accnt_subaccnttype_code=subaccnttype_code) ");

  QStringList types;
  QStringList where;

  if(_type->currentIndex() > 0)
  {
    where << "(accnt_type = '" + _type->itemData(_type->currentIndex()).toString() + "')";
  }
  else if(_typeval > 0)
  {
    if(_typeval & GLCluster::cAsset)
      types << ("'A'");
    if(_typeval & GLCluster::cLiability)
      types << ("'L'");
    if(_typeval & GLCluster::cExpense)
      types << ("'E'");
    if(_typeval & GLCluster::cRevenue)
      types << ("'R'");
    if(_typeval & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
    where << "(accnt_type IN (" + types.join(",") + "))";

  if (!_descrip->text().isEmpty())
    where << "((accnt_descrip ~* :descrip) OR (accnt_extref ~* :descrip))";

  if (! _showExternal)
    where << "(NOT COALESCE(company_external, false))";

  if (!where.isEmpty())
    sql += " WHERE " + where.join(" AND ");

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  XSqlQuery qry;
  qry.prepare(sql);
  qry.bindValue(":descrip", _descrip->text());
  qry.exec();
  _accnt->populate(qry, _accntid);
}

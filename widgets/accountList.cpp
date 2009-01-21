/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "accountList.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QVariant>

#include <parameter.h>

#include "xtreewidget.h"

#include "widgets.h"

#include "glcluster.h"

accountList::accountList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl) :
  QDialog(parent, name, modal, fl)
{
  if ( !name )
    setObjectName( "accountList" );

  _accntid = -1;
  _type = GLCluster::cUndefined;

  setWindowTitle(tr("Account Numbers"));

  QHBoxLayout *accountListLayout = new QHBoxLayout( this, 5, 7, "accountListLayout"); 
  QVBoxLayout *Layout68 = new QVBoxLayout( 0, 0, 0, "Layout68"); 
  QVBoxLayout *layout305 = new QVBoxLayout( 0, 0, 5, "layout305"); 

  QLabel *accountsLit = new QLabel(tr("Chart of Accounts:"), this, "accountsLit");
  Layout68->addWidget(accountsLit);

  _accnt = new XTreeWidget(this);
  _accnt->setObjectName("_accnt");
  Layout68->addWidget( _accnt );
  accountListLayout->addLayout( Layout68 );

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  layout305->addWidget( _close );

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setAutoDefault( TRUE );
  _select->setDefault( TRUE );
  layout305->addWidget( _select );

  _clear = new QPushButton(tr("C&lear"), this, "_clear");
  layout305->addWidget( _clear );
  QSpacerItem* spacer = new QSpacerItem( 20, 269, QSizePolicy::Minimum, QSizePolicy::Expanding );
  layout305->addItem( spacer );
  accountListLayout->addLayout( layout305 );

  resize( QSize(571, 351).expandedTo(minimumSizeHint()) );

  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _accnt, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _clear, SIGNAL( clicked() ), this, SLOT( sClear() ) );

  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _accnt->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _accnt->addColumn(tr("Profit"), 50, Qt::AlignCenter,  true, "accnt_profit");
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

void accountList::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("accnt_id", &valid);
  if (valid)
    _accntid = param.toInt();

  param = pParams.value("type", &valid);
  if (valid)
    _type = param.toUInt();

  _showExternal = pParams.inList("showExternal");

  sFillList();
}

void accountList::sClose()
{
  done(_accntid);
}

void accountList::sSelect()
{
  done(_accnt->id());
}

void accountList::sClear()
{
  done(-1);
}

void accountList::sFillList()
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

  if(_type > 0)
  {
    if(_type & GLCluster::cAsset)
      types << ("'A'");
    if(_type & GLCluster::cLiability)
      types << ("'L'");
    if(_type & GLCluster::cExpense)
      types << ("'E'");
    if(_type & GLCluster::cRevenue)
      types << ("'R'");
    if(_type & GLCluster::cEquity)
      types << ("'Q'");
  }
  if(!types.isEmpty())
    where << ("(accnt_type IN (" + types.join(",") + ")) ");

  if (! _showExternal)
    where << "(NOT COALESCE(company_external, false)) ";

  if (!where.isEmpty())
    sql += " WHERE " + where.join(" AND ");

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  _accnt->populate(sql, _accntid);
}

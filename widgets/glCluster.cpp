/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QHBoxLayout>
#include <QMessageBox>
#include "glcluster.h"

GLClusterLineEdit::GLClusterLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "accnt", "accnt_id", "formatGLAccount(accnt_id)", "accnt_descrip", "accnt_extref", 0, pName)
{
  setTitles(tr("Account"), tr("Accounts"));
  setUiName("accountNumber");
  setEditPriv("MaintainChartOfAccounts");
  setViewPriv("ViewChartOfAccounts");

  _showExternal = false;

  _query = "SELECT accnt_id, formatGLAccount(accnt_id) AS number, accnt_descrip AS name, accnt_extref AS description "
           "FROM accnt "
           "  LEFT OUTER JOIN company ON (accnt_company=company_number) "
           "WHERE (true) ";
}

void GLClusterLineEdit::setType(unsigned int pType)
{
  _type = pType;
}

void GLClusterLineEdit::setShowExternal(bool p)
{
  _showExternal = p;
}

void GLClusterLineEdit::sHandleCompleter()
{
  _extraClause.clear();
  QStringList types;

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
    _extraClause = "(accnt_type IN (" + types.join(",") + "))";

  if (!_extraClause.isEmpty() && !_showExternal)
    _extraClause += " AND ";

  if (!_showExternal)
    _extraClause += "(NOT COALESCE(company_external, false)) ";

  VirtualClusterLineEdit::sHandleCompleter();
  _extraClause.clear();
}

void GLClusterLineEdit::sList()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  accountList* newdlg = listFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("accnt_id", _id);
    params.append("type", _type);
    if (_showExternal)
      params.append("showExternal");
    newdlg->set(params);

    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sList() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

void GLClusterLineEdit::sSearch()
{
  disconnect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));

  accountSearch* newdlg = searchFactory();
  if (newdlg)
  {
    ParameterList params;
    params.append("accnt_id", _id);
    params.append("type", _type);
    if (_showExternal)
      params.append("showExternal");
    newdlg->set(params);
    newdlg->setSearchText(text());
    int id = newdlg->exec();
    setId(id);
  }
  else
    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
                          .arg(__FILE__)
                          .arg(__LINE__),
                          tr("%1::sSearch() not yet defined")
                          .arg(metaObject()->className()));

  connect(this, SIGNAL(editingFinished()), this, SLOT(sParse()));
}

accountList* GLClusterLineEdit::listFactory()
{
  return new accountList(this);
}

accountSearch* GLClusterLineEdit::searchFactory()
{
  return new accountSearch(this);
}

//////////////////////////////////////

GLCluster::GLCluster(QWidget *pParent, const char *pName) :
    VirtualCluster(pParent, pName)
{
  addNumberWidget(new GLClusterLineEdit(this, pName));
  _name->show();

  setOrientation(Qt::Horizontal);
}

///////////////////////////////////////////////////////////////////////////////

accountList::accountList(QWidget* pParent, Qt::WFlags pFlags) :
    VirtualList(pParent, pFlags)
{
  setObjectName("accountList");
  setMinimumWidth(600);
  _search->hide();
  _searchLit->hide();
  disconnect(_search,  SIGNAL(textChanged(const QString&)), this, SLOT(sSearch(const QString&)));

  _listTab->setColumnCount(0);
  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _listTab->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _listTab->addColumn(tr("Profit"), 50, Qt::AlignCenter,  true, "accnt_profit");
  }

  _listTab->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _listTab->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  }

  _listTab->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");
  _listTab->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _listTab->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _listTab->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");

  setWindowTitle(tr("Account Numbers"));
  _titleLit->setText(tr("Chart of Accounts"));
}

XTreeWidget* accountList::xtreewidget()
{
  return _listTab;
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
}

void accountList::sFillList()
{
  QString sql("SELECT accnt_id, *, "
              "       CASE WHEN(accnt_type='A') THEN '" + tr("Asset") + "'"
              "            WHEN(accnt_type='E') THEN '" + tr("Expense") + "'"
              "            WHEN(accnt_type='L') THEN '" + tr("Liability") + "'"
              "            WHEN(accnt_type='Q') THEN '" + tr("Equity") + "'"
              "            WHEN(accnt_type='R') THEN '" + tr("Revenue") + "'"
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

  _listTab->populate(sql, _accntid);
}

///////////////////////////

accountSearch::accountSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
  setAttribute(Qt::WA_DeleteOnClose);
  setObjectName( "accountSearch" );
  setMinimumWidth(600);

  _accntid = -1;
  _typeval = 0;

  setWindowTitle( tr( "Search for Account" ) );

  _typeLit = new QLabel(tr("Type:"));
  _type = new QComboBox();
  _type->addItem(tr("ALL"), QVariant(""));
  _type->addItem(tr("Asset"), QVariant("A"));
  _type->addItem(tr("Liability"), QVariant("L"));
  _type->addItem(tr("Expense"), QVariant("E"));
  _type->addItem(tr("Revenue"), QVariant("R"));
  _type->addItem(tr("Equity"), QVariant("Q"));

  _typeStrLyt = new QHBoxLayout(this);
  _typeStrLyt->setObjectName("typeStrLyt");
  _typeStrLyt->addWidget(_typeLit);
  _typeStrLyt->addWidget(_type);
  _typeStrLyt->addItem(new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Fixed));
  searchLyt->addItem(_typeStrLyt);

  // signals and slots connections
  connect( _type, SIGNAL(currentIndexChanged(int)), this, SLOT( sFillList()));

  _listTab->setColumnCount(0);
  if (_x_metrics)
  {
    if (_x_metrics->value("GLCompanySize").toInt() > 0)
      _listTab->addColumn(tr("Company"), 50, Qt::AlignCenter, true, "accnt_company");

    if (_x_metrics->value("GLProfitSize").toInt() > 0)
      _listTab->addColumn(tr("Profit"), 50, Qt::AlignCenter, true, "accnt_profit");
  }

  _listTab->addColumn(tr("Account Number"), 100, Qt::AlignCenter, true, "accnt_number");

  if (_x_metrics)
  {
    if (_x_metrics->value("GLSubaccountSize").toInt() > 0)
      _listTab->addColumn(tr("Sub."), 50, Qt::AlignCenter, true, "accnt_sub");
  }

  _listTab->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "accnt_descrip");
  _listTab->addColumn(tr("Type"),            75, Qt::AlignLeft ,  true, "accnt_type");
  _listTab->addColumn(tr("Sub. Type Code"),  75, Qt::AlignLeft,  false, "subaccnttype_code");
  _listTab->addColumn(tr("Sub. Type"),      100, Qt::AlignLeft,  false, "subaccnttype_descrip");

  disconnect(_searchNumber,  SIGNAL(clicked()),	        this, SLOT(sFillList()));
  disconnect(_searchDescrip, SIGNAL(clicked()),  	this, SLOT(sFillList()));
  _searchNumber->hide();
  _searchName->hide();
  _searchDescrip->hide();

  _showExternal = false;
}

void accountSearch::showEvent(QShowEvent* e)
{
  if (!_search->text().isEmpty())
    sFillList();
  VirtualSearch::showEvent(e);
}

void accountSearch::set(ParameterList &pParams)
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

  if (!_search->text().isEmpty())
    where << "((formatglaccount(accnt_id) ~* :descrip) OR (accnt_descrip ~* :descrip) OR (accnt_extref ~* :descrip))";

  if (! _showExternal)
    where << "(NOT COALESCE(company_external, false))";

  if (!where.isEmpty())
    sql += " WHERE " + where.join(" AND ");

  sql += "ORDER BY accnt_number, accnt_sub, accnt_profit;";

  XSqlQuery qry;
  qry.prepare(sql);
  qry.bindValue(":descrip", _search->text());
  qry.exec();
  _listTab->populate(qry, _accntid);
}




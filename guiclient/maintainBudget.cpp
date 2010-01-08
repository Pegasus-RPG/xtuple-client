/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "maintainBudget.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>

#include <xlistbox.h>
#include <accountList.h>
#include <glcluster.h>
#include <openreports.h>

/*
 *  Constructs a maintainBudget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
maintainBudget::maintainBudget(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_accountsAdd, SIGNAL(clicked()), this, SLOT(sAccountsAdd()));
  connect(_accountsDown, SIGNAL(clicked()), this, SLOT(sAccountsMoveDown()));
  connect(_accountsRemove, SIGNAL(clicked()), this, SLOT(sAccountsRemove()));
  connect(_accountsUp, SIGNAL(clicked()), this, SLOT(sAccountsMoveUp()));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_generate, SIGNAL(clicked()), this, SLOT(sGenerateTable()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_table, SIGNAL(valueChanged(int,int)), this, SLOT(sValueChanged(int,int)));
  connect(_periodsAll, SIGNAL(clicked()), this, SLOT(sPeriodsAll()));

  _dirty = false;
  _budgheadid = -1;
  _mode = cNew;
  
  // populate _periods
  q.exec("SELECT period_id, (formatDate(period_start) || '-' || formatDate(period_end)) AS f_name "
         "  FROM period "
         "ORDER BY period_start DESC;" );
  while(q.next())
  {
    XListBoxText *item = new XListBoxText(q.value("f_name").toString(), q.value("period_id").toInt());
    _periods->insertItem(item);
  }
}

/*
 *  Destroys the object and frees any allocated resources
 */
maintainBudget::~maintainBudget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void maintainBudget::languageChange()
{
  retranslateUi(this);
}

enum SetResponse maintainBudget::set(const ParameterList & pParams)
{
  XWidget::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("budghead_id", &valid);
  if(valid)
  {
    _budgheadid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
    {
      _mode = cNew;
      _print->setEnabled(false);
    }
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _name->setReadOnly(true);
      _descrip->setReadOnly(true);
      _save->setEnabled(false);
      _save->hide();
      _table->setReadOnly(true);
      _accountsAdd->setEnabled(false);
      _accountsDown->setEnabled(false);
      _accountsUp->setEnabled(false);
      _accountsRemove->setEnabled(false);
      _periodsAll->setEnabled(false);
      _periodsInvert->setEnabled(false);
      _periodsNone->setEnabled(false);
      _generate->setEnabled(false);
      _close->setFocus();
    }
  }

  return NoError;
}

void maintainBudget::sSave()
{
  if(_name->text().trimmed().isEmpty())
  {
    QMessageBox::warning(this, tr("Cannot Save Budget"),
        tr("You must specify a name for this budget before saving."));
    _name->setFocus();
    return;
  }

  _save->setFocus();

  if(cEdit == _mode)
    q.prepare("UPDATE budghead "
              "   SET budghead_name=:name,"
              "       budghead_descrip=:descrip "
              " WHERE(budghead_id=:budghead_id);");
  else if(cNew == _mode)
  {
    q.prepare("SELECT nextval('budghead_budghead_id_seq') AS result;");
    q.exec();
    if(q.first())
      _budgheadid = q.value("result").toInt();
    else
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare("INSERT INTO budghead"
              "      (budghead_id, budghead_name, budghead_descrip) "
              "VALUES(:budghead_id, :name, :descrip);");
  }

  q.bindValue(":budghead_id", _budgheadid);
  q.bindValue(":name", _name->text());
  q.bindValue(":descrip", _descrip->text());
  if(!q.exec())
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  q.prepare("SELECT setBudget(:budghead_id, :period_id, :accnt_id, :amount) AS result;");

  for(int r = 0; r < _table->numRows(); r++)
  {
    int accountid = *(_accountsRef.at(r));
    for(int c = 1; c < _table->numCols(); c++)
    {
      QString amount = _table->text(r, c);
      if(!amount.isEmpty())
      {
        q.bindValue(":budghead_id", _budgheadid);
        q.bindValue(":period_id", *(_periodsRef.at(c)));
        q.bindValue(":accnt_id", accountid);
        q.bindValue(":amount", amount.toDouble());
        q.exec();
      }
    }
  }
 
  _dirty = false;

  omfgThis->sBudgetsUpdated(_budgheadid, true);

  if(cNew == _mode)
  {
    _name->clear();
    _descrip->clear();
    _accounts->clear();
    _periods->clearSelection();
    _accountsRef.clear();
    _periodsRef.clear();
    _table->setNumRows(0);
    _table->setNumCols(0);
  }
  else
    close();
}

void maintainBudget::closeEvent( QCloseEvent * e )
{
  _close->setFocus();
  if(!_dirty)
  {
    e->accept();
    return;
  }

  switch( QMessageBox::information( this, tr("Unsaved Changes"),
                                    "The document has been changed since "
                                    "the last save.",
                                    "Save Now", "Cancel", "Close Anyway",
                                    0, 1 ) ) {
    case 0:
      sSave();
      e->accept();
      break;
    case 1:
    default: // just for sanity
      e->ignore();
      break;
    case 2:
      e->accept();
      break;
  }
}

void maintainBudget::sAccountsAdd()
{
  ParameterList params;
  params.append("type", (GLCluster::cAsset | GLCluster::cLiability | GLCluster::cExpense | GLCluster::cRevenue | GLCluster::cEquity));

  accountList newdlg(this, "", true);
  newdlg._accnt->setSelectionMode(QAbstractItemView::ExtendedSelection);
  newdlg.set(params);
  int accnt_id=newdlg.exec();

  if (accnt_id == -1)
    return;

  QList<XTreeWidgetItem*> selected = newdlg._accnt->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    XTreeWidgetItem *child = (XTreeWidgetItem*)selected[i];
    q.prepare("SELECT formatGLAccountLong(:accnt_id) AS result;");
    q.bindValue(":accnt_id", child->id());
    q.exec();
    if(q.first())
    {
      XListBoxText *item = new XListBoxText(q.value("result").toString(), child->id());
      _accounts->insertItem(item);
    }
  }
}

void maintainBudget::sAccountsRemove()
{
  int idx = _accounts->currentItem();
  if(idx != -1)
    _accounts->removeItem(idx);
}

void maintainBudget::sAccountsMoveUp()
{
  int idx = _accounts->currentItem();
  if(idx <= 0)
    return;

  Q3ListBoxItem * item = _accounts->item(idx);
  _accounts->takeItem(item);
  _accounts->insertItem(item, idx - 1);
  _accounts->setCurrentItem(item);
}

void maintainBudget::sAccountsMoveDown()
{
  int idx = _accounts->currentItem();
  if(idx == -1)
    return;

  Q3ListBoxItem * item = _accounts->item(idx);
  _accounts->takeItem(item);
  _accounts->insertItem(item, idx + 1);
  _accounts->setCurrentItem(item);
}

void maintainBudget::sPeriodsAll()
{
  _periods->selectAll(TRUE);
}

void maintainBudget::sValueChanged( int, int )
{
  _dirty = true;
}

void maintainBudget::sGenerateTable()
{
  _generate->setFocus();
  if(_dirty)
  {
    switch( QMessageBox::information( this, tr("Unsaved Changes"),
                                      "The document has been changed since "
                                      "the last save.",
                                      "Save Now", "Cancel", "Generate Anyway",
                                      0, 1 ) ) {
      case 0:
        sSave();
        break;
      case 1:
      default: // just for sanity
        return;
        break;
      case 2:
        break;
    }
  }

  _dirty = false;

  _accountsRef.clear();
  _periodsRef.clear();
  _table->setNumRows(0);
  _table->setNumCols(0);

  QStringList accounts;
  QStringList periods;

  XListBoxText* item = (XListBoxText*)_accounts->firstItem();
  while(item)
  {
    _accountsRef.append(item->id());
    accounts.append(item->text());
    item = (XListBoxText*)item->next();
  }
  _table->setNumRows(accounts.count());
  //_table->setRowLabels(accounts);

  bool isPeriodSelected = false;
  item = (XListBoxText*)_periods->firstItem();
  while(item)
  {
    if(item->isSelected())
    {
      isPeriodSelected = true;
      _periodsRef.prepend(item->id());
      periods.prepend(item->text());
    }
    item = (XListBoxText*)item->next();
  }
  if (!isPeriodSelected)
  {
    QMessageBox::critical(this, tr("Incomplete criteria"),
                          tr("<p>Please select at least one Period "
                             "before generating the table." ) );
    return;
  }
  
  _periodsRef.prepend(-1);
  periods.prepend(tr("Account"));
  _table->setNumCols(periods.count());
  _table->setColumnLabels(periods);

  periods.clear();
  Q3ValueList<int>::iterator it;
  for( it = _periodsRef.begin(); it != _periodsRef.end(); ++it )
  {
    if(-1 != (*it))
      periods.append(QString::number((*it)));
  }

  QString sql = "SELECT budgitem_period_id, budgitem_amount"
                "  FROM budgitem"
                " WHERE ((budgitem_accnt_id=:accnt_id)"
                "   AND  (budgitem_budghead_id=:budghead_id)"
                "   AND  (budgitem_period_id IN (" + periods.join(",") + ")) ); ";
  q.prepare(sql);
  
  for(int i = 0; i < _accountsRef.count(); i++)
  {
    _table->setText(i, 0, (accounts.at(i)));
    q.bindValue(":accnt_id", *(_accountsRef.at(i)));
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
      _table->setText(i, _periodsRef.findIndex(q.value("budgitem_period_id").toInt()), q.value("budgitem_amount").toString());
  }
  _table->setColumnReadOnly(0, true);
}

void maintainBudget::populate()
{
  q.prepare("SELECT budghead_name, budghead_descrip"
            "  FROM budghead"
            " WHERE(budghead_id=:budghead_id);");
  q.bindValue(":budghead_id", _budgheadid);
  q.exec();
  if(q.first())
  {
    _name->setText(q.value("budghead_name").toString());
    _descrip->setText(q.value("budghead_descrip").toString());

    q.prepare("SELECT DISTINCT budgitem_accnt_id, formatGLAccountLong(budgitem_accnt_id) AS result"
              "  FROM budgitem"
              " WHERE(budgitem_budghead_id=:budghead_id);");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      XListBoxText *item = new XListBoxText(q.value("result").toString(), q.value("budgitem_accnt_id").toInt());
      _accounts->insertItem(item);
    }

    q.prepare("SELECT DISTINCT budgitem_period_id"
              "  FROM budgitem"
              " WHERE(budgitem_budghead_id=:budghead_id);");
    q.bindValue(":budghead_id", _budgheadid);
    q.exec();
    while(q.next())
    {
      XListBoxText* item = (XListBoxText*)_periods->firstItem();
      while(item)
      {
        if(item->id() == q.value("budgitem_period_id").toInt())
        {
          _periods->setSelected(item, true);
          break;
        }
        item = (XListBoxText*)item->next();
      }
    }
    sGenerateTable();
  }
}

void maintainBudget::sPrint()
{
  ParameterList params;
  params.append("budghead_id", _budgheadid);

  orReport report("Budget", params);
  if(report.isValid())
    report.print();
  else
    report.reportError(this);
}

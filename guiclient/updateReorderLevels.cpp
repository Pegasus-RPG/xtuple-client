/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QSqlError>
#include <QMessageBox>

#include <metasql.h>
#include <qvariant.h>
#include <parameter.h>

#include "updateReorderLevels.h"
#include "mqlutil.h"
#include "submitAction.h"

/*
 *  Constructs a updateReorderLevels as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updateReorderLevels::updateReorderLevels(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    _daysGroupInt = new QButtonGroup(this);
    _daysGroupInt->addButton(_leadTime);
    _daysGroupInt->addButton(_fixedDays);

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_calendar, SIGNAL(newCalendarId(int)), _periods, SLOT(populate(int)));
    connect(_fixedDays, SIGNAL(toggled(bool)), _days, SLOT(setEnabled(bool)));
    connect(_leadTime, SIGNAL(toggled(bool)), _leadTimePad, SLOT(setEnabled(bool)));
    connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    connect(_preview, SIGNAL(toggled(bool)), this, SLOT(sHandleButtons()));
    connect(_results, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(sOpenEdit(QTreeWidgetItem*, int)));
    connect(_results, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, 
                                               SLOT(sCloseEdit(QTreeWidgetItem*,QTreeWidgetItem*)));
    
    
    _results->addColumn(tr("Site")            ,  _whsColumn,  Qt::AlignLeft,   true, "reordlvl_warehous_code");
    _results->addColumn(tr("Item Number")     , _itemColumn,  Qt::AlignLeft,   true, "reordlvl_item_number");
    _results->addColumn(tr("Description")     ,          -1,  Qt::AlignLeft,   true, "reordlvl_item_descrip");
    _results->addColumn(tr("Leadtime")        ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_leadtime");
    _results->addColumn(tr("Curr. Level")     ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_curr_level");
    _results->addColumn(tr("Days Stock")      ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_daysofstock");
    _results->addColumn(tr("Total Usage")     ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_total_usage");
    _results->addColumn(tr("New Level")       ,  _qtyColumn,  Qt::AlignRight,  true, "reordlvl_calc_level");
    
    if (!_metrics->boolean("EnableBatchManager"))
      _submit->hide();
}

/*
 *  Destroys the object and frees any allocated resources
 */
updateReorderLevels::~updateReorderLevels()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updateReorderLevels::languageChange()
{
    retranslateUi(this);
}


enum SetResponse updateReorderLevels::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;
  
  param = pParams.value("classcode", &valid);
  if (valid)
  {
    _parameter->setType(ParameterGroup::ClassCode);
    setWindowTitle("Update Reorder Levels by Class Code");
  }

  param = pParams.value("plancode", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Levels by Planner Code");
    _parameter->setType(ParameterGroup::PlannerCode);
  }

  param = pParams.value("item", &valid);
  if (valid)
  {
    setWindowTitle("Update Reorder Level by Item");
    _stack->setCurrentIndex(1);
  }

  return NoError;
}

bool updateReorderLevels::setParams(ParameterList &params)
{  
  if (_item->id() != -1)
    params.append("item_id", _item->id());
  else
    _parameter->appendValue(params);
  _warehouse->appendValue(params);
  
    if (_leadTime->isChecked())
    {
      params.append("addLeadtime"),
      params.append("daysOfStock", _leadTimePad->value());
    }
    else if (_fixedDays->isChecked())
      params.append("daysOfStock", _days->value());
  
  params.append("period_id_list",_periods->periodList());
  
  return true;
}

void updateReorderLevels::sUpdate()
{
  _results->clear();
  _totalDays->setText("");
  
  QString method;
  if (_periods->topLevelItemCount() > 0)
  {
    QString sql;

    if (_preview->isChecked())
      method = "query";
    else
      method = "update";

    ParameterList params;
    if (! setParams(params))
      return;

    MetaSQLQuery mql = mqlLoad("updateReorderLevels", method);
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
        
    if (_preview->isChecked())
    {
      if (q.first())
      {
        _totalDays->setText(q.value("reordlvl_total_days").toString());
        disconnect(_results, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(sItemChanged(QTreeWidgetItem*, int)));
        _results->populate(q, true);
        connect(_results, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(sItemChanged(QTreeWidgetItem*, int)));
        _tab->setCurrentIndex(1);
      }
      else
        QMessageBox::information(this, windowTitle(), tr("No item sites records found."));
    }
    else
      accept();
  }
}

void updateReorderLevels::sSubmit()
{
  if (_periods->topLevelItemCount() > 0)
  {
    ParameterList params;
    params.append("action_name", "UpdateReorderLevel");
    params.append("period_id_list", _periods->periodString());
    _warehouse->appendValue(params);
    _parameter->appendValue(params);

    if (_leadTime->isChecked())
      params.append("leadtimepad", _leadTimePad->value());
    else if (_fixedDays->isChecked())
      params.append("fixedlookahead", _days->value());

    submitAction newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() == XDialog::Accepted)
      accept();
  }
}

void updateReorderLevels::sHandleButtons()
{
  if (_preview->isChecked())
    _update->setText("Q&uery");
  else
    _update->setText("&Update");
}

void updateReorderLevels::sPost()
{
  MetaSQLQuery mql = mqlLoad("updateReorderLevels", "post");
  ParameterList params;
  QList<QTreeWidgetItem*> selected = _results->selectedItems();
  
  for (int i = 0; i < selected.size(); i++)
  {
    params.clear();
    params.append("itemsite_id", ((XTreeWidgetItem*)(selected[i]))->id());
    params.append("itemsite_reorderlevel",((XTreeWidgetItem*)(selected[i]))->text(7));
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
    delete selected[i];
  }
}

void updateReorderLevels::sOpenEdit(QTreeWidgetItem *item, const int col)
{
  if (col==7)
  {
    _results->openPersistentEditor(item,col);
    _results->editItem(item,col);
  }
}

void updateReorderLevels::sCloseEdit(QTreeWidgetItem * /*current*/, QTreeWidgetItem *previous)
{
  _results->closePersistentEditor(previous,7);
}

void updateReorderLevels::sItemChanged(QTreeWidgetItem *item, const int col)
{
  // Only positive numbers allowed
  if (col==7)
  {
    if (item->data(col,Qt::EditRole).toDouble() < 0)
      item->setData(col,Qt::EditRole,0);
    else
      item->setData(col,Qt::EditRole,item->data(col,Qt::EditRole).toDouble());
  }
}






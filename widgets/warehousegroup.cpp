/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>

#include <xsqlquery.h>
#include <parameter.h>

#include "warehouseCluster.h"
#include "warehousegroup.h"

WarehouseGroup::WarehouseGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  bool selectedOnly = false;
  if (_x_preferences)
    if (_x_preferences->boolean("selectedSites"))
      selectedOnly=true;
  
  if(pName)
    setObjectName(pName);

  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QWidget * selectedGroup = new QWidget(this);
  QButtonGroup * buttonGroup = new QButtonGroup(this);
  
  _all = new QRadioButton(tr("All Sites"), this, "_all");
  _site = new QLabel(tr("Site:"),this,"_site");
  _selected = new QRadioButton(tr("Selected:"), selectedGroup, "_selected");
	 
  if (!selectedOnly)
  {
    _all->setChecked(TRUE);
    buttonGroup->addButton(_all);
    buttonGroup->addButton(_selected);
  }
  
  _warehouses = new WComboBox(selectedGroup, "_warehouses");
  _warehouses->setEnabled(FALSE);

  if(selectedOnly)
  {
    QHBoxLayout *hLayout = new QHBoxLayout(selectedGroup);
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    hLayout->addWidget(_site);
    hLayout->addWidget(_warehouses);
    selectedGroup->setLayout(hLayout);
    
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(5);
    vLayout->setSpacing(0);
    vLayout->addWidget(selectedGroup);
    setLayout(vLayout);
    
    _all->hide();
    _selected->hide();
  }
  else
  {
    QHBoxLayout *hLayout = new QHBoxLayout(selectedGroup);
    hLayout->setMargin(0);
    hLayout->setSpacing(5);
    hLayout->addWidget(_selected);
    hLayout->addWidget(_warehouses);
    selectedGroup->setLayout(hLayout);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setMargin(5);
    vLayout->setSpacing(0);
    vLayout->addWidget(_all);
    vLayout->addWidget(selectedGroup);
    setLayout(vLayout);
    
    _site->hide();
  }

  connect(_selected, SIGNAL(toggled(bool)), _warehouses, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_warehouses, SIGNAL(newID(int)), this, SIGNAL(updated()));

  if (((_x_preferences) ? _x_preferences->value("PreferredWarehouse").toInt() : -1) != -1)
    _selected->setChecked(TRUE);

  setTabOrder(_all, _selected);
  setTabOrder(_selected, _warehouses);
  setTabOrder(_warehouses, _all);
  setFocusProxy(_all);
  
  if (_x_metrics)
  {
    if (!_x_metrics->boolean("MultiWhs"))
    {
      this->hide();
      setAll();
    }
  }
}

void WarehouseGroup::setAll()
{
  _all->setChecked(TRUE);
}

void WarehouseGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _warehouses->setId(pId);
}

void WarehouseGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
    pParams.append("warehous_id", _warehouses->id());
}

void WarehouseGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
    pQuery.bindValue(":warehous_id", _warehouses->id());
}

void WarehouseGroup::findItemSites(int pItemid)
{
  _warehouses->findItemsites(pItemid);
}

int WarehouseGroup::id()
{
  if (_selected->isChecked())
    return _warehouses->id();
  else
    return -1;
}

bool WarehouseGroup::isAll() const
{
  return _all->isChecked();
}

bool WarehouseGroup::isSelected() const
{
  return _selected->isChecked();
}


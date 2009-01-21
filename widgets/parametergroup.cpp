/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QRadioButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QButtonGroup>

#include <xsqlquery.h>
#include <parameter.h>

#include "xcombobox.h"

#include "parametergroup.h"

ParameterGroup::ParameterGroup(QWidget *pParent, const char *pName) :
  QGroupBox(pParent)
{
  if(pName)
    setObjectName(pName);

  _type = AdhocGroup;

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

  QWidget *selectedGroup = new QWidget(this);
  QWidget *patternGroup = new QWidget(this);

  _all= new QRadioButton(QObject::tr("All"), this, "_all");
  _all->setChecked(TRUE);

  _selected = new QRadioButton(tr("Selected:"), selectedGroup, "_selected");

  _items = new XComboBox(FALSE, selectedGroup, "_items");
  _items->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  _items->setEnabled(FALSE);

  QHBoxLayout * hLayout = new QHBoxLayout(selectedGroup);
  hLayout->setSpacing(5);
  hLayout->setMargin(0);
  hLayout->addWidget(_selected);
  hLayout->addWidget(_items);
  selectedGroup->setLayout(hLayout);
  
  _usePattern = new QRadioButton(tr("Pattern:"), patternGroup, "_usePattern");

  _pattern = new QLineEdit(patternGroup, "_pattern");
  _pattern->setEnabled(FALSE);

  hLayout = new QHBoxLayout(patternGroup);
  hLayout->setSpacing(5);
  hLayout->setMargin(0);
  hLayout->addWidget(_usePattern);
  hLayout->addWidget(_pattern);
  patternGroup->setLayout(hLayout);

  QVBoxLayout * vLayout = new QVBoxLayout(this);
  vLayout->setSpacing(0);
  vLayout->setMargin(5);
  vLayout->addWidget(_all);
  vLayout->addWidget(selectedGroup);
  vLayout->addWidget(patternGroup);
  setLayout(vLayout);
  
  QButtonGroup * buttonGroup = new QButtonGroup(this);
  buttonGroup->addButton(_all);
  buttonGroup->addButton(_selected);
  buttonGroup->addButton(_usePattern);

  connect(_selected, SIGNAL(toggled(bool)), _items, SLOT(setEnabled(bool)));
  connect(_usePattern, SIGNAL(toggled(bool)), _pattern, SLOT(setEnabled(bool)));
  connect(buttonGroup, SIGNAL(buttonClicked(int)), this, SIGNAL(updated()));
  connect(_items, SIGNAL(newID(int)), this, SIGNAL(updated()));
  connect(_pattern, SIGNAL(lostFocus()), this, SIGNAL(updated()));

  setFocusProxy(_all);
}

void ParameterGroup::setType(enum ParameterGroupTypes pType)
{
  if (_type == pType)
    return;

  _type = pType;

  switch (_type)
  {
    case AdhocGroup:
      _all->setText(QObject::tr("All"));
      break;

    case ClassCode:
      _all->setText(QObject::tr("All Class Codes"));
      _items->setType(XComboBox::ClassCodes);
      break;

    case PlannerCode:
      _all->setText(QObject::tr("All Planner Codes"));
      _items->setType(XComboBox::PlannerCodes);
      break;

    case ProductCategory:
      _all->setText(QObject::tr("All Product Categories"));
      _items->setType(XComboBox::ProductCategories);
      break;

    case ItemGroup:
      _all->setText(QObject::tr("All Item Groups"));
      _items->setType(XComboBox::ItemGroups);
      break;

    case CostCategory:
      _all->setText(QObject::tr("All Cost Categories"));
      _items->setType(XComboBox::CostCategories);
      break;

    case CustomerType:
      _all->setText(QObject::tr("All Customer Types"));
      _items->setType(XComboBox::CustomerTypes);
      break;

    case CustomerGroup:
      _all->setText(QObject::tr("All Customer Groups"));
      _items->setType(XComboBox::CustomerGroups);
      break;

    case CurrencyNotBase:
      _all->setText(QObject::tr("All Foreign Currencies"));
      _items->setType(XComboBox::CurrenciesNotBase);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case Currency:
      _all->setText(QObject::tr("All Currencies"));
      _items->setType(XComboBox::Currencies);
      if (_items->isHidden()) // if there's only one currency defined
        hide();
      break;

    case WorkCenter:
      _all->setText(QObject::tr("All Work Centers"));
      _items->setType(XComboBox::WorkCenters);
      break;

    case User:
      _all->setText(QObject::tr("All Users"));
      _items->setType(XComboBox::Users);
  }
}

void ParameterGroup::setId(int pId)
{
  _selected->setChecked(TRUE);
  _items->setId(pId);
}

void ParameterGroup::setPattern(const QString &pPattern)
{
  _usePattern->setChecked(TRUE);
  _pattern->setText(pPattern);
}

void ParameterGroup::setState(enum ParameterGroupStates pState)
{
  switch(pState)
  {
    case Selected:
      _selected->setChecked(true);
      break;
    case Pattern:
      _usePattern->setChecked(true);
      break;
    case All:
    default:
      _all->setChecked(true);
  }
}

void ParameterGroup::appendValue(ParameterList &pParams)
{
  if (_selected->isChecked())
  {
    if (_type == ClassCode)
      pParams.append("classcode_id", _items->id());
    else if (_type == PlannerCode)
      pParams.append("plancode_id", _items->id());
    else if (_type == ProductCategory)
      pParams.append("prodcat_id", _items->id());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_id", _items->id());
    else if (_type == CostCategory)
      pParams.append("costcat_id", _items->id());
    else if (_type == CustomerType)
      pParams.append("custtype_id", _items->id());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_id", _items->id());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("curr_id", _items->id());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_id", _items->id());
    else if (_type == User)
      pParams.append("usr_id", _items->id());
  }
  else if (_usePattern->isChecked())
  {
    if (_type == ClassCode)
      pParams.append("classcode_pattern", _pattern->text());
    else if (_type == PlannerCode)
      pParams.append("plancode_pattern", _pattern->text());
    else if (_type == ProductCategory)
      pParams.append("prodcat_pattern", _pattern->text());
    else if (_type == ItemGroup)
      pParams.append("itemgrp_pattern", _pattern->text());
    else if (_type == CostCategory)
      pParams.append("costcat_pattern", _pattern->text());
    else if (_type == CustomerType)
      pParams.append("custtype_pattern", _pattern->text());
    else if (_type == CustomerGroup)
      pParams.append("custgrp_pattern", _pattern->text());
    else if (_type == CurrencyNotBase || _type == Currency)
      pParams.append("currConcat_pattern", _pattern->text());
    else if (_type == WorkCenter)
      pParams.append("wrkcnt_pattern", _pattern->text());
    else if (_type == User)
      pParams.append("usr_pattern", _pattern->text());
  }
}

void ParameterGroup::repopulateSelected()
{
    _items->populate();
}

void ParameterGroup::bindValue(XSqlQuery &pQuery)
{
  if (_selected->isChecked())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_id";   break;

      case PlannerCode:
        name = ":plancode_id";    break;

      case ProductCategory:
        name = ":prodcat_id";     break;

      case ItemGroup:
        name = ":itemgrp_id";     break;

      case CostCategory:
        name = ":costcat_id";     break;

      case CustomerType:
        name = ":custtype_id";    break;

      case CustomerGroup:
        name = ":custgrp_id";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":curr_id";        break;

      case WorkCenter:
        name = ":wrkcnt_id";      break;

      case User:
        name = ":usr_id";         break;

      case AdhocGroup:
        name = ":id";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _items->id());
  }
  else if (_usePattern->isChecked())
  {
    QString name;

    switch (_type)
    {
      case ClassCode:
        name = ":classcode_pattern";   break;

      case PlannerCode:
        name = ":plancode_pattern";    break;

      case ProductCategory:
        name = ":prodcat_pattern";     break;

      case ItemGroup:
        name = ":itemgrp_pattern";     break;

      case CostCategory:
        name = ":costcat_pattern";     break;

      case CustomerType:
        name = ":custtype_pattern";    break;

      case CustomerGroup:
        name = ":custgrp_pattern";     break;

      case CurrencyNotBase:
      case Currency:
        name = ":currConcat_pattern";  break;

      case WorkCenter:
        name = ":wrkcnt_pattern";      break;

      case User:
        name = ":usr_pattern";         break;

      case AdhocGroup:
        name = ":pattern";             break;

      default:
        break;
    }

    pQuery.bindValue(name, _pattern->text());
  }
}

enum ParameterGroup::ParameterGroupStates ParameterGroup::state()
{
  if (_all->isChecked())
    return All;
  else if (_selected->isChecked())
    return Selected;
  else
    return Pattern;
}

int ParameterGroup::id()
{
  if (_selected->isChecked())
    return _items->id();
  else
    return -1;
}

QString ParameterGroup::pattern()
{
  if (_usePattern->isChecked())
    return _pattern->text();
  else
    return QString::null;
}


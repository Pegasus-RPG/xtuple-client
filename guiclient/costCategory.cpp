/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "costCategory.h"

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

costCategory::costCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  _costcatid = -1;

  connect(_buttonBox, SIGNAL(accepted()), this, SLOT(sSave()));
  connect(_category, SIGNAL(editingFinished()), this, SLOT(sCheck()));

  _asset->setType(GLCluster::cAsset);
  _expense->setType(GLCluster::cExpense);
  _wip->setType(GLCluster::cAsset);
  _inventoryCost->setType(GLCluster::cExpense | GLCluster::cAsset);
  _adjustment->setType(GLCluster::cExpense);
  _invScrap->setType(GLCluster::cExpense);
  _mfgScrap->setType(GLCluster::cExpense);
  _transformClearing->setType(GLCluster::cAsset);
  _purchasePrice->setType(GLCluster::cExpense | GLCluster::cAsset);
  _liability->setType(GLCluster::cLiability);
  _freight->setType(GLCluster::cExpense);
  _shippingAsset->setType(GLCluster::cAsset);
  _toLiabilityClearing->setType(GLCluster::cLiability);

  _transformClearingLit->setVisible(_metrics->boolean("Transforms")); 
  _transformClearing->setVisible(_metrics->boolean("Transforms"));
  _toLiabilityClearingLit->setVisible(_metrics->boolean("MultiWhs"));
  _toLiabilityClearing->setVisible(_metrics->boolean("MultiWhs"));

  // This should all be generated as part of the UI but it was the only
  // way I could get the tab order to work exactly as it was supposed to.
  QWidget::setTabOrder(_category, _description);
  QWidget::setTabOrder(_description, _asset);
  QWidget::setTabOrder(_asset, _expense);
  QWidget::setTabOrder(_expense, _wip);
  QWidget::setTabOrder(_wip, _inventoryCost);
  QWidget::setTabOrder(_inventoryCost, _transformClearing);
  QWidget::setTabOrder(_transformClearing, _purchasePrice);
  QWidget::setTabOrder(_purchasePrice, _adjustment);
  QWidget::setTabOrder(_adjustment, _invScrap);
  QWidget::setTabOrder(_invScrap, _mfgScrap);
  QWidget::setTabOrder(_mfgScrap, _liability);
  QWidget::setTabOrder(_liability, _shippingAsset);
  QWidget::setTabOrder(_shippingAsset, _freight);
  QWidget::setTabOrder(_freight, _toLiabilityClearing);
  QWidget::setTabOrder(_toLiabilityClearing, _buttonBox->button(QDialogButtonBox::Save));
  QWidget::setTabOrder(_buttonBox->button(QDialogButtonBox::Save), _buttonBox->button(QDialogButtonBox::Cancel));
}

costCategory::~costCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void costCategory::languageChange()
{
  retranslateUi(this);
}

enum SetResponse costCategory::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("costcat_id", &valid);
  if (valid)
  {
    _costcatid = param.toInt();
    populate();
  }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _category->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _description->setFocus();
    }
    else if (param.toString() == "copy")
    {
      _mode = cCopy;
      _description->setFocus();
      _costcatid = -1;
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _category->setEnabled(FALSE);
      _description->setEnabled(FALSE);
      _asset->setReadOnly(TRUE);
      _expense->setReadOnly(TRUE);
      _wip->setReadOnly(TRUE);
      _inventoryCost->setReadOnly(TRUE);
      _adjustment->setReadOnly(TRUE);
      _invScrap->setReadOnly(TRUE);
      _mfgScrap->setReadOnly(TRUE);
      _transformClearing->setReadOnly(TRUE);
      _purchasePrice->setReadOnly(TRUE);
      _liability->setReadOnly(TRUE);
      _freight->setReadOnly(TRUE);
      _shippingAsset->setReadOnly(TRUE);
      _toLiabilityClearing->setReadOnly(TRUE);
      _buttonBox->clear();
      _buttonBox->addButton(QDialogButtonBox::Close);
      _buttonBox->setFocus();
    }
  }

  return NoError;
}

void costCategory::sCheck()
{
  if ((_mode == cNew) && (_category->text().length() != 0))
  {
    q.prepare( "SELECT costcat_id"
               "  FROM costcat"
               " WHERE((UPPER(costcat_code)=UPPER(:costcat_code))"
               "   AND (costcat_id != :costcat_id));" );
    q.bindValue(":costcat_code", _category->text().trimmed());
    q.bindValue(":costcat_id", _costcatid);
    q.exec();
    if (q.first())
    {
      _costcatid = q.value("costcat_id").toInt();
      _mode = cEdit;
      populate();

      _category->setEnabled(FALSE);
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }
}

void costCategory::sSave()
{
  QList<GuiErrorCheck> errors;

  errors << GuiErrorCheck(_category->text().trimmed().isEmpty(), _category,
                         tr("<p>You must enter a name for this Cost Category before saving it."));

  if (_metrics->boolean("InterfaceToGL"))
  {
    errors << GuiErrorCheck(!_asset->isValid(), _asset,
                            tr("<p>You must select an Inventory Asset Account before saving."))
           << GuiErrorCheck(!_expense->isValid(), _expense,
                            tr("<p>You must select an Expense Asset Account before saving."))
           << GuiErrorCheck(!_wip->isValid(), _wip,
                            tr("<p>You must select a WIP Asset Account before saving."))
           << GuiErrorCheck(!_inventoryCost->isValid(), _inventoryCost,
                            tr("<p>You must select an Inventory Cost Variance Account before saving."))
           << GuiErrorCheck(_metrics->boolean("MultiWhs") && _metrics->boolean("Transforms") && !_transformClearing->isValid(), _transformClearing,
                            tr("<p>You must select a Transform Clearing Account before saving."))
           << GuiErrorCheck(!_purchasePrice->isValid(), _purchasePrice,
                            tr("<p>You must select a Purchase Price Variance Account before saving."))
           << GuiErrorCheck(!_adjustment->isValid(), _adjustment,
                            tr("<p>You must select an Inventory Adjustment Account before saving."))
           << GuiErrorCheck(!_invScrap->isValid(), _invScrap,
                            tr("<p>You must select an Inventory Scrap Account before saving."))
           << GuiErrorCheck(!_mfgScrap->isValid(), _mfgScrap,
                            tr("<p>You must select a Manufacturing Scrap Account before saving."))
           << GuiErrorCheck(!_liability->isValid(), _liability,
                            tr("<p>You must select a P/O Liability Clearing Account before saving."))
           << GuiErrorCheck(!_shippingAsset->isValid(), _shippingAsset,
                            tr("<p>You must select a Shipping Asset Account before saving."))
           << GuiErrorCheck(!_freight->isValid(), _freight,
                            tr("<p>You must select a Line Item Freight Expense Account before saving."))
           << GuiErrorCheck(_metrics->boolean("MultiWhs") && !_toLiabilityClearing->isValid(), _toLiabilityClearing,
                            tr("<p>You must select a Transfer Order Liability Clearing Account before saving."))
           ;
  }

  q.prepare( "SELECT costcat_id"
             "  FROM costcat"
             " WHERE((UPPER(costcat_code)=UPPER(:costcat_code))"
             "   AND (costcat_id != :costcat_id));" );
  q.bindValue(":costcat_code", _category->text().trimmed());
  q.bindValue(":costcat_id", _costcatid);
  q.exec();
  if (q.first())
  {
    errors << GuiErrorCheck(true, _category,
                           tr("<p>The Name you have entered for this Cost Category is already in use."));
  }

  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Cost Category"), errors))
    return;

  QSqlQuery newCostCategory;

  if ( (_mode == cNew) || (_mode == cCopy) )
  {
    q.exec("SELECT NEXTVAL('costcat_costcat_id_seq') AS costcat_id");
    if (q.first())
      _costcatid = q.value("costcat_id").toInt();
    else
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }

    q.prepare( "INSERT INTO costcat"
               "( costcat_id, costcat_code, costcat_descrip,"
               "  costcat_asset_accnt_id, costcat_invcost_accnt_id,"
               "  costcat_liability_accnt_id, costcat_freight_accnt_id,"
               "  costcat_adjustment_accnt_id, costcat_scrap_accnt_id, costcat_mfgscrap_accnt_id,"
               "  costcat_transform_accnt_id, costcat_wip_accnt_id,"
               "  costcat_purchprice_accnt_id,"
               "  costcat_shipasset_accnt_id, costcat_toliability_accnt_id, "
               "  costcat_exp_accnt_id) "
               "VALUES "
               "( :costcat_id, :costcat_code, :costcat_descrip,"
               "  :costcat_asset_accnt_id, :costcat_invcost_accnt_id,"
               "  :costcat_liability_accnt_id, :costcat_freight_accnt_id,"
               "  :costcat_adjustment_accnt_id, :costcat_scrap_accnt_id, :costcat_mfgscrap_accnt_id,"
               "  :costcat_transform_accnt_id, :costcat_wip_accnt_id,"
               "  :costcat_purchprice_accnt_id,"
               "  :costcat_shipasset_accnt_id, :costcat_toliability_accnt_id,"
               "  :costcat_exp_accnt_id);" );
  }
  else if (_mode == cEdit)
    q.prepare( "UPDATE costcat "
               "SET costcat_code=:costcat_code, costcat_descrip=:costcat_descrip,"
               "    costcat_asset_accnt_id=:costcat_asset_accnt_id,"
               "    costcat_invcost_accnt_id=:costcat_invcost_accnt_id,"
               "    costcat_liability_accnt_id=:costcat_liability_accnt_id,"
               "    costcat_freight_accnt_id=:costcat_freight_accnt_id,"
               "    costcat_adjustment_accnt_id=:costcat_adjustment_accnt_id,"
               "    costcat_scrap_accnt_id=:costcat_scrap_accnt_id,"
               "    costcat_mfgscrap_accnt_id=:costcat_mfgscrap_accnt_id,"
               "    costcat_transform_accnt_id=:costcat_transform_accnt_id,"
               "    costcat_wip_accnt_id=:costcat_wip_accnt_id,"
               "    costcat_purchprice_accnt_id=:costcat_purchprice_accnt_id,"
               "    costcat_shipasset_accnt_id=:costcat_shipasset_accnt_id,"
               "    costcat_toliability_accnt_id=:costcat_toliability_accnt_id, "
               "    costcat_exp_accnt_id=:costcat_exp_accnt_id "
               "WHERE (costcat_id=:costcat_id);" );

  q.bindValue(":costcat_id", _costcatid);
  q.bindValue(":costcat_code", _category->text().trimmed());
  q.bindValue(":costcat_descrip", _description->text().trimmed());
  q.bindValue(":costcat_asset_accnt_id", _asset->id());
  q.bindValue(":costcat_invcost_accnt_id", _inventoryCost->id());
  q.bindValue(":costcat_liability_accnt_id", _liability->id());
  q.bindValue(":costcat_freight_accnt_id", _freight->id());
  q.bindValue(":costcat_adjustment_accnt_id", _adjustment->id());
  q.bindValue(":costcat_scrap_accnt_id", _invScrap->id());
  q.bindValue(":costcat_mfgscrap_accnt_id", _mfgScrap->id());
  q.bindValue(":costcat_transform_accnt_id", _transformClearing->id());
  q.bindValue(":costcat_wip_accnt_id", _wip->id());
  q.bindValue(":costcat_purchprice_accnt_id", _purchasePrice->id());
  q.bindValue(":costcat_shipasset_accnt_id", _shippingAsset->id());
  q.bindValue(":costcat_exp_accnt_id", _expense->id());
  if (_toLiabilityClearing->isValid())
    q.bindValue(":costcat_toliability_accnt_id", _toLiabilityClearing->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  emit saved(_costcatid);
  done(_costcatid);
}

void costCategory::populate()
{
  q.prepare( "SELECT * "
             "FROM costcat "
             "WHERE (costcat_id=:costcat_id);" );
  q.bindValue(":costcat_id", _costcatid);
  q.exec();
  if (q.first())
  {
    if (_mode != cCopy)
    {
      _category->setText(q.value("costcat_code").toString());
      _description->setText(q.value("costcat_descrip").toString());
    }

    _asset->setId(q.value("costcat_asset_accnt_id").toInt());
    _expense->setId(q.value("costcat_exp_accnt_id").toInt());
    _inventoryCost->setId(q.value("costcat_invcost_accnt_id").toInt());
    _liability->setId(q.value("costcat_liability_accnt_id").toInt());
    _freight->setId(q.value("costcat_freight_accnt_id").toInt());
    _adjustment->setId(q.value("costcat_adjustment_accnt_id").toInt());
    _invScrap->setId(q.value("costcat_scrap_accnt_id").toInt());
    _mfgScrap->setId(q.value("costcat_mfgscrap_accnt_id").toInt());
    _wip->setId(q.value("costcat_wip_accnt_id").toInt());
    _transformClearing->setId(q.value("costcat_transform_accnt_id").toInt());
    _purchasePrice->setId(q.value("costcat_purchprice_accnt_id").toInt());
    _shippingAsset->setId(q.value("costcat_shipasset_accnt_id").toInt());
    _toLiabilityClearing->setId(q.value("costcat_toliability_accnt_id").toInt());

    emit populated(_costcatid);
  }
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

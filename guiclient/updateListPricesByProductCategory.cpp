/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updateListPricesByProductCategory.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "xdoublevalidator.h"

updateListPricesByProductCategory::updateListPricesByProductCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
  : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _updateBy->setValidator(new XDoubleValidator(-100, 9999, 2, _updateBy));
}

updateListPricesByProductCategory::~updateListPricesByProductCategory()
{
  // no need to delete child widgets, Qt does it all for us
}

void updateListPricesByProductCategory::languageChange()
{
  retranslateUi(this);
}

void updateListPricesByProductCategory::sUpdate()
{
  if (_updateBy->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter a Update Percentage"),
                           tr("You must indicate the percentage to update the selected Pricing Schedule.") );
    _updateBy->setFocus();
    return;
  }

  QString sql( "SELECT updateListPrice(item_id, :updatePercent) "
               "FROM item" );

  if (_productCategory->isSelected())
    sql += " WHERE (item_prodcat_id=:prodcat_id);";
  else if (_productCategory->isPattern())
    sql += " WHERE (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)));";
  else
    sql += ";";

  q.prepare(sql);
  q.bindValue(":updatePercent", (1.0 + (_updateBy->toDouble() / 100.0)));
  _productCategory->bindValue(q);
  q.exec();

  accept();
}


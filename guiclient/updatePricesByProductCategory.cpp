/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updatePricesByProductCategory.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "guiclient.h"

/*
 *  Constructs a updatePricesByProductCategory as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updatePricesByProductCategory::updatePricesByProductCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));

  _productCategory->setType(ParameterGroup::ProductCategory);

  _updateBy->setValidator(new QDoubleValidator(-100, 9999, 2, _updateBy));
}

/*
 *  Destroys the object and frees any allocated resources
 */
updatePricesByProductCategory::~updatePricesByProductCategory()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updatePricesByProductCategory::languageChange()
{
    retranslateUi(this);
}

void updatePricesByProductCategory::sUpdate()
{
  if (_updateBy->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter a Update Percentage"),
                           tr("You must indicate the percentage to update the selected Pricing Schedule.") );
    _updateBy->setFocus();
    return;
  }

  QString sql;

  if (_percent->isChecked())
  {
  sql = "SELECT updatePrice(ipsitem_id, 'P', :updatePercent) "
        "FROM ipsitem, item "
        "WHERE ( (ipsitem_item_id=item_id)";
  }
  else
  {
  sql = "SELECT updatePrice(ipsitem_id, 'V', :updateValue) "
        "FROM ipsitem, item "
        "WHERE ( (ipsitem_item_id=item_id)";
  }

  if (_productCategory->isSelected())
    sql += " AND (item_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " AND (item_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  sql += ");";

  q.prepare(sql);

  if (_percent->isChecked())
    q.bindValue(":updatePercent", (1.0 + (_updateBy->toDouble() / 100.0)));
  else
    q.bindValue(":updateValue", _updateBy->toDouble());

  _productCategory->bindValue(q);
  q.exec();

/*
  sql = "UPDATE ipsprodcat"
        "   SET ipsprodcat_discntprcnt = ipsprodcat_discntprcnt - :updatePercent";

  if (_productCategory->isSelected())
    sql += " WHERE (ipsprodcat_prodcat_id=:prodcat_id)";
  else if (_productCategory->isPattern())
    sql += " WHERE (ipsprodcat_prodcat_id IN (SELECT prodcat_id FROM prodcat WHERE (prodcat_code ~ :prodcat_pattern)))";

  q.prepare(sql);
  q.bindValue(":updatePercent", (_updateBy->toDouble() / 100.0));
  _productCategory->bindValue(q);
  q.exec();
*/

  accept();
}


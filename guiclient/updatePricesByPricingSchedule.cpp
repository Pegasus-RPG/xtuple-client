/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "updatePricesByPricingSchedule.h"

#include <QVariant>
#include <QMessageBox>
#include <QValidator>
#include "guiclient.h"

/*
 *  Constructs a updatePricesByPricingSchedule as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
updatePricesByPricingSchedule::updatePricesByPricingSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_update, SIGNAL(clicked()), this, SLOT(sUpdate()));
  connect(_ipshead, SIGNAL(currentIndexChanged(int)), this, SLOT(sIPSChanged()));

  _ipshead->populate( "SELECT ipshead_id, (ipshead_name || '-' || ipshead_descrip) "
                      "FROM ipshead "
                      "ORDER BY ipshead_name;" );

  _updateBy->setValidator(new QDoubleValidator(-100, 9999, 2, _updateBy));
}

/*
 *  Destroys the object and frees any allocated resources
 */
updatePricesByPricingSchedule::~updatePricesByPricingSchedule()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void updatePricesByPricingSchedule::languageChange()
{
    retranslateUi(this);
}


void updatePricesByPricingSchedule::sUpdate()
{
  if (!_ipshead->isValid())
  {
    QMessageBox::critical( this, tr( "Select Pricing Schedule to Update"),
                           tr("You must select a Pricing Schedule to update.") );
    _ipshead->setFocus();
    return;
  }

  if (_updateBy->toDouble() == 0.0)
  {
    QMessageBox::critical( this, tr("Enter a Update Value"),
                           tr("You must indicate the value to update the selected Pricing Schedule.") );
    _updateBy->setFocus();
    return;
  }

  if (_value->isChecked())
  {
    q.prepare( "SELECT updatePrice(ipsitem_id, 'V' ,:value) "
               "FROM ipsitem "
               "WHERE (ipsitem_ipshead_id=:ipshead_id);" );
    q.bindValue(":value", _updateBy->toDouble());
    q.bindValue(":ipshead_id", _ipshead->id());
  }
  else
  {
    q.prepare( "SELECT updatePrice(ipsitem_id, 'P' ,:rate) "
               "FROM ipsitem "
               "WHERE (ipsitem_ipshead_id=:ipshead_id);" );
    q.bindValue(":rate", (1.0 + (_updateBy->toDouble() / 100.0)));
    q.bindValue(":ipshead_id", _ipshead->id());
  }

  q.exec();

/*
  q.prepare( "UPDATE ipsprodcat"
             "   SET ipsprodcat_discntprcnt = ipsprodcat_discntprcnt - :rate"
             " WHERE (ipsprodcat_ipshead_id=:ipshead_id);");
  q.bindValue(":rate", (_updateBy->toDouble() / 100.0));
  q.bindValue(":ipshead_id", _ipshead->id());
  q.exec();
*/

  accept();
}

void updatePricesByPricingSchedule::sIPSChanged()
{
/*
  q.prepare( "SELECT curr_symbol, curr_name "
             "FROM curr_symbol "
             "LEFT OUTER JOIN ipshead ON (ipshead_id=:ipshead_id) "
             "WHERE curr_id = ipshead_curr_id ");
  q.bindValue(":ipshead_id", _ipshead->id());
  q.exec();
  if(q.first())
  _value->setText(q.value("curr_symbol").toString()+" - "+q.value("curr_name").toString());
*/

}


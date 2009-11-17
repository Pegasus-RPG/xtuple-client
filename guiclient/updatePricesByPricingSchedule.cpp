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
#include "xdoublevalidator.h"

updatePricesByPricingSchedule::updatePricesByPricingSchedule(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);
    
    
    // signals and slots connections
    connect(_close,   SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update,  SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_value,   SIGNAL(clicked()), this, SLOT(sHandleCharPrice()));
    connect(_percent, SIGNAL(clicked()), this, SLOT(sHandleCharPrice()));
    
    _ipshead->populate( "SELECT ipshead_id, (ipshead_name || '-' || ipshead_descrip) "
                        "FROM ipshead "
                        "ORDER BY ipshead_name;" );
    
    _updateBy->setValidator(new XDoubleValidator(-100, 9999, decimalPlaces("curr"), _updateBy));
}

updatePricesByPricingSchedule::~updatePricesByPricingSchedule()
{
    // no need to delete child widgets, Qt does it all for us
}

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
        QMessageBox::critical( this, tr("Enter an Update Value"),
                               tr("You must provide a value to update the selected Pricing Schedule.") );
        _updateBy->setFocus();
        return;
    }
    
     q.prepare( "SELECT updatePricesByPricingSchedule(:ipshead_id, :type, :value, :updateCharPrices);" );
     q.bindValue(":ipshead_id", _ipshead->id());
     q.bindValue(":value", _updateBy->toDouble());
     
    if (_value->isChecked())
    {
        q.bindValue(":type", "V");
        q.bindValue(":updateCharPrices", false);       
    }
    else
    {
        q.bindValue(":type", "P");
        q.bindValue(":updateCharPrices", _updateCharPrices->isChecked());
    }
    
    q.exec();
    
    accept();
}

void updatePricesByPricingSchedule::sHandleCharPrice()
{
    // Only enable update char prices for percentage updates.
    _updateCharPrices->setEnabled( _percent->isChecked() );
}

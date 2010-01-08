/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
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
#include "xdoublevalidator.h"

updatePricesByProductCategory::updatePricesByProductCategory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);
    
    
    // signals and slots connections
    connect(_close,   SIGNAL(clicked()), this, SLOT(reject()));
    connect(_update,  SIGNAL(clicked()), this, SLOT(sUpdate()));
    connect(_value,   SIGNAL(clicked()), this, SLOT(sHandleCharPrice()));
    connect(_percent, SIGNAL(clicked()), this, SLOT(sHandleCharPrice()));
    
    _productCategory->setType(ParameterGroup::ProductCategory);
    
    _updateBy->setValidator(new XDoubleValidator(-100, 9999, decimalPlaces("curr"), _updateBy));
}

updatePricesByProductCategory::~updatePricesByProductCategory()
{
    // no need to delete child widgets, Qt does it all for us
}

void updatePricesByProductCategory::languageChange()
{
    retranslateUi(this);
}

void updatePricesByProductCategory::sUpdate()
{
    if (_updateBy->toDouble() == 0.0)
    {
        QMessageBox::critical( this, tr("Enter an Update Percentage"),
                               tr("You must provide a value to update the selected Product Categories.") );
        _updateBy->setFocus();
        return;
    }
    
    q.prepare( "SELECT updatePricesByProductCategory(:prodcat_id, :prodcat_pattern, :type, :value, :updateCharPrices);" );
    q.bindValue(":value", _updateBy->toDouble());
    
    if (_productCategory->isSelected())
    {
        q.bindValue(":prodcat_id", _productCategory->id());
    }
    else if (_productCategory->isPattern())
    {
        q.bindValue(":prodcat_pattern", _productCategory->pattern());
    }
    
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

void updatePricesByProductCategory::sHandleCharPrice()
{
    // Only enable update char prices for percentage updates.
    _updateCharPrices->setEnabled( _percent->isChecked() );
}

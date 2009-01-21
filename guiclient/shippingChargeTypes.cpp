/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shippingChargeTypes.h"

#include <qvariant.h>
#include <qmessagebox.h>
//#include <qstatusbar.h>
#include <parameter.h>
#include "shippingChargeType.h"
#include "guiclient.h"

/*
 *  Constructs a shippingChargeTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
shippingChargeTypes::shippingChargeTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shippingChargeTypes::~shippingChargeTypes()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shippingChargeTypes::languageChange()
{
    retranslateUi(this);
}


void shippingChargeTypes::init()
{
//  statusBar()->hide();
  
  if (_privileges->check("MaintainShippingChargeTypes"))
  {
    connect(_shipchrg, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_shipchrg, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_shipchrg, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
    _new->setEnabled(FALSE);

  _shipchrg->addColumn(tr("Name"),        _itemColumn, Qt::AlignLeft,   true,  "shipchrg_name" );
  _shipchrg->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "shipchrg_descrip" ); 
  
  sFillList();
}

void shippingChargeTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  shippingChargeType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingChargeTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("shipchrg_id", _shipchrg->id());

  shippingChargeType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void shippingChargeTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("shipchrg_id", _shipchrg->id());

  shippingChargeType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void shippingChargeTypes::sDelete()
{
  q.prepare("SELECT deleteShippingCharge(:shipchrg_id) AS result;");
  q.bindValue(":shipchrg_id", _shipchrg->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete Shipping Charge Type"),
                               tr( "The selected Shipping Charge Type cannot be delete as one or more Customers are assigned to it.\n"
                                   "You must reassigned these Customers before you may delete the selected Shipping Charge Type." ) );
        return;
    }

    sFillList();
  }
}

void shippingChargeTypes::sFillList()
{
  _shipchrg->populate( "SELECT shipchrg_id, shipchrg_name, shipchrg_descrip "
                      "FROM shipchrg "
                      "ORDER BY shipchrg_name;" );
}


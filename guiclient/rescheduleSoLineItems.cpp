/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "rescheduleSoLineItems.h"

#include <QVariant>
#include <QMessageBox>
#include "inputManager.h"
#include "salesOrderList.h"
#include "rescheduleSoLineItems.h"

/*
 *  Constructs a rescheduleSoLineItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rescheduleSoLineItems::rescheduleSoLineItems(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_salesOrderList, SIGNAL(clicked()), this, SLOT(sSoList()));
  connect(_so, SIGNAL(newId(int)), this, SLOT(sPopulate(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_reschedule, SIGNAL(clicked()), this, SLOT(sReschedule()));
  connect(_so, SIGNAL(valid(bool)), _reschedule, SLOT(setEnabled(bool)));
  connect(_so, SIGNAL(requestList()), this, SLOT(sSoList()));

  _captive = FALSE;

#ifndef Q_WS_MAC
  _salesOrderList->setMaximumWidth(25);
#endif

  omfgThis->inputManager()->notify(cBCSalesOrder, this, _so, SLOT(setId(int)));
}

/*
 *  Destroys the object and frees any allocated resources
 */
rescheduleSoLineItems::~rescheduleSoLineItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rescheduleSoLineItems::languageChange()
{
    retranslateUi(this);
}

enum SetResponse rescheduleSoLineItems::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("sohead_id", &valid);
  if (valid)
  {
    _so->setId(param.toInt());
    _so->setEnabled(FALSE);
    _date->setFocus();
  }

  return NoError;
}

void rescheduleSoLineItems::sReschedule()
{
  if (!_so->isValid())
  {
    QMessageBox::critical( this, tr("Select Sales Order"),
                          tr("You must select a Sales Order whose Line Item you wish to reschedule.") );
    _so->setFocus();
    return;
  }

  if (!_date->isValid())
  {
    QMessageBox::critical( this, tr("Enter New Scheduled Date"),
                           tr("You must enter a new Schedule Date." ) );
    _date->setFocus();
    return;
  }

  XSqlQuery reschedule;
  reschedule.prepare( "UPDATE coitem "
                      "SET coitem_scheddate=:newDate "
                      "WHERE ( (coitem_status NOT IN ('C','X'))"
                      "  AND   (NOT coitem_firm)"
                      "  AND   (coitem_cohead_id=:sohead_id) );" );
  reschedule.bindValue(":newDate", _date->date());
  reschedule.bindValue(":sohead_id", _so->id());
  reschedule.exec();

  if (_captive)
    accept();
  else
  {
    _so->setId(-1);
    _close->setText(tr("&Close"));
    _so->setFocus();
  }
}

void rescheduleSoLineItems::sSoList()
{
  ParameterList params;
  params.append("sohead_id", _so->id());
  params.append("soType", cSoOpen);
  
  salesOrderList newdlg(this, "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  if(id != QDialog::Rejected)
    _so->setId(id);
}

void rescheduleSoLineItems::sPopulate(int pSoheadid)
{
  if (pSoheadid != -1)
  {
    XSqlQuery query;
    query.prepare( "SELECT cohead_number,"
                   "       cohead_custponumber,"
                   "       cust_name, cust_phone "
                   "FROM cohead, cust "
                   "WHERE ( (cohead_cust_id=cust_id)"
                   " AND (cohead_id=:sohead_id) );" );
    query.bindValue(":sohead_id", pSoheadid); 
    query.exec();
    if (query.first())
    {
      _poNumber->setText(query.value("cohead_custponumber").toString());
      _custName->setText(query.value("cust_name").toString());
      _custPhone->setText(query.value("cust_phone").toString());
    }
  }
  else
  {
    _poNumber->clear();
    _custName->clear();
    _custPhone->clear();
  }
}


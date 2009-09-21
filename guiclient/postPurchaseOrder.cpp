/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postPurchaseOrder.h"

#include <qvariant.h>

/*
 *  Constructs a postPurchaseOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
postPurchaseOrder::postPurchaseOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_po, SIGNAL(valid(bool)), _post, SLOT(setEnabled(bool)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
postPurchaseOrder::~postPurchaseOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void postPurchaseOrder::languageChange()
{
    retranslateUi(this);
}


void postPurchaseOrder::init()
{
  _captive = FALSE;

  _po->setType(cPOUnposted);
}

enum SetResponse postPurchaseOrder::set(ParameterList &pParams)
{
  XDialog::set(pParams);
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("pohead_id", &valid);
  if (valid)
  {
    _po->setId(param.toInt());
    _post->setFocus();
  }

  return NoError;
}

void postPurchaseOrder::sPost()
{
  q.prepare("SELECT postPurchaseOrder(:pohead_id) AS result;");
  q.bindValue(":pohead_id", _po->id());
  q.exec();

  omfgThis->sPurchaseOrdersUpdated(_po->id(), TRUE);

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    _po->setId(-1);
    _po->setFocus();
  }
}


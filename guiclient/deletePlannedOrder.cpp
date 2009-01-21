/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "deletePlannedOrder.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "plCluster.h"

/*
 *  Constructs a deletePlannedOrder as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
deletePlannedOrder::deletePlannedOrder(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_planord, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
deletePlannedOrder::~deletePlannedOrder()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void deletePlannedOrder::languageChange()
{
    retranslateUi(this);
}


void deletePlannedOrder::init()
{
  _captive = FALSE;
}

enum SetResponse deletePlannedOrder::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("planord_id", &valid);
  if (valid)
  {
    _planord->setId(param.toInt());
    _planord->setReadOnly(TRUE);
    _delete->setFocus();
  }

  return NoError;
}

void deletePlannedOrder::sDelete()
{
  q.prepare( "SELECT deletePlannedOrder(:planord_id, :deleteChildren);" );
  q.bindValue(":planord_id", _planord->id());
  q.bindValue(":deleteChildren", QVariant(_deleteChildren->isChecked()));
  q.exec();

  if (_captive)
    accept();
  else
  {
    _planord->setId(-1);
    _close->setText(tr("&Close"));
    _planord->setFocus();
  }
}


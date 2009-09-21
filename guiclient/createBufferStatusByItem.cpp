/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createBufferStatusByItem.h"

#include <QVariant>
#include <QMessageBox>

#include "submitAction.h"

/*
 *  Constructs a createBufferStatusByItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
createBufferStatusByItem::createBufferStatusByItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_item, SIGNAL(valid(bool)), _create, SLOT(setEnabled(bool)));
  connect(_item, SIGNAL(newId(int)), _warehouse, SLOT(findItemsites(int)));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_create, SIGNAL(clicked()), this, SLOT(sCreate()));
  connect(_submit, SIGNAL(clicked()), this, SLOT(sSubmit()));

  if (!_metrics->boolean("EnableBatchManager"))
    _submit->hide();
    
  _captive = FALSE;
  
  //If not multi-warehouse hide whs control
  if (!_metrics->boolean("MultiWhs"))
  {
    _warehouseLit->hide();
    _warehouse->hide();
  }

}

/*
 *  Destroys the object and frees any allocated resources
 */
createBufferStatusByItem::~createBufferStatusByItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void createBufferStatusByItem::languageChange()
{
    retranslateUi(this);
}

enum SetResponse createBufferStatusByItem::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemsite_id", &valid);
  if (valid)
  {
    _captive = TRUE;

    _item->setItemsiteid(param.toInt());
    _create->setFocus();
  }

  return NoError;
}

void createBufferStatusByItem::sSubmit()
{
  ParameterList params;
  params.append("action_name", "CreateBufferStatus");
  params.append("item_id", _item->id());
  params.append("warehous_id", _warehouse->id());

  submitAction newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() == XDialog::Accepted)
    accept();
}

void createBufferStatusByItem::sCreate()
{
  q.prepare( "SELECT createBufferStatus(itemsite_id) "
             "FROM itemsite "
             "WHERE ( (itemsite_item_id=:item_id)"
             " AND (itemsite_active)"
             " AND (itemsite_warehous_id=:warehous_id) );" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":warehous_id", _warehouse->id());
  q.exec();

  if (_captive)
    accept();
  else
  {
    _close->setText(tr("&Close"));

    _item->setId(-1);
    _item->setFocus();
  }
}


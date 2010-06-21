/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspUnusedPurchasedItems.h"

#include <QMessageBox>

#include <qvariant.h>
//#include <qstatusbar.h>
#include <metasql.h>
#include <openreports.h>
#include <parameter.h>

#include "mqlutil.h"

/*
 *  Constructs a dspUnusedPurchasedItems as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
dspUnusedPurchasedItems::dspUnusedPurchasedItems(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_query, SIGNAL(clicked()), this, SLOT(sFillList()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
dspUnusedPurchasedItems::~dspUnusedPurchasedItems()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void dspUnusedPurchasedItems::languageChange()
{
    retranslateUi(this);
}


void dspUnusedPurchasedItems::init()
{
//  statusBar()->hide();
  
  _classCode->setType(ParameterGroup::ClassCode);

  _item->addColumn(tr("Item Number"), _itemColumn, Qt::AlignLeft,   true,  "item_number"  );
  _item->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "itemdescrip"  );
  _item->addColumn(tr("UOM"),         _uomColumn,  Qt::AlignLeft,   true,  "uom_name"  );
  _item->addColumn(tr("Total QOH"),   _qtyColumn,  Qt::AlignRight,  true,  "qoh" );
  _item->addColumn(tr("Last Cnt'd"),  _dateColumn, Qt::AlignRight,  true,  "lastcount" );
  _item->addColumn(tr("Last Used"),   _dateColumn, Qt::AlignRight,  true,  "lastused" );
}

void dspUnusedPurchasedItems::sPrint()
{
  ParameterList params;

  _classCode->appendValue(params);

  if(_includeUncontrolled->isChecked())
    params.append("includeUncontrolledItems");

  orReport report("UnusedPurchasedItems", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void dspUnusedPurchasedItems::sFillList()
{
  _item->clear();
  ParameterList params;
  if (! setParams(params))
    return;

  MetaSQLQuery mql = mqlLoad("unusedPurchasedItems", "detail");
  q = mql.toQuery(params);
  _item->populate(q);
}

bool dspUnusedPurchasedItems::setParams(ParameterList & params)
{
  if (_classCode->isPattern() && ! _classCode->isAll())
  {
    QString pattern = _classCode->pattern();
    if (pattern.length() == 0)
    {
      QMessageBox::warning(this, tr("Enter Class Code"),
                           tr("Class Code Pattern cannot be blank."));
	  return false;
    }
  }

  if (_classCode->isSelected())
    params.append("classcode_id", _classCode->id());
  else if (_classCode->isPattern()  && ! _classCode->isAll())
    params.append("classcode_pattern", _classCode->pattern());

  if (!_includeUncontrolled->isChecked())
    params.append("includeUncontrolled");

  return true;
}

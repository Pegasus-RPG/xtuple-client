/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "externalShipping.h"
#include "externalShippingList.h"

bool externalShippingList::userHasPriv()
{
  bool retval = _privileges->check("MaintainExternalShipping");
  return retval;
}

void externalShippingList::showEvent(QShowEvent *event)
{
  if (event)
    _screen->select();
  XWidget::showEvent(event);
}

void externalShippingList::setVisible(bool visible)
{
  if (! visible)
    XWidget::setVisible(false);
  else if (!userHasPriv())
  {
    systemError(this,
	        tr("You do not have sufficient privilege to view this window."),
		__FILE__, __LINE__);
    close();
  }
  else
    XWidget::setVisible(true);
}

externalShippingList::externalShippingList(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);
  
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
}

externalShippingList::~externalShippingList()
{
    // no need to delete child widgets, Qt does it all for us
}

void externalShippingList::languageChange()
{
    retranslateUi(this);
}

void externalShippingList::sNew()
{
  sOpen(cNew);
}

void externalShippingList::sEdit()
{
  sOpen(cEdit);
}

void externalShippingList::sView()
{
  sOpen(cView);
}

void externalShippingList::sDelete()
{
  _screen->removeCurrent();
  _screen->save();
}

void externalShippingList::sOpen(int mode)
{
  if (mode == cNew)
    _ship->clearSelection();
    
  ParameterList params;
  params.append("mode", mode);
  params.append("currentIndex",_screen->currentIndex());
  
  externalShipping newdlg(this, "", TRUE);
  newdlg.setModel(_screen->model());
  newdlg.set(params);
  newdlg.exec();
}

/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "booItemList.h"

#include <QSqlError>
#include <QVariant>

booItemList::booItemList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_select, SIGNAL(clicked()), this, SLOT(sSelect()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_clear, SIGNAL(clicked()), this, SLOT(sClear()));

  _booitem->addColumn(tr("#"),           _seqColumn, Qt::AlignCenter,true, "booitem_seqnumber");
  _booitem->addColumn(tr("Description"), -1,         Qt::AlignLeft,  true, "descrip");
}

booItemList::~booItemList()
{
  // no need to delete child widgets, Qt does it all for us
}

void booItemList::languageChange()
{
  retranslateUi(this);
}

enum SetResponse booItemList::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("booitem_seq_id", &valid);
  if (valid)
    _booitemseqid = param.toInt();

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _item->setId(param.toInt());
    _item->setReadOnly(TRUE);
  }

  return NoError;
}

void booItemList::sClose()
{
  done(_booitemseqid);
}

void booItemList::sClear()
{
  done(-1);
}

void booItemList::sSelect()
{
  done(_booitem->id());
}

void booItemList::sFillList()
{
  q.prepare( "SELECT booitem_seq_id, booitem_seqnumber,"
             "       (booitem_descrip1 || ' ' || booitem_descrip2) AS descrip "
             "FROM booitem(:item_id) "
             "WHERE ( (CURRENT_DATE BETWEEN booitem_effective AND booitem_expires) ) "
             "ORDER BY booitem_seqnumber;" );
  q.bindValue(":item_id", _item->id());
  q.exec();
  _booitem->populate(q, _booitemseqid );
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

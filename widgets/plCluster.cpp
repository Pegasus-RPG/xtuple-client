/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <xsqlquery.h>
#include <parameter.h>

#include "plannedOrderList.h"
#include "plCluster.h"

PlanOrdLineEdit::PlanOrdLineEdit(QWidget *pParent, const char *name) :
  XLineEdit(pParent, name)
{
  _qtyOrdered = 0.0;
  _mapper = new XDataWidgetMapper(this);

  connect(this, SIGNAL(lostFocus()), this, SLOT(sParse()));
  
}

void PlanOrdLineEdit::setId(int pId)
{
  XSqlQuery planord;
  planord.prepare( "SELECT formatPloNumber(planord_id) AS plnumber,"
                   "       warehous_code, item_number, uom_name,"
                   "       item_descrip1, item_descrip2,"
                   "       formatQty(planord_qty) AS qty,"
                   "       formatDate(planord_duedate) AS duedate "
                   "FROM planord, itemsite, item, warehous, uom "
                   "WHERE ( (planord_itemsite_id=itemsite_id)"
                   " AND (itemsite_item_id=item_id)"
                   " AND (item_inv_uom_id=uom_id)"
                   " AND (itemsite_warehous_id=warehous_id)"
                   " AND (planord_id=:planord_id) );" );
  planord.bindValue(":planord_id", pId);
  planord.exec();
  if (planord.first())
  {
    _id    = pId;
    _valid = TRUE;

    setText(planord.value("plnumber").toString());

    emit newId(_id);
    emit warehouseChanged(planord.value("warehous_code").toString());
    emit itemNumberChanged(planord.value("item_number").toString());
    emit uomChanged(planord.value("uom_name").toString());
    emit itemDescrip1Changed(planord.value("item_descrip1").toString());
    emit itemDescrip2Changed(planord.value("item_descrip2").toString());
    emit dueDateChanged(planord.value("duedate").toString());
    emit qtyChanged(planord.value("qty").toString());
    emit valid(TRUE);
  }
  else
  {
    _id    = -1;
    _valid = FALSE;

    setText("");

    emit newId(-1);
    emit warehouseChanged("");
    emit itemNumberChanged("");
    emit uomChanged("");
    emit itemDescrip1Changed("");
    emit itemDescrip2Changed("");
    emit dueDateChanged("");
    emit qtyChanged("");
    emit valid(FALSE);
  }
  
  if (_mapper->model() &&
    _mapper->model()->data(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this))).toString() != text())
      _mapper->model()->setData(_mapper->model()->index(_mapper->currentIndex(),_mapper->mappedSection(this)), text());
}

void PlanOrdLineEdit::sParse()
{
  if (text().contains('-'))
  {
    int number = text().left(text().find('-')).toInt();
    int subNumber = text().right(text().length() - text().find('-') - 1).toInt();

    XSqlQuery planord;
    planord.prepare( "SELECT planord_id "
                     "FROM planord "
                     "WHERE ( (planord_number=:planord_number)"
                     " AND (planord_subnumber=:planord_subnumber) );" );
    planord.bindValue(":planord_number", number);
    planord.bindValue(":planord_subnumber", subNumber);
    planord.exec();
    if (planord.first())
      setId(planord.value("planord_id").toInt());
    else
      setId(-1);
  }

  else if (text().length())
  {
    XSqlQuery planord;
    planord.prepare( "SELECT planord_id, planord_number "
                     "FROM planord "
                     "WHERE (planord_number=:planord_number);" );
    planord.bindValue(":planord_number", text().toInt());
    planord.exec();
    if (planord.first())
    {
      if (planord.size() == 1)
        setId(planord.value("planord_id").toInt());
      else
      {
        setId(-1);
        setText(planord.value("planord_number").toString() + "-");
        setFocus();
      }
    }
    else
      setId(-1);
  }
  else
    setId(-1);
}


PlanOrdCluster::PlanOrdCluster(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  constructor();
}

void PlanOrdCluster::constructor()
{
//  Create the component Widgets
  QVBoxLayout *_mainLayout       = new QVBoxLayout(this, 0, 0);
  QHBoxLayout *_firstLineLayout  = new QHBoxLayout(0, 0, 7);
  QHBoxLayout *_orderLayout      = new QHBoxLayout(0, 0, 5);
  QHBoxLayout *_warehouseLayout  = new QHBoxLayout(0, 0, 5);
  QHBoxLayout *_itemLineLayout   = new QHBoxLayout(0, 0, 7);
  QHBoxLayout *_itemNumberLayout = new QHBoxLayout(0, 0, 5);
  QHBoxLayout *_uomLayout        = new QHBoxLayout(0, 0, 5);
  QHBoxLayout *_statusLayout     = new QHBoxLayout(0, 0, 7);

  QLabel *_numberLit = new QLabel(tr("Planned Order #:"), this, "_numberLit");

  _numberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _orderLayout->addWidget(_numberLit);

  _number = new PlanOrdLineEdit(this);
  _orderLayout->addWidget(_number);

  _list = new QPushButton(tr("..."), this, "_list");
  _list->setFocusPolicy(Qt::NoFocus);
  _orderLayout->addWidget(_list);
  _firstLineLayout->addLayout(_orderLayout);

  QLabel *_warehouseLit = new QLabel(tr("Whs.:"), this, "_warehouseLit");
  _warehouseLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouseLit);

  _warehouse = new QLabel(this, "_warehouse");
  _warehouse->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _warehouseLayout->addWidget(_warehouse);
  _firstLineLayout->addLayout(_warehouseLayout);
  _mainLayout->addLayout(_firstLineLayout);

  QLabel *_itemNumberLit = new QLabel(tr("Item Number:"), this, "_itemNumberLit");
  _itemNumberLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _itemNumberLayout->addWidget(_itemNumberLit);

  _itemNumber = new QLabel(this, "_itemNumber");
  _itemNumber->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _itemNumberLayout->addWidget(_itemNumber);
  _itemLineLayout->addLayout(_itemNumberLayout);

  QLabel *_uomLit = new QLabel(tr("UOM:"), this, "_uomLit");
  _uomLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _uomLayout->addWidget(_uomLit);

  _uom = new QLabel(this, "_uom");
  _uom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _uomLayout->addWidget(_uom);
  _itemLineLayout->addLayout(_uomLayout);
  _mainLayout->addLayout(_itemLineLayout);

  _descrip1 = new QLabel(this, "_descrip1");
  _descrip1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip1);

  _descrip2 = new QLabel(this, "_descrip2");
  _descrip2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _mainLayout->addWidget(_descrip2);

  QLabel *_statusLit = new QLabel(tr("Status:"), this, "_statusLit");
  _statusLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  _statusLayout->addWidget(_statusLit);

  _status = new QLabel(this, "_status");
  _status->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  _status->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  _statusLayout->addWidget(_status);
  _mainLayout->addLayout(_statusLayout);

  setMinimumSize(6 * 50, 7 * 15);

//  Make some internal connections
  connect(_number, SIGNAL(itemNumberChanged(const QString &)), _itemNumber, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(uomChanged(const QString &)), _uom, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(itemDescrip1Changed(const QString &)), _descrip1, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(itemDescrip2Changed(const QString &)), _descrip2, SLOT(setText(const QString &)));
  connect(_number, SIGNAL(warehouseChanged(const QString &)), _warehouse, SLOT(setText(const QString &)));

  connect(_number, SIGNAL(newId(int)), this, SIGNAL(newId(int)));
  connect(_number, SIGNAL(dueDateChanged(const QString &)), this, SIGNAL(dueDateChanged(const QString &)));
  connect(_number, SIGNAL(qtyChanged(const QString &)), this, SIGNAL(qtyChanged(const QString &)));
  connect(_number, SIGNAL(valid(bool)), this, SIGNAL(valid(bool)));

  connect(_list, SIGNAL(clicked()), SLOT(sList()));

  setFocusProxy(_number);
}

void PlanOrdCluster::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
  {
    _number->setEnabled(FALSE);
    _list->hide();
  }
  else
  {
    _number->setEnabled(TRUE);
    _list->show();
  }
}

void PlanOrdCluster::setId(int pId)
{
  _number->setId(pId);
}

void PlanOrdCluster::setNumber(const QString& number)
{
  if (_number->text() == number)
    return;
  
  _number->setText(number);
  _number->sParse();
}

void PlanOrdCluster::sList()
{
  ParameterList params;
  params.append("planord_id", _number->_id);

  plannedOrderList newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);

  int id = newdlg.exec();
  setId(id);

  if (id != -1)
  {
    _number->setFocus();
    focusNextPrevChild(TRUE);
  }
}

QString PlanOrdCluster::woNumber() const
{
  return _number->text();
}

void PlanOrdCluster::setDataWidgetMap(XDataWidgetMapper* m)
{
  m->addMapping(this, _fieldName, "number", "defaultNumber");
  _number->_mapper=m;
}


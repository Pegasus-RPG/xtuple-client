/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "releaseTransferOrdersByClassCode.h"

#include <QVariant>
#include <QMessageBox>

releaseTransferOrdersByClassCode::releaseTransferOrdersByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  connect(_release, SIGNAL(clicked()), this, SLOT(sRelease()));

  _classCode->setType(XComboBox::ClassCodes);
}

releaseTransferOrdersByClassCode::~releaseTransferOrdersByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void releaseTransferOrdersByClassCode::languageChange()
{
  retranslateUi(this);
}

void releaseTransferOrdersByClassCode::sRelease()
{
  ParameterList params;
  int countRelease = 0;
  int _counter = 0;
  QString tonotreleased;

  QString sql("SELECT DISTINCT tohead_id "
              "FROM (SELECT DISTINCT tohead_id, toitem_id, item_number, item_classcode_id "
              "      FROM tohead, toitem, item "
              "      WHERE((toitem_tohead_id=tohead_id) "
              "         AND(toitem_item_id = item_id) "
              "         AND(toitem_status='U') ");
  if(_selectedClassCode->isChecked())
  sql+=       "         AND(item_classcode_id = :classcode_id) ";

  sql+=       "     )) AS data ";
  if(_selectedClassCode->isChecked())
  sql+=       "WHERE tohead_id NOT IN (SELECT DISTINCT tohead_id "
              "      FROM tohead, toitem, item "
              "      WHERE((toitem_tohead_id=tohead_id) "
              "         AND(toitem_item_id = item_id) "
              "         AND(toitem_status='U') "
              "         AND(item_classcode_id <> :classcode_id)));";

  q.prepare(sql);
  q.bindValue(":classcode_id", _classCode->id());
  q.exec();
  while (q.next())
  {
    XSqlQuery qry;
    qry.prepare("SELECT releaseTransferOrder(:tohead_id) AS result;");
    qry.bindValue(":tohead_id", q.value("tohead_id"));
    qry.exec();
    if (qry.first())
    {
      if ((qry.value("result").toInt()) > 0)
        countRelease++;
    }
    omfgThis->sTransferOrdersUpdated(-1);
  }

  QMessageBox::information( this, tr("Transfer Orders Released"),
                            tr("%1 Transfer Orders have been released.")
                            .arg(countRelease));

  if (_selectedClassCode->isChecked())
  {
    QString qry("SELECT DISTINCT tohead_id, tohead_number "
                "FROM (SELECT DISTINCT tohead_id, tohead_number "
                "      FROM tohead, toitem, item "
                "      WHERE((toitem_tohead_id=tohead_id) "
                "         AND(toitem_item_id = item_id) "
                "         AND(toitem_status='U') "
                "         AND(item_classcode_id <> :classcode_id))) AS data "
                "WHERE tohead_id IN (SELECT DISTINCT tohead_id "
                "                    FROM tohead, toitem, item "
                "                    WHERE((toitem_tohead_id=tohead_id) "
                "                       AND(toitem_item_id = item_id) "
                "                       AND(toitem_status='U') "
                "                       AND(item_classcode_id = :classcode_id)));");

    q.prepare(qry);
    q.bindValue(":classcode_id", _classCode->id());
    q.exec();
    while (q.next())
    {
      _counter = _counter + 1;
      if (_counter == 1)
        tonotreleased = tonotreleased + q.value("tohead_number").toString() ;
      else if (_counter > 1)
        tonotreleased = tonotreleased + ", " +q.value("tohead_number").toString() ;
    }

    if (_counter > 0)
        QMessageBox::information(this, tr("Transfer Orders Can not be Released"),
                                tr("One or more Transfer Orders: %1 could not be released because "
                                    "they contain items from another classcode.")
                                   .arg(tonotreleased));
  }
  accept();
}

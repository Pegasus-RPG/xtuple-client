/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bbomItem.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvalidator.h>
#include "bbomItem.h"

/*
 *  Constructs a bbomItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
bbomItem::bbomItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_item, SIGNAL(valid(bool)), _save, SLOT(setEnabled(bool)));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_item, SIGNAL(typeChanged(const QString &)), this, SLOT(sHandleItemType(const QString&)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
bbomItem::~bbomItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void bbomItem::languageChange()
{
    retranslateUi(this);
}


void bbomItem::init()
{
  _item->setType((ItemLineEdit::cCoProduct | ItemLineEdit::cByProduct));

  _dates->setStartNull(tr("Always"), omfgThis->startOfTime(), TRUE);
  _dates->setStartCaption(tr("Effective"));
  _dates->setEndNull(tr("Never"), omfgThis->endOfTime(), TRUE);
  _dates->setEndCaption(tr("Expires"));
  _qtyPer->setValidator(omfgThis->qtyPerVal());
  _costAbsorption->setValidator(omfgThis->percentVal());
}

enum SetResponse bbomItem::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("bbomitem_id", &valid);
  if (valid)
  {
    _bbomitemid = param.toInt();
    populate();
  }

  param = pParams.value("item_id", &valid);
  if (valid)
    _itemid = param.toInt();

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      q.exec("SELECT NEXTVAL('bbomitem_bbomitem_id_seq') AS _bbomitem_id");
      if (q.first())
        _bbomitemid = q.value("_bbomitem_id").toInt();
//  ToDo

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);

      _qtyPer->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _item->setReadOnly(TRUE);
      _qtyPer->setEnabled(FALSE);
      _costAbsorption->setEnabled(FALSE);
      _unique->setEnabled(FALSE);
      _dates->setEnabled(FALSE);
      _comments->setEnabled(FALSE);
      _close->setText(tr("&Close"));
      _save->hide();
    
      _close->setFocus();
    }
  }

  return NoError;
}

void bbomItem::sSave()
{
  if (_unique->isChecked())
  {
    q.prepare( "SELECT bbomitem_id "
               "FROM bbomitem "
               "WHERE ( (bbomitem_item_id=:item_id) "
               " AND (bbomitem_expires>=CURRENT_DATE)"
               " AND (bbomitem_id<>:bbomitem_id) ) "
               "LIMIT 1;" );
    q.bindValue(":item_id", _item->id());
    q.bindValue(":bbomitem_id", _bbomitemid);
    q.exec();
    if (q.first())
    {
      QMessageBox::warning( this, tr("Cannot Save Breeder BOM Item"),
                            tr( "There are other Breeder BOM Items that define the production of the selected Co/By-Product.\n"
                                "Because of this, you may not indicate that this Breeder BOM Item defines a unique manufacturing path\n"
                                "for the selected Co/By-Product without first expiring the other Breeder BOM Items." ) );
      _unique->setFocus();
      return;
    }
  }

  if (_mode == cNew)
    q.prepare( "INSERT INTO bbomitem "
               "( bbomitem_id, bbomitem_parent_item_id, bbomitem_item_id,"
               "  bbomitem_seqnumber, bbomitem_uniquemfg,"
               "  bbomitem_qtyper, bbomitem_effective, bbomitem_expires,"
               "  bbomitem_costabsorb, bbomitem_comments ) "
               "VALUES "
               "( :bbomitem_id, :bbomitem_parent_item_id, :bbomitem_item_id,"
               "  :bbomitem_seqnumber, :bbomitem_uniquemfg,"
               "  :bbomitem_qtyper, :bbomitem_effective, :bbomitem_expires,"
               "  :bbomitem_costabsorb, :bbomitem_comments );" );
  else if (_mode == cEdit)
    q.prepare( "UPDATE bbomitem "
               "SET bbomitem_qtyper=:bbomitem_qtyper, bbomitem_uniquemfg=:bbomitem_uniquemfg,"
               "    bbomitem_effective=:bbomitem_effective, bbomitem_expires=:bbomitem_expires,"
               "    bbomitem_costabsorb=:bbomitem_costabsorb,"
               "    bbomitem_comments=:bbomitem_comments "
               "WHERE (bbomitem_id=:bbomitem_id);" );

  q.bindValue(":bbomitem_id", _bbomitemid);
  q.bindValue(":bbomitem_parent_item_id", _itemid);
  q.bindValue(":bbomitem_item_id", _item->id());
  q.bindValue(":bbomitem_qtyper", _qtyPer->toDouble());
  q.bindValue(":bbomitem_uniquemfg", QVariant(_unique->isChecked()));
  q.bindValue(":bbomitem_effective", _dates->startDate());
  q.bindValue(":bbomitem_expires", _dates->endDate());
  q.bindValue(":bbomitem_costabsorb", (_costAbsorption->toDouble() / 100.0));
  q.bindValue(":bbomitem_comments",   _comments->toPlainText());
  q.exec();

  omfgThis->sBBOMsUpdated(_itemid, TRUE);

  done(_bbomitemid);
}

void bbomItem::sHandleItemType(const QString &pItemType)
{
  if (pItemType == "C")
    _costAbsorption->setEnabled(TRUE);
  else
    _costAbsorption->setEnabled(FALSE);
}

void bbomItem::populate()
{
  q.prepare( "SELECT bbomitem_item_id, bbomitem_parent_item_id,"
             "       item_type, bbomitem_uniquemfg,"
             "       bbomitem_qtyper,"
             "       bbomitem_effective, bbomitem_expires,"
             "       bbomitem_costabsorb,"
             "       bbomitem_comments "
             "FROM bbomitem, item "
             "WHERE ( (bbomitem_item_id=item_id)"
             " AND (bbomitem_id=:bbomitem_id) );" );
  q.bindValue(":bbomitem_id", _bbomitemid);
  q.exec();
  if (q.first())
  {
    _itemid = q.value("bbomitem_parent_item_id").toInt();
    _item->setId(q.value("bbomitem_item_id").toInt());
    _qtyPer->setDouble(q.value("bbomitem_qtyper").toDouble());
    _unique->setChecked(q.value("bbomitem_uniquemfg").toBool());
    _dates->setStartDate(q.value("bbomitem_effective").toDate());
    _dates->setEndDate(q.value("bbomitem_expires").toDate());

    if (q.value("item_type").toString() == "C")
      _costAbsorption->setDouble(q.value("bbomitem_costabsorb").toDouble() * 100.0);

    _comments->setText(q.value("bbomitem_comments").toString());
  }
}

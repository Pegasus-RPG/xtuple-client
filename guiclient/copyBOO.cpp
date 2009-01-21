/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "copyBOO.h"

#include <QVariant>
#include <QMessageBox>

/*
 *  Constructs a copyBOO as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
copyBOO::copyBOO(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_copy, SIGNAL(clicked()), this, SLOT(sCopy()));
  connect(_source, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_target, SIGNAL(valid(bool)), this, SLOT(sHandleButtons()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _captive = FALSE;

  _source->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cPlanning | ItemLineEdit::cJob);
  _target->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cPlanning | ItemLineEdit::cJob);
}

/*
 *  Destroys the object and frees any allocated resources
 */
copyBOO::~copyBOO()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void copyBOO::languageChange()
{
  retranslateUi(this);
}

enum SetResponse copyBOO::set(const ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
  {
    _source->setId(param.toInt());
    _source->setEnabled(FALSE);
    _target->setFocus();
  }

  return NoError;
}

void copyBOO::sCopy()
{
  q.prepare( "SELECT booitem_id "
             "FROM booitem "
             "WHERE (booitem_item_id=:item_id);" );
  q.bindValue(":item_id", _source->id());
  q.exec();
  if (!q.first())
    QMessageBox::information( this, tr("Empty Bill of Operations"),
                              tr("The selected source Item does not have any BOO items associated with it."));

  else
  {
    q.prepare( "SELECT booitem_id "
               "FROM booitem "
               "WHERE (booitem_item_id=:item_id);" );
    q.bindValue(":item_id", _target->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Existing Bill of Operations"),
                                tr( "The selected target Item already has a Bill of Operations associated with it.\n"
                                    "You must first delete the Bill of Operations for the selected target item\n"
                                    "before attempting copy an existing BOO." ));
    else
    {
      q.prepare("SELECT copyBOO(:sourceid, :targetid) AS result;");
      q.bindValue(":sourceid", _source->id());
      q.bindValue(":targetid", _target->id());
      q.exec();

      if(q.first() && q.value("result").toInt() < 0)
        QMessageBox::information( this, tr("Existing Bill of Operations"),
                                  tr( "The selected target Item already has a Bill of Operations associated with it.\n"
                                      "You must first delete the Bill of Operations for the selected target item\n"
                                      "before attempting copy an existing BOO." ));

      omfgThis->sBOOsUpdated(_target->id(), TRUE);
    }
  }

  if (_captive)
    accept();
  else
  {
    _source->setId(-1);
    _target->setId(-1);
    _source->setFocus();
  }
}

void copyBOO::sHandleButtons()
{
  _copy->setEnabled(_source->isValid() && _target->isValid());
}


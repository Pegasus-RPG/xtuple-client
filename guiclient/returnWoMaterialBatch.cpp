/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "returnWoMaterialBatch.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include "inputManager.h"
#include "distributeInventory.h"

/*
 *  Constructs a returnWoMaterialBatch as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
returnWoMaterialBatch::returnWoMaterialBatch(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_wo, SIGNAL(valid(bool)), _return, SLOT(setEnabled(bool)));
    connect(_return, SIGNAL(clicked()), this, SLOT(sReturn()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
returnWoMaterialBatch::~returnWoMaterialBatch()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void returnWoMaterialBatch::languageChange()
{
    retranslateUi(this);
}


void returnWoMaterialBatch::init()
{
  _captive = FALSE;

  omfgThis->inputManager()->notify(cBCWorkOrder, this, _wo, SLOT(setId(int)));

  _wo->setType(cWoExploded | cWoReleased | cWoIssued);
}

enum SetResponse returnWoMaterialBatch::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("wo_id", &valid);
  if (valid)
  {
    _wo->setId(param.toInt());
    _return->setFocus();
  }

  return NoError;
}

void returnWoMaterialBatch::sReturn()
{
  q.prepare( "SELECT wo_qtyrcv "
             "FROM wo "
             "WHERE (wo_id=:wo_id);" );
  q.bindValue(":wo_id", _wo->id());
  q.exec();
  if (q.first())
  {
    if (_wo->method() == "A" && q.value("wo_qtyrcv").toDouble() != 0)
    {
      QMessageBox::warning( this, tr("Cannot return Work Order Material"),
                            tr( "This Work Order has had material received against it\n"
                                "and thus the material issued against it cannot be returned.\n"
                                "You must instead return each Work Order Material item individually.\n" ) );
      _wo->setId(-1);
      _wo->setFocus();
    }
    else
    {
      XSqlQuery rollback;
      rollback.prepare("ROLLBACK;");

      q.exec("BEGIN;");	// because of possible lot, serial, or location distribution cancelations
      q.prepare("SELECT returnWoMaterialBatch(:wo_id) AS result;");
      q.bindValue(":wo_id", _wo->id());
      q.exec();
      if (q.first())
      {
        if (q.value("result").toInt() < 0)
        {
          rollback.exec();
          systemError( this, tr("A System Error occurred at returnWoMaterialBatch::%1, W/O ID #%2, Error #%3.")
                             .arg(__LINE__)
                             .arg(_wo->id())
                             .arg(q.value("result").toInt()) );
          return;
        }
        else if (distributeInventory::SeriesAdjust(q.value("result").toInt(), this) == XDialog::Rejected)
        {
          rollback.exec();
          QMessageBox::information( this, tr("Material Return"), tr("Transaction Canceled") );
          return;
        }

        q.exec("COMMIT;");
      }
      else
      {
        rollback.exec();
        systemError( this, tr("A System Error occurred at returnWoMaterialBatch::%1, W/O ID #%2.")
                           .arg(__LINE__)
                           .arg(_wo->id()) );
        return;
      }
    }
  }

  if (_captive)
    accept();
  else
  {
    _wo->setId(-1);
    _wo->setFocus();
  }
}

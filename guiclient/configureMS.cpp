/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureMS.h"

#include <QVariant>
#include <QValidator>

/*
 *  Constructs a configureMS as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureMS::configureMS(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _nextPlanNumber->setValidator(omfgThis->orderVal());

  q.exec("SELECT currentPlanNumber() AS result;");
  if (q.first())
    _nextPlanNumber->setText(q.value("result").toString());

  int cid = _metrics->value("DefaultMSCalendar").toInt();
  if(cid > 0)
    _calendar->setId(cid);
    
  if (_metrics->value("Application") == "Manufacturing")  
    _bufferMgt->setChecked(_metrics->boolean("BufferMgt"));
  else
    _bufferMgt->hide();
    
  this->setWindowTitle("Schedule Configuration");
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureMS::~configureMS()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureMS::languageChange()
{
    retranslateUi(this);
}

void configureMS::sSave()
{
  q.prepare("SELECT setNextPlanNumber(:planord_number) AS result;");
  q.bindValue(":planord_number", _nextPlanNumber->text().toInt());
  q.exec();

  _metrics->set("DefaultMSCalendar", _calendar->id());
  _metrics->set("BufferMgt", _bufferMgt->isChecked());

  _metrics->load();

  accept();
}

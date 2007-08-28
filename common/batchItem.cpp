/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "batchItem.h"

#include <qvariant.h>
#include <xsqlquery.h>

/*
 *  Constructs a batchItem as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
batchItem::batchItem(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_reschedule, SIGNAL(toggled(bool)), _interval, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
batchItem::~batchItem()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void batchItem::languageChange()
{
    retranslateUi(this);
}


#define cUndefined    0x00
#define cReschedule  0x10

void batchItem::init()
{
  _mode = cUndefined;
  _batchid = -1;
  _db = QSqlDatabase();
}

int batchItem::Reschedule( int pBatchid, QWidget * pParent )
{
  return Reschedule(pBatchid, pParent, QSqlDatabase());
}

int batchItem::Reschedule(int pBatchid, QWidget *pParent, QSqlDatabase pDb)
{
  batchItem newdlg(pParent);

  newdlg._mode = cReschedule;
  newdlg._batchid = pBatchid;
  newdlg._db = pDb;
  newdlg.populate();

  newdlg._email->setEnabled(FALSE);

  return newdlg.exec();
}

void batchItem::sSave()
{
  QString interval;

  if (_reschedule->isChecked())
  {
    switch (_interval->currentItem())
    {
      case 0:
        interval = "D";
        break;

      case 1:
        interval = "W";
        break;

      case 2:
        interval = "M";
        break;
    }
  }

  if (interval.length() == 0)
    interval = "N";

  if (_mode == cReschedule)
    XSqlQuery(_db).exec( QString("SELECT rescheduleBatchItem(%1, '%2 %3', '%4') AS resul;")
                      .arg(_batchid)
                      .arg(_scheduledDate->dateString())
                      .arg(_scheduledTime->time().toString("hh:mm:ss"))
                      .arg(interval) ); 
  done(_batchid);
}

void batchItem::populate()
{
  XSqlQuery batch(_db);
  batch.prepare( "SELECT batch_action, batch_parameter,"
                 "       batch_user, batch_email, batch_reschedinterval,"
                 "       date(batch_submitted) AS submitted_date,"
                 "       CAST(batch_submitted AS TIME) AS submitted_time,"
                 "       date(batch_scheduled) AS scheduled_date,"
                 "       CAST(batch_scheduled AS TIME) AS scheduled_time,"
                 "       date(batch_started) AS started_date,"
                 "       CAST(batch_started AS TIME) AS started_time,"
                 "       date(batch_completed) AS completed_date,"
                 "       CAST(batch_completed AS TIME) AS completed_time "
                 "FROM batch "
                 "WHERE (batch_id=:batch_id);" );
  batch.bindValue(":batch_id", _batchid);
  batch.exec();
  if (batch.first())
  {
    _submittedBy->setText(batch.value("batch_user").toString());
    _email->setText(batch.value("batch_email").toString());
    _action->setText(batch.value("batch_action").toString());
    _parameter->setText(batch.value("batch_parameter").toString());

    QString interval = batch.value("batch_reschedinterval").toString();
    if (interval != "N")
    {
      _reschedule->setChecked(TRUE);

      if (interval == "D")
        _interval->setCurrentItem(0);
      else if (interval == "W")
        _interval->setCurrentItem(1);
      else if (interval == "M")
        _interval->setCurrentItem(2);
    }

    _submittedDate->setDate(batch.value("submitted_date").toDate());
    _submittedTime->setTime(batch.value("submitted_time").toTime());
    _scheduledDate->setDate(batch.value("scheduled_date").toDate());
    _scheduledTime->setTime(batch.value("scheduled_time").toTime());
    _startedDate->setDate(batch.value("started_date").toDate());
    _startedTime->setTime(batch.value("started_time").toTime());
    _completedDate->setDate(batch.value("completed_date").toDate());
    _completedTime->setTime(batch.value("completed_time").toTime());
  }
}


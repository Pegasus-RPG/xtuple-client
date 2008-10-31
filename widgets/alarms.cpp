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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#include <QDesktopServices>
#include <QDialog>

#include <parameter.h>
#include <xsqlquery.h>

#include "alarmMaint.h"
#include "alarms.h"


const Alarms::AlarmMap Alarms::_alarmMap[] =
{
  AlarmMap( Uninitialized,     " "   ),
  AlarmMap( Address,           "ADDR"),
  AlarmMap( BBOMHead,          "BBH" ),
  AlarmMap( BBOMItem,          "BBI" ),
  AlarmMap( BOMHead,           "BMH" ),
  AlarmMap( BOMItem,           "BMI" ),
  AlarmMap( BOOHead,           "BOH" ),
  AlarmMap( BOOItem,           "BOI" ),
  AlarmMap( CRMAccount,        "CRMA"),
  AlarmMap( Contact,           "T"   ),
  AlarmMap( Customer,          "C"   ),
  AlarmMap( Employee,          "EMP" ),
  AlarmMap( Incident,          "INCDT"),
  AlarmMap( Item,              "I"   ),
  AlarmMap( ItemSite,          "IS"  ),
  AlarmMap( ItemSource,        "IR"  ),
  AlarmMap( Location,          "L"   ),
  AlarmMap( LotSerial,         "LS"   ),
  AlarmMap( Opportunity,       "OPP" ),
  AlarmMap( Project,           "J"   ),
  AlarmMap( PurchaseOrder,     "P"   ),
  AlarmMap( PurchaseOrderItem, "PI"  ),
  AlarmMap( ReturnAuth,        "RA"  ),
  AlarmMap( ReturnAuthItem,    "RI"  ),
  AlarmMap( Quote,             "Q"   ),
  AlarmMap( QuoteItem,         "QI"  ),
  AlarmMap( SalesOrder,        "S"   ),
  AlarmMap( SalesOrderItem,    "SI"  ),
  AlarmMap( TodoItem,          "TODO" ),
  AlarmMap( TransferOrder,     "TO"  ),
  AlarmMap( TransferOrderItem, "TI"  ),
  AlarmMap( Vendor,            "V"   ),
  AlarmMap( Warehouse,         "WH"  ),
  AlarmMap( WorkOrder,         "W"   ),
};

Alarms::Alarms(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;
  _recipient1 = -1;
  _recipient2 = -1;
  XSqlQuery tickle;
  tickle.exec( "SELECT CURRENT_TIME AS dbdate;" );
  if (tickle.first())
  {
    _dueDate = tickle.value("dbdate").toDate();
    _dueTime = tickle.value("dbdate").toTime();
  }


  _alarms->addColumn(tr("Qualifier"),        _itemColumn,   Qt::AlignLeft, true, "f_offset" );
  _alarms->addColumn(tr("Due"),              -1,            Qt::AlignLeft, true, "alarm_time" );
  _alarms->addColumn(tr("Event Recipient"),  _itemColumn,   Qt::AlignLeft, true, "alarm_event_recipient" );
  _alarms->addColumn(tr("Email Recipient"),  _itemColumn,   Qt::AlignLeft, true, "alarm_email_recipient" );
  _alarms->addColumn(tr("SysMsg Recipient"), _itemColumn,   Qt::AlignLeft, true, "alarm_sysmsg_recipient" );

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
}

void Alarms::setType(enum AlarmSources pSource)
{
  _source = pSource;
}

void Alarms::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Alarms::setRecipient1(int pRecipient)
{
  _recipient1 = pRecipient;
}

void Alarms::setRecipient2(int pRecipient)
{
  _recipient2 = pRecipient;
}

void Alarms::setDate(QDate pDate)
{
  _dueDate = pDate;
}

void Alarms::setReadOnly(bool pReadOnly)
{
  _new->setEnabled(!pReadOnly);
  _edit->setEnabled(!pReadOnly);
  _delete->setEnabled(!pReadOnly);
  
}

void Alarms::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("source", _source);
  params.append("source_id", _sourceid);
  params.append("due_date", _dueDate);
  params.append("recipient1", _recipient1);
  params.append("recipient2", _recipient2);

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Alarms::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("alarm_id", _alarms->id());

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Alarms::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("alarm_id", _alarms->altId());

  alarmMaint newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Alarms::sDelete()
{
  XSqlQuery q;
  q.prepare( "DELETE FROM alarm "
             "WHERE (alarm_id=:alarm_id);" );
  q.bindValue(":alarm_id", _alarms->id());
  q.exec();

  refresh();
}

void Alarms::refresh()
{
  if(-1 == _sourceid)
  {
    _alarms->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate alarms
  QString sql( "SELECT alarm_id,"
               "       CASE WHEN (alarm_time_offset > 0) THEN"
               "                 CAST(alarm_time_offset AS TEXT) || ' ' || "
               "                 CASE WHEN (alarm_time_qualifier = 'MB') THEN :minutesbefore"
               "                      WHEN (alarm_time_qualifier = 'HB') THEN :hoursbefore"
               "                      WHEN (alarm_time_qualifier = 'DB') THEN :daysbefore"
               "                      WHEN (alarm_time_qualifier = 'MA') THEN :minutesafter"
               "                      WHEN (alarm_time_qualifier = 'HA') THEN :hoursafter"
               "                      WHEN (alarm_time_qualifier = 'DA') THEN :daysafter"
               "                 END"
               "            ELSE '' "
               "       END AS f_offset, alarm_time,"
               "       CASE WHEN (alarm_event) THEN alarm_event_recipient END AS alarm_event_recipient,"
               "       CASE WHEN (alarm_email) THEN alarm_email_recipient END AS alarm_email_recipient,"
               "       CASE WHEN (alarm_sysmsg) THEN alarm_sysmsg_recipient END AS alarm_sysmsg_recipient "
               "FROM alarm "
               "WHERE ( (alarm_source=:source) "
               "  AND   (alarm_source_id=:sourceid) ) "
               "ORDER BY alarm_time; ");
  query.prepare(sql);
  query.bindValue(":email", tr("Email"));
  query.bindValue(":event", tr("Event"));
  query.bindValue(":sysmsg", tr("System Message"));
  query.bindValue(":other", tr("Other"));
  query.bindValue(":minutesbefore", tr("minutes before"));
  query.bindValue(":hoursbefore", tr("hours before"));
  query.bindValue(":daysbefore", tr("days before"));
  query.bindValue(":minutesafter", tr("mintues after"));
  query.bindValue(":hoursafter", tr("hours after"));
  query.bindValue(":daysafter", tr("days after"));
  query.bindValue(":source", _alarmMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _alarms->populate(query);
}



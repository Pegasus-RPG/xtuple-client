/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this are subject to the Common Public Attribution 
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

#include <QFileDialog>
#include <QUrl>

#include "alarmMaint.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <qvariant.h>

const char *_alarmQualifiers[] = { "MB", "HB", "DB", "MA", "HA", "DA" };

/*
 *  Constructs a file as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
alarmMaint::alarmMaint(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);

    // signals and slots connections
    connect(_cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_contactCluster, SIGNAL(newId(int)), this, SLOT(sContactLookup(int)));
    connect(_usernameCluster, SIGNAL(newId(int)), this, SLOT(sUserLookup(int)));

    _mode = cNew;
    _source = Alarms::Uninitialized;
    _sourceid = -1;
    _alarmid = -1;
    XSqlQuery tickle;
    tickle.exec( "SELECT CURRENT_TIME AS dbdate;" );
    if (tickle.first())
    {
      _alarmDate->setDate(tickle.value("dbdate").toDate());
      _alarmTime->setTime(tickle.value("dbdate").toTime());
    }
    
    _eventAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmEventDefault"));
    if (_x_metrics)
    {
      if (_x_metrics->boolean("EnableBatchManager"))
        _emailAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmEmailDefault"));
      else
      {
        _emailAlarm->hide();
        _emailRecipient->hide();
      }
    }
    _sysmsgAlarm->setChecked(_x_preferences && _x_preferences->boolean("AlarmSysmsgDefault"));

    sHandleButtons();
}

/*
 *  Destroys the object and frees any allocated resources
 */
alarmMaint::~alarmMaint()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void alarmMaint::languageChange()
{
    retranslateUi(this);
}

void alarmMaint::set( const ParameterList & pParams )
{
  QVariant param;
  bool        valid;
  
  param = pParams.value("source", &valid);
  if (valid)
  {
    _source = (enum Alarms::AlarmSources)param.toInt();
    if ( (Alarms::_alarmMap[_source].ident == "TODO") || (Alarms::_alarmMap[_source].ident == "J") )
      _alarmDate->setEnabled(false);
  }
    
  param = pParams.value("source_id", &valid);
  if(valid)
    _sourceid = param.toInt();

  param = pParams.value("due_date", &valid);
  if(valid)
    _alarmDate->setDate(param.toDate());

  param = pParams.value("usrId1", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("usrId2", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("usrId3", &valid);
  if(valid)
    sGetUser(param.toInt());

  param = pParams.value("cntctId1", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("cntctId2", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("cntctId3", &valid);
  if(valid)
    sGetContact(param.toInt());

  param = pParams.value("mode", &valid);
  if(valid)
  {
    if(param.toString() == "new")
    {
      _mode = cNew;
    }
    else if(param.toString() == "edit")
      _mode = cEdit;
    else if(param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
    }
  }

  param = pParams.value("alarm_id", &valid);
  if (valid)
  {
    _alarmid = param.toInt();
    sPopulate();
  }

}

void alarmMaint::sSave()
{
  XSqlQuery q;

  q.prepare("SELECT saveAlarm(:alarm_id, :alarm_number,"
            "                 :alarm_date, :alarm_time, :alarm_time_offset, :alarm_time_qualifier,"
            "                 :alarm_event, :alarm_event_recipient,"
            "                 :alarm_email, :alarm_email_recipient,"
            "                 :alarm_sysmsg, :alarm_sysmsg_recipient,"
            "                 :alarm_source, :alarm_source_id, 'CHANGEALL') AS result; ");

  q.bindValue(":alarm_event", QVariant(_eventAlarm->isChecked(), 0));
  q.bindValue(":alarm_email", QVariant(_emailAlarm->isChecked(), 0));
  q.bindValue(":alarm_sysmsg", QVariant(_sysmsgAlarm->isChecked(), 0));
  q.bindValue(":alarm_event_recipient", _eventRecipient->text());
  q.bindValue(":alarm_email_recipient", _emailRecipient->text());
  q.bindValue(":alarm_sysmsg_recipient", _sysmsgRecipient->text());
  q.bindValue(":alarm_date", _alarmDate->date());
  q.bindValue(":alarm_time", _alarmTime->time());
  q.bindValue(":alarm_time_offset", _alarmOffset->value());
  q.bindValue(":alarm_time_qualifier", _alarmQualifiers[_alarmQualifier->currentItem()]);
  q.bindValue(":alarm_source", Alarms::_alarmMap[_source].ident);
  q.bindValue(":alarm_source_id", _sourceid);
  q.bindValue(":alarm_id", _alarmid);
  q.exec();

  accept();
}

void alarmMaint::sHandleButtons()
{
  _contactCluster->hide();
}

void alarmMaint::sUserLookup(int pId)
{
  sGetUser(pId);
}

void alarmMaint::sContactLookup(int /*pId*/)
{
  QString recipient;
  recipient = _emailRecipient->text();
  if (recipient.length() == 0)
    recipient = _contactCluster->emailAddress();
  else if (recipient != _contactCluster->emailAddress())
  {
    recipient += ",";
    recipient += _contactCluster->emailAddress();
  }
  _emailRecipient->setText(recipient);
}

void alarmMaint::sGetUser(int pUsrId)
{
  XSqlQuery q;
  q.prepare( "SELECT usr.* "
             "FROM usr "
             "WHERE (usr_id=:usr_id);" );
  q.bindValue(":usr_id", pUsrId);
  q.exec();
  if (q.first())
  {
    bool recipientFound = false;
    QString recipient;
    recipient = _eventRecipient->text();
    for (int pcounter = 0; pcounter < 10; pcounter++)
    {
      if (recipient.section(",", pcounter, pcounter) == q.value("usr_username").toString())
        recipientFound = true;
    }
      
    if (recipient.length() == 0)
      recipient = q.value("usr_username").toString();
    else if (!recipientFound)
    {
      recipient += ",";
      recipient += q.value("usr_username").toString();
    }
    _eventRecipient->setText(recipient);
    _sysmsgRecipient->setText(recipient);

    recipientFound = false;
    recipient = _emailRecipient->text();
    for (int pcounter = 0; pcounter < 10; pcounter++)
    {
      if (recipient.section(",", pcounter, pcounter) == q.value("usr_email").toString())
        recipientFound = true;
    }
      
    if (recipient.length() == 0)
      recipient = q.value("usr_email").toString();
    else if (!recipientFound)
    {
      recipient += ",";
      recipient += q.value("usr_email").toString();
    }
    _emailRecipient->setText(recipient);
  }
}

void alarmMaint::sGetContact(int pCntctId)
{
  XSqlQuery q;
  q.prepare( "SELECT cntct.* "
             "FROM cntct "
             "WHERE (cntct_id=:cntct_id);" );
  q.bindValue(":cntct_id", pCntctId);
  q.exec();
  if (q.first())
  {
    QString recipient;
    recipient = _emailRecipient->text();
    if (recipient.length() == 0)
      recipient = q.value("cntct_email").toString();
    else if (recipient != q.value("cntct_email").toString())
    {
      recipient += ",";
      recipient += q.value("cntct_email").toString();
    }
    _emailRecipient->setText(recipient);
  }
}

void alarmMaint::sPopulate()
{
  XSqlQuery q;
  
  q.prepare( "SELECT * "
             "FROM alarm "
             "WHERE (alarm_id=:alarm_id);" );
  q.bindValue(":alarm_id", _alarmid);
  q.exec();
  if (q.first())
  {
    _alarmOffset->setValue(q.value("alarm_time_offset").toInt());
    for (int pcounter = 0; pcounter < _alarmQualifier->count(); pcounter++)
    {
      if (QString(q.value("alarm_time_qualifier").toString()) == _alarmQualifiers[pcounter])
        _alarmQualifier->setCurrentItem(pcounter);
    }
    _alarmDate->setDate(q.value("alarm_time").toDate());
    _alarmTime->setTime(q.value("alarm_time").toTime());
    _eventAlarm->setChecked(q.value("alarm_event").toBool());
    _emailAlarm->setChecked(q.value("alarm_email").toBool());
    _sysmsgAlarm->setChecked(q.value("alarm_sysmsg").toBool());
    _eventRecipient->setText(q.value("alarm_event_recipient").toString());
    _emailRecipient->setText(q.value("alarm_email_recipient").toString());
    _sysmsgRecipient->setText(q.value("alarm_sysmsg_recipient").toString());
    _source = (enum Alarms::AlarmSources)q.value("alarm_source").toInt();
    _sourceid = q.value("alarm_source_id").toInt();
  }
}

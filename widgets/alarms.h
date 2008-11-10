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

#ifndef alarms_h
#define alarms_h

#include <QWidget>
#include <QDateTime>

#include <xsqlquery.h>

#include "ui_alarms.h"

class OPENMFGWIDGETS_EXPORT Alarms : public QWidget, public Ui::alarms
{
  Q_OBJECT

  Q_ENUMS(AlarmSources)

  Q_PROPERTY(AlarmSources type READ type WRITE setType)

  friend class alarm;

  public:
    Alarms(QWidget *);

    // if you add to this then add to the _alarmMap[] below
    enum AlarmSources
    {
      Uninitialized,
      Address,          BBOMHead,           BBOMItem,
      BOMHead,          BOMItem,            BOOHead,
      BOOItem,          CRMAccount,         Contact, 
      Customer,         Employee,           Incident,   
      Item,             ItemSite,           ItemSource,
      Location,		LotSerial,          Opportunity,
      Project,		PurchaseOrder,      PurchaseOrderItem,
      ReturnAuth,       ReturnAuthItem,     Quote, 
      QuoteItem,        SalesOrder,         SalesOrderItem,
      TodoItem,
      TransferOrder,	TransferOrderItem,  Vendor,
      Warehouse,	WorkOrder
    };

    inline int  sourceid()          { return _sourceid; }
    inline enum AlarmSources type() { return _source;   }

    struct AlarmMap
    {
      enum AlarmSources source;
      QString           ident;

      AlarmMap(enum AlarmSources s, const QString & i)
      {
        source = s;
        ident = i;
      }
    };
    static const struct AlarmMap _alarmMap[]; // see Alarms.cpp for init

  public slots:
    void setType(enum AlarmSources);
    void setId(int);
    void setUsrId1(int);
    void setUsrId2(int);
    void setUsrId3(int);
    void setCntctId1(int);
    void setCntctId2(int);
    void setCntctId3(int);
    void setDate(QDate);
    void setReadOnly(bool);

    void sNew();
    void sEdit();
    void sView();
    void sDelete();
    
    void refresh();

  private:
    enum AlarmSources _source;
    int               _sourceid;
    int               _usrId1;
    int               _usrId2;
    int               _usrId3;
    int               _cntctId1;
    int               _cntctId2;
    int               _cntctId3;
    QDate             _dueDate;
    QTime             _dueTime;

};

#endif

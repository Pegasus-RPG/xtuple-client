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

#include "comments.h"
#include <QtScript>

void setupComments(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("Uninitialized",	 QScriptValue(engine, Comments::Uninitialized),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Address",	         QScriptValue(engine, Comments::Address),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMHead",	 QScriptValue(engine, Comments::BBOMHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BBOMItem",	 QScriptValue(engine, Comments::BBOMItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMHead",	         QScriptValue(engine, Comments::BOMHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOMItem",	         QScriptValue(engine, Comments::BOMItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOHead",	         QScriptValue(engine, Comments::BOOHead),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("BOOItem",	         QScriptValue(engine, Comments::BOOItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CRMAccount",	 QScriptValue(engine, Comments::CRMAccount),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Contact",	         QScriptValue(engine, Comments::Contact),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Customer",	 QScriptValue(engine, Comments::Customer),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Employee",	 QScriptValue(engine, Comments::Employee),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Incident",	 QScriptValue(engine, Comments::Incident),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Item",	         QScriptValue(engine, Comments::Item),	           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSite",	 QScriptValue(engine, Comments::ItemSite),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemSource",	 QScriptValue(engine, Comments::ItemSource),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Location",	 QScriptValue(engine, Comments::Location),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LotSerial",	 QScriptValue(engine, Comments::LotSerial),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Opportunity",	 QScriptValue(engine, Comments::Opportunity),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Project",	         QScriptValue(engine, Comments::Project),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrder",	 QScriptValue(engine, Comments::PurchaseOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PurchaseOrderItem",QScriptValue(engine, Comments::PurchaseOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuth",	 QScriptValue(engine, Comments::ReturnAuth),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReturnAuthItem",	 QScriptValue(engine, Comments::ReturnAuthItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Quote",	         QScriptValue(engine, Comments::Quote),	           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("QuoteItem",	 QScriptValue(engine, Comments::QuoteItem),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrder",	 QScriptValue(engine, Comments::SalesOrder),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesOrderItem",	 QScriptValue(engine, Comments::SalesOrderItem),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrder",	 QScriptValue(engine, Comments::TransferOrder),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TransferOrderItem",QScriptValue(engine, Comments::TransferOrderItem),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Vendor",	         QScriptValue(engine, Comments::Vendor),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Warehouse",	 QScriptValue(engine, Comments::Warehouse),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkOrder",	 QScriptValue(engine, Comments::WorkOrder),	   QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("Comments", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

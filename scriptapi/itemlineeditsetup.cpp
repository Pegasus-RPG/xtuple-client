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

#include "itemcluster.h"
#include <QtScript>

void setupItemLineEdit(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("cUndefined",	    QScriptValue(engine, ItemLineEdit::cUndefined),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPurchased",	    QScriptValue(engine, ItemLineEdit::cPurchased),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cManufactured",	    QScriptValue(engine, ItemLineEdit::cManufactured),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPhantom",	    QScriptValue(engine, ItemLineEdit::cPhantom),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cBreeder",	    QScriptValue(engine, ItemLineEdit::cBreeder),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cCoProduct",	    QScriptValue(engine, ItemLineEdit::cCoProduct),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cByProduct",	    QScriptValue(engine, ItemLineEdit::cByProduct),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cReference",	    QScriptValue(engine, ItemLineEdit::cReference),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cCosting",	    QScriptValue(engine, ItemLineEdit::cCosting),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cTooling",	    QScriptValue(engine, ItemLineEdit::cTooling),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cOutsideProcess",	    QScriptValue(engine, ItemLineEdit::cOutsideProcess),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanning",	    QScriptValue(engine, ItemLineEdit::cPlanning),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cJob",	            QScriptValue(engine, ItemLineEdit::cJob),	             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cKit",	            QScriptValue(engine, ItemLineEdit::cKit),	             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cAllItemTypes_Mask",  QScriptValue(engine, ItemLineEdit::cAllItemTypes_Mask),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningMRP",	    QScriptValue(engine, ItemLineEdit::cPlanningMRP),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningMPS",	    QScriptValue(engine, ItemLineEdit::cPlanningMPS),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningNone",	    QScriptValue(engine, ItemLineEdit::cPlanningNone),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cPlanningAny",	    QScriptValue(engine, ItemLineEdit::cPlanningAny),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cItemActive",	    QScriptValue(engine, ItemLineEdit::cItemActive),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cSold",	            QScriptValue(engine, ItemLineEdit::cSold),	             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cLocationControlled", QScriptValue(engine, ItemLineEdit::cLocationControlled), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cLotSerialControlled",QScriptValue(engine, ItemLineEdit::cLotSerialControlled),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cDefaultLocation",    QScriptValue(engine, ItemLineEdit::cDefaultLocation),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cActive",	            QScriptValue(engine, ItemLineEdit::cActive),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralManufactured",QScriptValue(engine, ItemLineEdit::cGeneralManufactured),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralPurchased",   QScriptValue(engine, ItemLineEdit::cGeneralPurchased),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralComponents",  QScriptValue(engine, ItemLineEdit::cGeneralComponents),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cGeneralInventory",   QScriptValue(engine, ItemLineEdit::cGeneralInventory),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("cKitComponents",	    QScriptValue(engine, ItemLineEdit::cKitComponents),	     QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("ItemLineEdit", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

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

#include "xcombobox.h"
#include <QtScript>

void setupXComboBox(QScriptEngine *engine)
{
  QScriptValue widget = engine->newObject();

  widget.setProperty("First",   QScriptValue(engine, XComboBox::First), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("None",    QScriptValue(engine, XComboBox::None),  QScriptValue::ReadOnly | QScriptValue::Undeletable);

  widget.setProperty("Adhoc",                QScriptValue(engine, XComboBox::Adhoc),                QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("APBankAccounts",       QScriptValue(engine, XComboBox::APBankAccounts),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("APTerms",              QScriptValue(engine, XComboBox::APTerms),              QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ARBankAccounts",       QScriptValue(engine, XComboBox::ARBankAccounts),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ARTerms",              QScriptValue(engine, XComboBox::ARTerms),              QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AccountingPeriods",    QScriptValue(engine, XComboBox::AccountingPeriods),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Agent",                QScriptValue(engine, XComboBox::Agent),                QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllCommentTypes",      QScriptValue(engine, XComboBox::AllCommentTypes),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("AllProjects",          QScriptValue(engine, XComboBox::AllProjects),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CRMAccounts",          QScriptValue(engine, XComboBox::CRMAccounts),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ClassCodes",           QScriptValue(engine, XComboBox::ClassCodes),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Companies",            QScriptValue(engine, XComboBox::Companies),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CostCategories",       QScriptValue(engine, XComboBox::CostCategories),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Countries",            QScriptValue(engine, XComboBox::Countries),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Currencies",           QScriptValue(engine, XComboBox::Currencies),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CurrenciesNotBase",    QScriptValue(engine, XComboBox::CurrenciesNotBase),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomerCommentTypes", QScriptValue(engine, XComboBox::CustomerCommentTypes), QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomerGroups",       QScriptValue(engine, XComboBox::CustomerGroups),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("CustomerTypes",        QScriptValue(engine, XComboBox::CustomerTypes),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ExpenseCategories",    QScriptValue(engine, XComboBox::ExpenseCategories),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FinancialLayouts",     QScriptValue(engine, XComboBox::FinancialLayouts),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FiscalYears",          QScriptValue(engine, XComboBox::FiscalYears),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("FreightClasses",       QScriptValue(engine, XComboBox::FreightClasses),       QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Honorifics",           QScriptValue(engine, XComboBox::Honorifics),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IncidentCategory",     QScriptValue(engine, XComboBox::IncidentCategory),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IncidentPriority",     QScriptValue(engine, XComboBox::IncidentPriority),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IncidentResolution",   QScriptValue(engine, XComboBox::IncidentResolution),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("IncidentSeverity",     QScriptValue(engine, XComboBox::IncidentSeverity),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemCommentTypes",     QScriptValue(engine, XComboBox::ItemCommentTypes),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ItemGroups",           QScriptValue(engine, XComboBox::ItemGroups),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Locales",              QScriptValue(engine, XComboBox::Locales),              QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LocaleCountries",      QScriptValue(engine, XComboBox::LocaleCountries),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LocaleLanguages",      QScriptValue(engine, XComboBox::LocaleLanguages),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("LotSerialCommentTypes",QScriptValue(engine, XComboBox::LotSerialCommentTypes),QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OpportunityStages",    QScriptValue(engine, XComboBox::OpportunityStages),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OpportunitySources",   QScriptValue(engine, XComboBox::OpportunitySources),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("OpportunityTypes",     QScriptValue(engine, XComboBox::OpportunityTypes),     QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PlannerCodes",         QScriptValue(engine, XComboBox::PlannerCodes),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("PoProjects",           QScriptValue(engine, XComboBox::PoProjects),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ProductCategories",    QScriptValue(engine, XComboBox::ProductCategories),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ProfitCenters",        QScriptValue(engine, XComboBox::ProfitCenters),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ProjectCommentTypes",  QScriptValue(engine, XComboBox::ProjectCommentTypes),  QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ReasonCodes",          QScriptValue(engine, XComboBox::ReasonCodes),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("RegistrationTypes",    QScriptValue(engine, XComboBox::RegistrationTypes),    QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Reports",              QScriptValue(engine, XComboBox::Reports),              QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesCategories",      QScriptValue(engine, XComboBox::SalesCategories),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesReps",            QScriptValue(engine, XComboBox::SalesReps),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SalesRepsActive",      QScriptValue(engine, XComboBox::SalesRepsActive),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ShipVias",             QScriptValue(engine, XComboBox::ShipVias),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ShippingCharges",      QScriptValue(engine, XComboBox::ShippingCharges),      QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("ShippingForms",        QScriptValue(engine, XComboBox::ShippingForms),        QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SiteTypes",            QScriptValue(engine, XComboBox::SiteTypes),            QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("SoProjects",           QScriptValue(engine, XComboBox::SoProjects),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Subaccounts",          QScriptValue(engine, XComboBox::Subaccounts),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TaxAuths",             QScriptValue(engine, XComboBox::TaxAuths),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TaxCodes",             QScriptValue(engine, XComboBox::TaxCodes),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("TaxTypes",             QScriptValue(engine, XComboBox::TaxTypes),             QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Terms",                QScriptValue(engine, XComboBox::Terms),                QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("UOMs",                 QScriptValue(engine, XComboBox::UOMs),                 QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("Users",                QScriptValue(engine, XComboBox::Users),                QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("VendorCommentTypes",   QScriptValue(engine, XComboBox::VendorCommentTypes),   QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("VendorGroups",         QScriptValue(engine, XComboBox::VendorGroups),         QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("VendorTypes",          QScriptValue(engine, XComboBox::VendorTypes),          QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WoProjects",           QScriptValue(engine, XComboBox::WoProjects),           QScriptValue::ReadOnly | QScriptValue::Undeletable);
  widget.setProperty("WorkCenters",          QScriptValue(engine, XComboBox::WorkCenters),          QScriptValue::ReadOnly | QScriptValue::Undeletable);

  engine->globalObject().setProperty("XComboBox", widget, QScriptValue::ReadOnly | QScriptValue::Undeletable);
}

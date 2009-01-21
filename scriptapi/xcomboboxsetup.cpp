/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
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

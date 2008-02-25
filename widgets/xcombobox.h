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

//  xcombobox.h
//  Created 03/16/2003 JSL
//  Copyright (c) 2003-2008, OpenMFG, LLC

#ifndef xcombobox_h
#define xcombobox_h

#include <QComboBox>
#include <QLabel>
#include <QMouseEvent>
#include <QList>

#include "OpenMFGWidgets.h"

class XSqlQuery;

class OPENMFGWIDGETS_EXPORT XComboBox : public QComboBox
{
  Q_OBJECT

  Q_ENUMS(XComboBoxTypes)

  Q_PROPERTY(bool      allowNull READ allowNull WRITE setAllowNull )
  Q_PROPERTY(QString     nullStr READ nullStr   WRITE setNullStr   )
  Q_PROPERTY(XComboBoxTypes type READ type      WRITE setType      )

  public:
    XComboBox(QWidget * = 0, const char * = 0);
    XComboBox(bool, QWidget * = 0, const char * = 0);

    enum XComboBoxTypes
      {
      Adhoc,
      APBankAccounts,	APTerms,		ARBankAccounts,
      ARTerms,		AccountingPeriods, 	Agent,
      AllCommentTypes,	AllProjects,		CRMAccounts,
      ClassCodes,	Companies,		CostCategories,
      Currencies,	CurrenciesNotBase,	CustomerCommentTypes,
      CustomerGroups,	CustomerTypes,		ExpenseCategories,
      FinancialLayouts,	FiscalYears,		Honorifics,
      IncidentCategory,
      IncidentPriority,	IncidentResolution,	IncidentSeverity,
      ItemCommentTypes,	ItemGroups,		LotSerialCommentTypes,
      OpportunityStages, OpportunitySources,    OpportunityTypes,
      PlannerCodes,	PoProjects,		ProductCategories,
      ProfitCenters,	ProjectCommentTypes,
      ReasonCodes,	Reports,
      SalesCategories,	SalesReps,		SalesRepsActive,
      ShipVias,		ShippingCharges,	ShippingForms,
      SoProjects,	Subaccounts,
      TaxAuths,		TaxCodes,
      TaxTypes,		Terms, 			UOMs,
      Users,		VendorCommentTypes,	VendorGroups,
      VendorTypes,	WoProjects,		WorkCenters
      };

    XComboBoxTypes type();
    void setType(XComboBoxTypes);

    void setText(QVariant &);
    void setText(const QString &);
    void setText(const QVariant &);

    virtual bool allowNull() const { return _allowNull; };
    virtual void setAllowNull(bool);
    virtual void setNull();

    QString nullStr() const { return _nullStr; };
    void setNullStr(const QString &);

    bool editable() const;
    void setEditable(bool);

    inline QLabel* label() const { return _label; };
    void setLabel(QLabel* pLab);


    int  id(int) const;
    int  id() const;
    bool isValid() const;

    QSize sizeHint() const;

  public slots:
    void setId(int);
    void populate(XSqlQuery &, int = -1);
    void populate(const char *, int = -1);
    void populate();
    void append(int, const QString &);
    void clear();

  private slots:
    void sHandleNewIndex(int);

  signals:
    void clicked();
    void newID(int);
    void valid(bool);
    void notNull(bool);

  protected:
    void mousePressEvent(QMouseEvent *);

    QList<int>          _ids;
    int                 _lastId;
    bool                _allowNull;
    enum XComboBoxTypes _type;
    QLabel*             _label;
    QString             _nullStr;
};

#endif


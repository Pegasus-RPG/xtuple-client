/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "replacePrices.h"

#include <QCloseEvent>
#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <metasql.h>
#include <parameter.h>
#include "mqlutil.h"
#include "guiclient.h"
#include "xdoublevalidator.h"

replacePrices::replacePrices(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);
    
    
    // signals and slots connections
    connect(_close,              SIGNAL(clicked()),     this, SLOT(close()));
    connect(_add,                SIGNAL(clicked()),     this, SLOT(sAdd()));
    connect(_addAll,             SIGNAL(clicked()),     this, SLOT(sAddAll()));
    connect(_remove,             SIGNAL(clicked()),     this, SLOT(sRemove()));
    connect(_removeAll,          SIGNAL(clicked()),     this, SLOT(sRemoveAll()));
    connect(_replace,            SIGNAL(clicked()),     this, SLOT(sReplace()));
    connect(_showEffective,      SIGNAL(clicked()),     this, SLOT(populate()));
    connect(_showExpired,        SIGNAL(clicked()),     this, SLOT(populate()));
    
    MetaSQLQuery mql = mqlLoad("replaceprices", "createselsched");
    ParameterList params;
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);

    _avail->addColumn(tr("Schedule"),      -1,          Qt::AlignLeft,  true,  "ipshead_name");
    _avail->addColumn(tr("Description"),   -1,          Qt::AlignLeft,  true,  "ipshead_descrip");
    _avail->addColumn(tr("Effective"),     -1,          Qt::AlignLeft,  true,  "ipshead_effective");
    _avail->addColumn(tr("Expires"),       -1,          Qt::AlignLeft,  true,  "ipshead_expires");

    _sel->addColumn(tr("Schedule"),        -1,          Qt::AlignLeft,  true,  "ipshead_name");
    _sel->addColumn(tr("Description"),     -1,          Qt::AlignLeft,  true,  "ipshead_descrip");
	
	populate();
}

replacePrices::~replacePrices()
{
    // no need to delete child widgets, Qt does it all for us
}

void replacePrices::languageChange()
{
    retranslateUi(this);
}

void replacePrices::closeEvent(QCloseEvent *pEvent)
{
    MetaSQLQuery mql = mqlLoad("replaceprices", "dropselsched");
    ParameterList params;
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
}

void replacePrices::sReplace()
{
    if (!_effective->isValid())
	{
      QMessageBox::critical( this, tr("Incomplete Data"),
                             tr("You must enter an Effective Date to continue.") );
      _effective->setFocus();
      return;
    }

    if (!_sel->topLevelItemCount())
	{
      QMessageBox::critical( this, tr("Incomplete Data"),
                             tr("You must select a Pricing Schedule to continue.") );
      return;
    }

    MetaSQLQuery mql = mqlLoad("replaceprices", "replace");
    ParameterList params;
    params.append("effective", _effective->date());

    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
	{
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	  return;
	}
	
    QMessageBox::information( this, tr("Success"),
                              tr("Replace Completed.") );
}

void replacePrices::populate()
{
    ParameterList params;
    if (_showEffective->isChecked())
      params.append("showEffective", true);
    if (_showExpired->isChecked())
      params.append("showExpired", true);

    MetaSQLQuery mql = mqlLoad("replaceprices", "availsched");
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _avail->populate(q);
	
    MetaSQLQuery mql2 = mqlLoad("replaceprices", "selsched");
    q = mql2.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    _sel->populate(q);
}

void replacePrices::sAdd()
{
    MetaSQLQuery mql = mqlLoad("replaceprices", "add");
    ParameterList params;
	params.append("ipshead_id", _avail->id());
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	populate();
}

void replacePrices::sAddAll()
{
    MetaSQLQuery mql = mqlLoad("replaceprices", "add");
    ParameterList params;
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	populate();
}

void replacePrices::sRemove()
{
    MetaSQLQuery mql = mqlLoad("replaceprices", "remove");
    ParameterList params;
	params.append("ipshead_id", _sel->id());
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	populate();
}

void replacePrices::sRemoveAll()
{
    MetaSQLQuery mql = mqlLoad("replaceprices", "remove");
    ParameterList params;
    q = mql.toQuery(params);
    if (q.lastError().type() != QSqlError::NoError)
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
	populate();
}


/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "shifts.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <openreports.h>
#include "shift.h"

/*
 *  Constructs a shifts as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
shifts::shifts(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_shiftList, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
shifts::~shifts()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void shifts::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>
#include <QSqlError>
void shifts::init()
{
//    statusBar()->hide();

    _shiftList->addColumn(tr("Shift Number"),	_userColumn,	Qt::AlignLeft, true, "shift_number" );
    _shiftList->addColumn(tr("Shift Name"),	-1,		Qt::AlignLeft, true, "shift_name" );

    if (_privileges->check("MaintainShifts"))
    {
	connect(_shiftList, SIGNAL(valid(bool)), _edit,	SLOT(setEnabled(bool)));
	connect(_shiftList, SIGNAL(valid(bool)), _delete,SLOT(setEnabled(bool)));
	connect(_shiftList, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }
    else
    {
	_new->setEnabled(false);
	_edit->setEnabled(false);
	_delete->setEnabled(false);
	connect(_shiftList, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
    }
    connect(_shiftList, SIGNAL(valid(bool)), _view,	SLOT(setEnabled(bool)));

    sFillList();
}

void shifts::sClose()
{
    close();
}


void shifts::sPrint()
{
    orReport report("ShiftsMasterList");
    if (report.isValid())
	report.print();
    else
	report.reportError(this);
}

void shifts::sNew()
{
    ParameterList params;
    params.append("mode", "new");

    shift newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void shifts::sEdit()
{
    ParameterList params;
    params.append("mode", "edit");
    params.append("shift_id", _shiftList->id());

    shift newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();	//if (newdlg.exec() != XDialog::Rejected)
	sFillList();
}

void shifts::sView()
{
    ParameterList params;
    params.append("mode", "view");
    params.append("shift_id", _shiftList->id());

    shift *newdlg = new shift(this, "", TRUE);
    newdlg->set(params);
    newdlg->show();
}

void shifts::sDelete()
{
    q.prepare("DELETE FROM shift WHERE shift_id = :shift_id;");
    q.bindValue(":shift_id", _shiftList->id());
    q.exec();
    if (q.lastError().type() != QSqlError::NoError)
	systemError(this, tr("A System Error occurred at %1::%2\n\n%3")
			    .arg(__FILE__)
			    .arg(__LINE__)
			    .arg(q.lastError().databaseText()));
    sFillList();
}

void shifts::sFillList()
{
    _shiftList->populate("SELECT shift_id, shift_number, shift_name "
			"FROM shift "
			"ORDER BY shift_number;");
}

void shifts::sPopulateMenu(QMenu *pMenu )
{
    int menuItem;

    menuItem = pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShifts"));

    menuItem = pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("ViewShifts") ||
				    _privileges->check("MaintainShifts"));

    menuItem = pMenu->insertItem(tr("Delete"), this, SLOT(sDelete()), 0);
    pMenu->setItemEnabled(menuItem, _privileges->check("MaintainShifts"));
}

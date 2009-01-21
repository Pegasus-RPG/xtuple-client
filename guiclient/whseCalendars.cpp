/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "whseCalendars.h"

#include <QVariant>
//#include <QStatusBar>
#include <QMessageBox>
#include <parameter.h>
#include <openreports.h>
#include "whseCalendar.h"

/*
 *  Constructs a whseCalendars as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
whseCalendars::whseCalendars(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

//  (void)statusBar();

  // signals and slots connections
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));

//  statusBar()->hide();
  
  if (_privileges->check("MaintainWarehouseCalendarExceptions"))
  {
    connect(_whsecal, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_whsecal, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_whsecal, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_whsecal, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
  }

  _whsecal->addColumn(tr("Site"),        70, Qt::AlignLeft,   true,  "code" );
  _whsecal->addColumn(tr("Description"), -1, Qt::AlignLeft,   true,  "whsecal_descrip" );

  sFillList(-1);
}

/*
 *  Destroys the object and frees any allocated resources
 */
whseCalendars::~whseCalendars()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void whseCalendars::languageChange()
{
  retranslateUi(this);
}

void whseCalendars::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  whseCalendar newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void whseCalendars::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("whsecal_id", _whsecal->id());

  whseCalendar newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void whseCalendars::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("whsecal_id", _whsecal->id());

  whseCalendar newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void whseCalendars::sDelete()
{
  q.prepare("DELETE FROM whsecal WHERE whsecal_id=:whsecal_id;");
  q.bindValue(":whsecal_id", _whsecal->id());
  q.exec();
  sFillList(-1);
}

void whseCalendars::sPrint()
{
  orReport report("WarehouseCalendarExceptionsMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void whseCalendars::sFillList(int pId)
{
  _whsecal->populate("SELECT whsecal_id,"
                     "       COALESCE((SELECT warehous_code FROM whsinfo WHERE warehous_id=whsecal_warehous_id) , 'Any') AS code,"
                     "       whsecal_descrip "
                     "  FROM whsecal "
                     " WHERE ( (whsecal_warehous_id IN (SELECT warehous_id FROM site())) OR (whsecal_warehous_id IS NULL) )"
                     " ORDER BY code,"
                     "          whsecal_effective,"
                     "          whsecal_expires;", pId  );
}


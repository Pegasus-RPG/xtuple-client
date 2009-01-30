/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "siteTypes.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <QWorkspace>
#include <openreports.h>
#include "siteType.h"

/*
 *  Constructs a siteTypes as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
siteTypes::siteTypes(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_sitetype, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    connect(_sitetype, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
siteTypes::~siteTypes()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void siteTypes::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void siteTypes::init()
{
//  statusBar()->hide();
  
  _sitetype->addColumn(tr("Code"),        _itemColumn, Qt::AlignCenter, true,  "sitetype_name" );
  _sitetype->addColumn(tr("Description"), -1,          Qt::AlignLeft,   true,  "sitetype_descrip"   );

  if (_privileges->check("MaintainSiteTypes"))
  {
    connect(_sitetype, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_sitetype, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_sitetype, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_sitetype, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  sFillList();
}

void siteTypes::sPrint()
{
  orReport report("SiteTypesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void siteTypes::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  siteType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void siteTypes::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sitetype_id", _sitetype->id());

  siteType newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void siteTypes::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sitetype_id", _sitetype->id());

  siteType newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void siteTypes::sDelete()
{
  if ( QMessageBox::information( this, tr("Delete Site Type"),
                                 tr("Are you sure that you want to delete the selected Site Type?"),
                                 tr("&Delete"), tr("&Cancel"), 0, 0, 1 ) == 0 )
  {
    q.prepare( "SELECT warehous_id "
               "FROM whsinfo "
               "WHERE (warehous_sitetype_id=:sitetype_id) "
               "LIMIT 1;" );
    q.bindValue(":sitetype_id", _sitetype->id());
    q.exec();
    if (q.first())
      QMessageBox::information( this, tr("Site Type in Use"),
                                tr( "The selected Site Type cannot be deleted as it still contains Sites.\n"
                                    "You must reassign these Sites before deleting this Site Type.") );

    else
    {
      q.prepare( "DELETE FROM sitetype "
                 "WHERE (sitetype_id=:sitetype_id);" );
      q.bindValue(":sitetype_id", _sitetype->id());
      q.exec();
      sFillList();
    }
  }
}

void siteTypes::sPopulateMenu(QMenu *menu)
{
  int menuItem;

  menuItem = menu->insertItem(tr("Edit Site Type..."), this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainSiteTypes"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("View Site Type..."), this, SLOT(sView()), 0);
  if ((!_privileges->check("MaintainSiteTypes")) && (!_privileges->check("ViewSiteTypes")))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem(tr("Delete Site Type..."), this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainSiteTypes"))
    menu->setItemEnabled(menuItem, FALSE);
}

void siteTypes::sFillList()
{
  _sitetype->populate( "SELECT sitetype_id, sitetype_name, sitetype_descrip "
                      "FROM sitetype "
                      "ORDER BY sitetype_name" );
}

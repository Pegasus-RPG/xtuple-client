/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projects.h"

#include <QVariant>
#include <QMessageBox>
//#include <QStatusBar>
#include <parameter.h>
#include <QWorkspace>
#include "project.h"

/*
 *  Constructs a projects as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
projects::projects(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_prj, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_prj, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
projects::~projects()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void projects::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>

void projects::init()
{
//  statusBar()->hide();
  
  if (_privileges->check("MaintainProjects"))
  {
    connect(_prj, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prj, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prj, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    _new->setEnabled(FALSE);
    connect(_prj, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
  }

  _prj->addColumn(tr("Number"), -1,           Qt::AlignCenter, true, "prj_number" );
  _prj->addColumn(tr("Name"),   _orderColumn, Qt::AlignLeft,   true, "prj_name"   );
  _prj->addColumn(tr("Status"), _itemColumn,  Qt::AlignCenter, true, "prj_status" );

  connect(omfgThis, SIGNAL(projectsUpdated(int)), this, SLOT(sFillList()));

  sFillList();
}

void projects::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sEdit()
{
  ParameterList params;
  params.append("prj_id", _prj->id());
  params.append("mode", "edit");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void projects::sView()
{
  ParameterList params;
  params.append("prj_id", _prj->id());
  params.append("mode", "view");

  project newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void projects::sDelete()
{
  q.prepare("SELECT deleteProject(:prj_id) AS result");
  q.bindValue(":prj_id", _prj->id());
  q.exec();
  if(q.first())
  {
    int result = q.value("result").toInt();
    if(result < 0)
    {
      QString errmsg;
      switch(result)
      {
        case -1:
          errmsg = tr("One or more Quote's refer to this project.");
          break;
        case -2:
          errmsg = tr("One or more Sales Orders refer to this project.");
          break;
        case -3:
          errmsg = tr("One or more Work Orders refer to this project.");
          break;
        case -4:
          errmsg = tr("One or more Purchase Requests refer to this project.");
          break;
        case -5:
          errmsg = tr("One or more Purchase order Items refer to this project.");
          break;
        case -6:
          errmsg = tr("One or more Invoices refer to this project.");
          break;
        default:
          errmsg = tr("Error #%1 encountered while trying to delete project.").arg(result);
      }
      QMessageBox::critical( this, tr("Cannot Delete Project"),
        tr("Could not delete the project for one or more reasons.\n") + errmsg);
      return;
    }
  }
  sFillList();
}

void projects::sPopulateMenu(QMenu *pMenu)
{
  int menuItem;

  pMenu->insertItem("View...", this, SLOT(sView()), 0);

  menuItem = pMenu->insertItem("Edit...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainProjects"))
    pMenu->setItemEnabled(menuItem, FALSE);

  menuItem = pMenu->insertItem("Delete...", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainProjects"))
    pMenu->setItemEnabled(menuItem, FALSE);
}

void projects::sFillList()
{
  q.prepare( "SELECT prj_id,"
             "       prj_number, prj_name,"
             "       CASE WHEN(prj_status='P') THEN :planning"
             "            WHEN(prj_status='O') THEN :open"
             "            WHEN(prj_status='C') THEN :complete"
             "            ELSE :undefined"
             "       END AS prj_status "
             "FROM prj "
             "ORDER BY prj_number;" );
  q.bindValue(":planning", tr("Concept"));
  q.bindValue(":open", tr("In-Process"));
  q.bindValue(":complete", tr("Complete"));
  q.bindValue(":undefined", tr("Undefined"));
  q.exec();
  _prj->populate(q);
}


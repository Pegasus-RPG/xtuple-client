/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QApplication>

#include <xsqlquery.h>
#include "projectcluster.h"
#include "projectCopy.h"

#define DEBUG true

ProjectCluster::ProjectCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new ProjectLineEdit(this, pName));
    _name->setVisible(true);
    setLabel(tr("Project #:"));
}

void ProjectCluster::setType(ProjectLineEdit::ProjectType ptype)
{
  return (static_cast<ProjectLineEdit*>(_number))->setType(ptype);
}

ProjectLineEdit::ProjectStatuses ProjectCluster::allowedStatuses() const
{
  return ((ProjectLineEdit*)_number)->allowedStatuses();
}

void ProjectCluster::setAllowedStatuses(const ProjectLineEdit::ProjectStatuses p)
{
  ((ProjectLineEdit*)_number)->setAllowedStatuses(p);
}

void ProjectCluster::sCopy()
{
  ((ProjectLineEdit*)_number)->sCopy();
}

void ProjectCluster::setAllowedStatuses(const int p)
{
  ((ProjectLineEdit*)_number)->setAllowedStatuses((ProjectLineEdit::ProjectStatuses)p);
}

ProjectLineEdit::ProjectType ProjectCluster::type()
{
  return (static_cast<ProjectLineEdit*>(_number))->type();
}

ProjectLineEdit::ProjectLineEdit(QWidget* pParent, const char* pName) :
    CrmClusterLineEdit(pParent, "prj", "prj_id", "prj_number", "prj_name", 0, "prj_owner_username", "prj_username", 0, pName)
{
  _objtype = "J";

  setTitles(tr("Project"), tr("Projects"));
  setUiName("project");
  setEditPriv("MaintainAllProjects");
  setNewPriv("MaintainAllProjects");
  setViewPriv("ViewAllProjects");
  setEditOwnPriv("MaintainPersonalProjects");
  setViewOwnPriv("ViewPersonalProjects");

  _type = Undefined;

  connect(_copyProject, SIGNAL(triggered()), this, SLOT(sCopy()));

  // Add copy Project menu item
  if (!_x_preferences->boolean("ClusterButtons"))
    menu()->insertAction(menu()->actions().at(6),_copyProject);
}

ProjectLineEdit::ProjectLineEdit(enum ProjectType pPrjType, QWidget *pParent, const char *pName) :
    CrmClusterLineEdit(pParent, "prj()", "prj_id", "prj_number", "prj_name", 0, 0, pName)
{
  setTitles(tr("Project"), tr("Projects"));
  setUiName("project");
  setEditPriv("MaintainAllProjects");
  setViewPriv("ViewAllProjects");

  _type = pPrjType;
  _allowedStatuses = 0x00;

}

void ProjectLineEdit::buildExtraClause()
{
  QString extraClause = _prjExtraClause;
  QString typeClause;
  QString statusClause;
  QStringList clauses;

  // Add in type clause
  if (_type & SalesOrder)    clauses << "(prj_so)";
  if (_type & WorkOrder)     clauses << "(prj_wo)";
  if (_type & PurchaseOrder) clauses << "(prj_po)";

  if (clauses.count())
    typeClause =  "(" + clauses.join(" OR ") + ")";

  if (!extraClause.isEmpty() &&
      !typeClause.isEmpty())
    extraClause.append(" AND ");

  if (!typeClause.isEmpty())
    extraClause.append(typeClause);

  // Add in status clause
  if (_allowedStatuses &&
      (_allowedStatuses != Concept + InProcess + Complete))
  {
    QStringList statusList;

    if (_allowedStatuses & Concept)	statusList << "'P'";
    if (_allowedStatuses & InProcess)	statusList << "'O'";
    if (_allowedStatuses & Complete)	statusList << "'C'";

    statusClause = "(prj_status IN (" +
                    statusList.join(", ") +
                    "))";
  }
  else
    statusClause = "";

  if (!extraClause.isEmpty() &&
      !statusClause.isEmpty())
    extraClause.append(" AND ");

  if (!statusClause.isEmpty())
    extraClause.append(statusClause);

  VirtualClusterLineEdit::setExtraClause(extraClause);
}

void ProjectLineEdit::setExtraClause(const QString& pExt)
{
  _prjExtraClause = pExt;
  buildExtraClause();
}

void ProjectLineEdit::setType(ProjectType ptype)
{
  _type = ptype;
  buildExtraClause();
}

void ProjectLineEdit::setAllowedStatuses(const ProjectStatuses p)
{
  _allowedStatuses = p;
  buildExtraClause();
}

void ProjectLineEdit::sCopy()
{
  if (DEBUG)
    qDebug("ProjectLineEdit::sCopy() Project ID: %d)", id());  

  if (id() == -1)
  {
    QMessageBox::information(this, tr("Project Copy"), tr("Please select a project to copy first"));
    return;
  }
  
  ParameterList params;
  params.append("prj_id", id());
  
  projectCopy newdlg(parentWidget(), "", TRUE);
  newdlg.set(params);
  
  int copiedProjectid;
  if ((copiedProjectid = newdlg.exec()) != QDialog::Rejected)
    setId(copiedProjectid);
}

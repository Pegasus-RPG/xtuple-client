/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "projectcluster.h"

ProjectCluster::ProjectCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    addNumberWidget(new ProjectLineEdit(this, pName));
}

ProjectLineEdit::ProjectType ProjectCluster::type()
{
  return (static_cast<ProjectLineEdit*>(_number))->type();
}

void ProjectCluster::setType(ProjectLineEdit::ProjectType ptype)
{
  return (static_cast<ProjectLineEdit*>(_number))->setType(ptype);
}

ProjectLineEdit::ProjectLineEdit(QWidget* pParent, const char* pName) :
    VirtualClusterLineEdit(pParent, "prj", "prj_id", "prj_number", "prj_name", 0, 0, pName)
{
  setTitles(tr("Project"), tr("Projects"));

  _type = Undefined;
}

ProjectLineEdit::ProjectLineEdit(enum ProjectType pPrjType, QWidget *pParent, const char *pName) :
    VirtualClusterLineEdit(pParent, "prj", "prj_id", "prj_number", "prj_name", 0, 0, pName)
{
  setTitles(tr("Project"), tr("Projects"));

  _type = pPrjType;
}

void ProjectLineEdit::setType(ProjectType ptype)
{
  QStringList clauses;
  if (ptype & SalesOrder)    clauses << "(prj_so)";
  if (ptype & WorkOrder)     clauses << "(prj_wo)";
  if (ptype & PurchaseOrder) clauses << "(prj_po)";

  VirtualClusterLineEdit::setExtraClause( "(" + clauses.join(" OR ") + ")");

  _type = ptype;
}

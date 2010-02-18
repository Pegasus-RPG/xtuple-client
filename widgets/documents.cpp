/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QDesktopServices>
#include <QMessageBox>
#include <QDialog>
#include <QUrl>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>
#include <metasql.h>
#include "mqlutil.h"

#include "documents.h"
#include "file.h"
#include "imageAssignment.h"
#include "docAttach.h"


// CAUTION: This will break if the order of this list does not match
//          the order of the enumerated values as defined.
const Documents::DocumentMap Documents::_documentMap[] =
{
  DocumentMap( Uninitialized,     " "   ),
  DocumentMap( Address,           "ADDR"),
  DocumentMap( BBOMHead,          "BBH" ),
  DocumentMap( BBOMItem,          "BBI" ),
  DocumentMap( BOMHead,           "BMH" ),
  DocumentMap( BOMItem,           "BMI" ),
  DocumentMap( BOOHead,           "BOH" ),
  DocumentMap( BOOItem,           "BOI" ),
  DocumentMap( CRMAccount,        "CRMA"),
  DocumentMap( Contact,           "T"   ),
  DocumentMap( Customer,          "C"   ),
  DocumentMap( Employee,          "EMP" ),
  DocumentMap( Incident,          "INCDT"),
  DocumentMap( Item,              "I"   ),
  DocumentMap( ItemSite,          "IS"  ),
  DocumentMap( ItemSource,        "IR"  ),
  DocumentMap( Location,          "L"   ),
  DocumentMap( LotSerial,         "LS"   ),
  DocumentMap( Opportunity,       "OPP" ),
  DocumentMap( Project,           "J"   ),
  DocumentMap( PurchaseOrder,     "P"   ),
  DocumentMap( PurchaseOrderItem, "PI"  ),
  DocumentMap( ReturnAuth,        "RA"  ),
  DocumentMap( ReturnAuthItem,    "RI"  ),
  DocumentMap( Quote,             "Q"   ),
  DocumentMap( QuoteItem,         "QI"  ),
  DocumentMap( SalesOrder,        "S"   ),
  DocumentMap( SalesOrderItem,    "SI"  ),
  DocumentMap( Todo,              "TODO"),
  DocumentMap( TransferOrder,     "TO"  ),
  DocumentMap( TransferOrderItem, "TI"  ),
  DocumentMap( Vendor,            "V"   ),
  DocumentMap( Warehouse,         "WH"  ),
  DocumentMap( WorkOrder,         "W"   ),
};

GuiClientInterface* Documents::_guiClientInterface = 0;
//xTupleGuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "type" );
  _doc->addColumn(tr("Id"), _itemColumn, Qt::AlignLeft, false, "target_id" );
  _doc->addColumn(tr("Number"), _itemColumn, Qt::AlignLeft, true, "targetnumber" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  -1, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  -1, Qt::AlignLeft,true, "doc_purpose");
  
  connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));
  connect(_doc, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(sEditDoc()));

  int menuItem;
  QMenu * newDocMenu = new QMenu;

  menuItem = newDocMenu->insertItem(tr("Image"),  this, SLOT(sNewImage()));
  menuItem = newDocMenu->insertItem(tr("File"),  this, SLOT(sNewFile()));
  menuItem = newDocMenu->insertItem(tr("Incident"), this, SLOT(sNewIncdt()));
  //if (!_privileges->check("MaintainIncidents"))
  //  newDocMenu->setItemEnabled(menuItem, FALSE);
  menuItem = newDocMenu->insertItem(tr("To-Do Item"),   this, SLOT(sNewToDo()));
  //if (!_privileges->check("MaintainPersonalTodoList") &&
  //    !_privileges->check("MaintainOtherTodoLists"))
  //  newDocMenu->setItemEnabled(menuItem, FALSE);
  //menuItem = newDocMenu->insertItem(tr("Task"),  this, SLOT(sNewTask()));
  menuItem = newDocMenu->insertItem(tr("Opportunity"),  this, SLOT(sNewOpp()));
  menuItem = newDocMenu->insertItem(tr("Project"),  this, SLOT(sNewProj()));

  _newDoc->setMenu(newDocMenu);

}

void Documents::setType(enum DocumentSources pSource)
{
  _source = pSource;
}

void Documents::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Documents::setReadOnly(bool pReadOnly)
{

  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _viewDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
  disconnect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  if(!pReadOnly)
  {
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
    connect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  }
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sNewFile()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Documents::sNewIncdt()
{
  ParameterList params;
  params.append("mode", "new");
  int target_id;
    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("incident", params, parentWidget(),Qt::WindowModal));
    target_id = newdlg->exec();
    if (target_id != QDialog::Rejected)
       sInsertDocass("INCDT", target_id);
    refresh();
}

void Documents::sNewToDo()
{
  if (_guiClientInterface)
  {
    ParameterList params;
    params.append("mode", "new");
    int target_id;
    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("todoItem", params, parentWidget(),Qt::WindowModal));
    target_id = newdlg->exec();
    if (target_id != QDialog::Rejected)
       sInsertDocass("TODO", target_id);
      refresh();
  }
}

void Documents::sNewProj()
{
    ParameterList params;
    params.append("mode", "new");
    int target_id;
    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("project", params, parentWidget(),Qt::WindowModal));
    target_id = newdlg->exec();
    if (target_id != QDialog::Rejected)
       sInsertDocass("J", target_id);
      refresh();
}

void Documents::sNewOpp()
{
    ParameterList params;
    params.append("mode", "new");
    int target_id;
    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("opportunity", params, parentWidget(),Qt::WindowModal));
    target_id = newdlg->exec();
    if (target_id != QDialog::Rejected)
       sInsertDocass("OPP", target_id);
      refresh();
}

void Documents::sInsertDocass(QString target_type, int target_id)
{

XSqlQuery q;
  q.prepare("INSERT INTO docass ( docass_source_id, docass_source_type, docass_target_id, docass_target_type )"
            "  VALUES ( :sourceid, :sourcetype::text, :targetid, :targettype); ");
  q.bindValue(":sourceid", _sourceid);
  q.bindValue(":sourcetype", _documentMap[_source].ident);
  q.bindValue(":targetid", target_id);
  q.bindValue(":targettype", target_type);
  q.exec();

}

void Documents::sEditDoc()
{
  QString docType;
  QString targetid;
  XTreeWidgetItem *pItem = _doc->currentItem();
  docType = pItem->rawValue("type").toString();
  //targetid = pItem->id("targetnumber");
  targetid = pItem->rawValue("target_id").toString();
  qDebug() << "this is the type:" << docType << "this is the id:" << targetid;

  //INCDT
  if (docType == "Incident")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("incdt_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("incident", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //URL -- In the future this should write to docass
  else if (docType == "URL")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("url_id", _doc->id());
    file newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    refresh();
  }

  //image -- In the future this needs to be changed to use docass instead of imageass
  else if (docType == "Image")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("imageass_id", _doc->id());
    imageAssignment newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
    refresh();
  }

  //todo
  else if (docType == "Todo")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("todoitem_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("todoItem", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }
  //opportunity
  else if (docType == "Opportunity")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("opphead_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("opportunity", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //project
  else if (docType == "Project")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("prj_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("project", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //crmacct
  else if (docType == "CRM Account")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("crmacct_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("crmaccount", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //cntct
  else if (docType == "Contact")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("cntct_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("contact", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //vendor
  else if (docType == "Vendor")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("vend_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("vendor", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //item
  else if (docType == "Item")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("item_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("item", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //project task
  else if (docType == "Task")
  {
    ParameterList params;
    params.append("mode", "edit");
    params.append("prjtask_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("task", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }
  else
      QMessageBox::critical(this, tr("Error"),
                          tr("That didn't work "));
}

void Documents::sViewDoc()
{
  QString docType;
  QString targetid;
  XTreeWidgetItem *pItem = _doc->currentItem();
  docType = pItem->rawValue("type").toString();
  targetid = _doc->currentItem()->id("targetnumber");
  //targetid = pItem->rawValue("target_id").toString();
  qDebug() << "this is the type:" << docType << "this is the id:" << targetid;

  //INCDT
  if (docType == "Incident")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("incdt_id", targetid);
		if (_guiClientInterface)
		{
			QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("incident", params, parentWidget(),Qt::WindowModal));
			newdlg->exec();

			refresh();
		}
  }

  //URL -- In the future this should write to docass
  else if (docType == "URL")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("url_id", _doc->id());
    file newdlg(this, "", TRUE);
    newdlg.set(params);
    newdlg.exec();
    refresh();
  }

  //image -- In the future this needs to be changed to use docass instead of imageass
  else if (docType == "Image")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("imageass_id", _doc->id());
    imageAssignment newdlg(this, "", TRUE);
    newdlg.set(params);

    if (newdlg.exec() != QDialog::Rejected)
    refresh();
  }

  //todo
  else if (docType == "Todo")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("todoitem_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("todoItem", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }
  //opportunity
  else if (docType == "Opportunity")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("opphead_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("opportunity", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //project
  else if (docType == "Project")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("prj_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("project", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //crmacct
  else if (docType == "CRM Account")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("crmacct_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("crmaccount", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //cntct
  else if (docType == "Contact")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("cntct_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("contact", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }

  //vendor
  else if (docType == "Vendor")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("vend_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("vendor", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //item
  else if (docType == "Item")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("item_id", targetid);

    QWidget* newdlg = _guiClientInterface->openDialog("item", params, parentWidget(),Qt::WindowModal, Qt::Dialog);
    newdlg->show();

    refresh();
  }

  //project task
  else if (docType == "Task")
  {
    ParameterList params;
    params.append("mode", "view");
    params.append("prjtask_id", targetid);

    QDialog* newdlg = qobject_cast<QDialog*>(_guiClientInterface->openDialog("task", params, parentWidget(),Qt::WindowModal));
    newdlg->exec();

    refresh();
  }
  else
      QMessageBox::critical(this, tr("Error"),
                          tr("That didn't work "));
	
}

void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  docAttach newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();

  refresh();
}

void Documents::sDetachDoc()
{
  XSqlQuery q;
  if ( 1 == _doc->altId() )
  {
  q.prepare( "DELETE FROM imageass "
             "WHERE (imageass_id = :docid );" );
  }
  else if ( 2 == _doc->altId() )
  {
  q.prepare( "DELETE FROM url "
             "WHERE (url_id = :docid );" );
  }
  else
  {
  q.prepare( "DELETE FROM docass "
             "WHERE (docass_id = :docid );" );
  }
  q.bindValue(":docid", _doc->id());
  q.exec();
  refresh();
}

void Documents::refresh()
{
  if(-1 == _sourceid)
  {
    _doc->clear();
   // _files->clear();
    return;
  }

  XSqlQuery query;
  
  //Populate doc list
  QString sql( "SELECT imageass_id AS id, 1,"
       " :number AS targetnumber,"
       " imageass_image_id AS target_id,"
       " :image AS type,"
       " image_name AS name, image_descrip AS description,"
       " CASE WHEN (imageass_purpose='I') THEN :inventory"
            " WHEN (imageass_purpose='P') THEN :product"
            " WHEN (imageass_purpose='E') THEN :engineering"
            " WHEN (imageass_purpose='M') THEN :misc"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM imageass, image"
               " WHERE (imageass_source=:source)"
               " AND (imageass_image_id=image_id)"
               " AND (imageass_source_id=:sourceid)"
" UNION"
" SELECT url_id AS id, 2,"
       " :number AS targetnumber,"
       " url_id AS target_id,"
       "  :url AS type,"
       "  url_title AS name, url_url AS description,"
       "  :sibling AS doc_purpose"
               " FROM url"
               " WHERE (url_source=:source)"
               " AND (url_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 3,"
       " incdt_number::text AS targetnumber,"
       " docass_target_id AS target_id,"
       " CASE WHEN (docass_target_type='IMG') THEN :image"
            " WHEN (docass_target_type='INCDT') THEN :incident"
            " WHEN (docass_target_type='TODO') THEN :todo"
            " WHEN (docass_target_type='TASK') THEN :task"
            " WHEN (docass_target_type='J') THEN :project"
            " WHEN (docass_target_type='I') THEN :item"
            " WHEN (docass_target_type='CRMA') THEN :crma"
            " WHEN (docass_target_type='C') THEN :cust"
            " WHEN (docass_target_type='V') THEN :vendor"
            " WHEN (docass_target_type='T') THEN :contact"
            " WHEN (docass_target_type='OPP') THEN :opp"
            " WHEN (docass_target_type='URL') THEN :url"
       " END AS type,"
       " incdt_summary AS name, firstline(incdt_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, incdt"
               " WHERE (docass_source_type=:source)"
               " AND (docass_target_type='INCDT')"
               " AND (docass_target_id=incdt_id)"
               " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 13,"
       " incdt_number::text AS targetnumber,"
       " docass_source_id AS target_id,"
       " :incident AS type,"
       " incdt_summary AS name, firstline(incdt_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, incdt"
               " WHERE (docass_source_type='INCDT')"
               " AND (docass_target_type=:source)"
               " AND (docass_target_id=:sourceid)"
               " AND (docass_source_id=incdt_id)"
" UNION"
" SELECT docass_id AS id, 4,"
       " :number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :todo AS type,"
       " todoitem_name AS name, firstline(todoitem_description) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, todoitem"
               " WHERE (docass_source_type=:source)"
               " AND (docass_target_type='TODO')"
               " AND (docass_target_id=todoitem_id)"
               " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 14,"
       " :number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :todo AS type,"
       " todoitem_name AS name, firstline(todoitem_description) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, todoitem"
               " WHERE (docass_source_type='TODD')"
               " AND (docass_target_type=:source)"
               " AND (docass_source_id=todoitem_id)"
               " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 5,"
       " prjtask_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :task AS type,"
       " prjtask_name AS name, firstline(prjtask_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, prjtask"
               " WHERE (docass_source_type=:source)"
               " AND (docass_target_type='TASK')"
               " AND (docass_target_id=prjtask_id)"
               " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 15,"
       " prjtask_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :task AS type,"
       " prjtask_name AS name, firstline(prjtask_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
               " FROM docass, prjtask"
               " WHERE (docass_target_type=:source)"
               " AND (docass_source_type='TASK')"
               " AND (docass_source_id=prjtask_id)"
               " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 6,"
       " prj_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :project AS type,"
       " prj_name AS name, firstline(prj_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, prj"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='J')"
            " AND (docass_target_id=prj_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 16,"
       " prj_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :project AS type,"
       " prj_name AS name, firstline(prj_descrip) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, prj"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='J')"
            " AND (docass_source_id=prj_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 7,"
       " item_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :item AS type,"
       " firstline(item_descrip1) AS name, firstline(item_descrip2) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, item"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='I')"
            " AND (docass_target_id=item_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 17,"
       " item_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :item AS type,"
       " firstline(item_descrip1) AS name, firstline(item_descrip2) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, item"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='I')"
            " AND (docass_source_id=item_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 8,"
       " crmacct_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :crma AS type,"
       " crmacct_name AS name,"
       " CASE WHEN (crmacct_type='O') THEN :organization"
            " WHEN (crmacct_type='I') THEN :individual"
            " ELSE :other"
            " END AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, crmacct"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='CRMA')"
            " AND (docass_target_id=crmacct_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 18,"
       " crmacct_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :crma AS type,"
       " crmacct_name AS name,"
       " CASE WHEN (crmacct_type='O') THEN :organization"
            " WHEN (crmacct_type='I') THEN :individual"
            " ELSE :other"
            " END AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, crmacct"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='CRMA')"
            " AND (docass_source_id=crmacct_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 9,"
       " cust_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :cust AS type,"
       " cust_name AS name, cust_address1 AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, cust"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='C')"
            " AND (docass_target_id=cust_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 19,"
       " cust_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :cust AS type,"
       " cust_name AS name, cust_address1 AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, cust"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='C')"
            " AND (docass_source_id=cust_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 10,"
       " vend_number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :vendor AS type,"
       " vend_name AS name, vendtype_code AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, vendinfo, vendtype"
            " WHERE (docass_source_type=:source)"
            " AND (vend_vendtype_id=vendtype_id)"
            " AND (docass_target_type='V')"
            " AND (docass_target_id=vend_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 20,"
       " vend_number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :vendor AS type,"
       " vend_name AS name, vendtype_code AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, vendinfo, vendtype"
            " WHERE (docass_target_type=:source)"
            " AND (vend_vendtype_id=vendtype_id)"
            " AND (docass_source_type='V')"
            " AND (docass_source_id=vend_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 11,"
       " :number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :contact AS type,"
       " cntct_first_name ||' '|| cntct_last_name AS name, cntct_email AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, cntct"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='T')"
            " AND (docass_target_id=cntct_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 21,"
       " :number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :contact AS type,"
       " cntct_first_name ||' '|| cntct_last_name AS name, cntct_email AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, cntct"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='T')"
            " AND (docass_source_id=cntct_id)"
            " AND (docass_target_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 12,"
       " :number AS targetnumber,"
       " docass_target_id AS target_id,"
       " :opp AS type,"
       " ophead_name AS name, firstline(ophead_notes) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :parent"
            " WHEN (docass_purpose='C') THEN :child"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, ophead"
            " WHERE (docass_source_type=:source)"
            " AND (docass_target_type='OPP')"
            " AND (docass_target_id=ophead_id)"
            " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 22,"
       " :number AS targetnumber,"
       " docass_source_id AS target_id,"
       " :opp AS type,"
       " ophead_name AS name, firstline(ophead_notes) AS description,"
       " CASE WHEN (docass_purpose='I') THEN :inventory"
            " WHEN (docass_purpose='P') THEN :product"
            " WHEN (docass_purpose='E') THEN :engineering"
            " WHEN (docass_purpose='M') THEN :misc"
            " WHEN (docass_purpose='A') THEN :child"
            " WHEN (docass_purpose='C') THEN :parent"
            " WHEN (docass_purpose='S') THEN :sibling"
            " WHEN (docass_purpose='D') THEN :dupe"
            " ELSE :other"
       " END AS doc_purpose"
            " FROM docass, ophead"
            " WHERE (docass_target_type=:source)"
            " AND (docass_source_type='OPP')"
            " AND (docass_source_id=ophead_id)"
            " AND (docass_target_id=:sourceid);"
            );
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));
  query.bindValue(":organization", tr("Organization"));
  query.bindValue(":number", tr("-"));
  query.bindValue(":individual", tr("Individual"));
  query.bindValue(":parent", tr("Parent"));
  query.bindValue(":child", tr("Child"));
  query.bindValue(":sibling", tr("Related to"));
  query.bindValue(":dupe", tr("Duplicate of"));
  query.bindValue(":other", tr("Other"));
  query.bindValue(":image", tr("Image"));
  query.bindValue(":incident", tr("Incident"));
  query.bindValue(":todo", tr("To-Do"));
  query.bindValue(":task", tr("Task"));
  query.bindValue(":project", tr("Project"));
  query.bindValue(":item", tr("Item"));
  query.bindValue(":crma", tr("CRM Account"));
  query.bindValue(":cust", tr("Customer"));
  query.bindValue(":vendor", tr("Vendor"));
  query.bindValue(":contact", tr("Contact"));
  query.bindValue(":opp", tr("Opportunity"));
  query.bindValue(":url", tr("URL"));
  query.bindValue(":sourceCust", _documentMap[Customer].ident);
  query.bindValue(":sourceContact", _documentMap[Contact].ident);
  query.bindValue(":sourceVend", _documentMap[Vendor].ident);
  query.bindValue(":source", _documentMap[_source].ident);
  query.bindValue(":sourceid", _sourceid);
  query.exec();
  _doc->populate(query,TRUE);
  
}




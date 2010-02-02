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
#include <QDialog>
#include <QUrl>
#include <QMenu>

#include <openreports.h>
#include <parameter.h>
#include <xsqlquery.h>

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
  DocumentMap( TransferOrder,     "TO"  ),
  DocumentMap( TransferOrderItem, "TI"  ),
  DocumentMap( Vendor,            "V"   ),
  DocumentMap( Warehouse,         "WH"  ),
  DocumentMap( WorkOrder,         "W"   ),
};

GuiClientInterface* Documents::_guiClientInterface = 0;

Documents::Documents(QWidget *pParent) :
  QWidget(pParent)
{
  setupUi(this);
  
  _source = Uninitialized;
  _sourceid = -1;

  _doc->addColumn(tr("Type"),  _itemColumn,  Qt::AlignLeft, true, "type" );
  _doc->addColumn(tr("Name"), -1,  Qt::AlignLeft, true, "name" );
  _doc->addColumn(tr("Description"),  _itemColumn*2, Qt::AlignLeft, true, "description");
  _doc->addColumn(tr("Relationship"),  -1, Qt::AlignLeft,true, "doc_purpose");
  
  connect(_openDoc, SIGNAL(clicked()), this, SLOT(sOpenDoc()));
  //connect(_attachDoc, SIGNAL(clicked()), this, SLOT(sAttachDoc()));
  connect(_editDoc, SIGNAL(clicked()), this, SLOT(sEditDoc()));
  connect(_viewDoc, SIGNAL(clicked()), this, SLOT(sViewDoc()));
  connect(_printDoc, SIGNAL(clicked()), this, SLOT(sPrintDoc()));
  connect(_detachDoc, SIGNAL(clicked()), this, SLOT(sDetachDoc()));

  int menuItem;
  QMenu * newDocMenu = new QMenu;

  menuItem = newDocMenu->insertItem(tr("Incident"), this, SLOT(sNewIncdt()));
  //if (!_privileges->check("MaintainIncidents"))
  //  newDocMenu->setItemEnabled(menuItem, FALSE);
  menuItem = newDocMenu->insertItem(tr("To-Do Item"),   this, SLOT(sNewToDo()));
  //if (!_privileges->check("MaintainPersonalTodoList") &&
  //    !_privileges->check("MaintainOtherTodoLists"))
  //  newDocMenu->setItemEnabled(menuItem, FALSE);
  menuItem = newDocMenu->insertItem(tr("Task"),  this, SLOT(sNewTask()));
  menuItem = newDocMenu->insertItem(tr("Opportunity"),  this, SLOT(sNewOpp()));
  menuItem = newDocMenu->insertItem(tr("Project"),  this, SLOT(sNewProj()));

  _newDoc->setMenu(newDocMenu);

  QMenu * attachDocMenu = new QMenu;

  menuItem = attachDocMenu->insertItem(tr("Image"),  this, SLOT(sNewImage()));
  menuItem = attachDocMenu->insertItem(tr("File"),  this, SLOT(sNewFile()));
  menuItem = attachDocMenu->insertItem(tr("Other"), this, SLOT(sAttachDoc()));
  //if (!_privileges->check("MaintainIncidents"))
  //  newDocMenu->setItemEnabled(menuItem, FALSE);

  _attachDoc->setMenu(attachDocMenu);
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
  _openDoc->setEnabled(!pReadOnly);
  _newDoc->setEnabled(!pReadOnly);
  _attachDoc->setEnabled(!pReadOnly);
  _editDoc->setEnabled(!pReadOnly);
  _detachDoc->setEnabled(!pReadOnly);

  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _openDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _viewDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
  disconnect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
  disconnect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  if(pReadOnly)
  {
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _openDoc, SLOT(animateClick()));
  }
  else
  {
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _openDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _editDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(doubleClicked(QModelIndex)), _detachDoc, SLOT(animateClick()));
    connect(_doc, SIGNAL(valid(bool)), _openDoc, SLOT(setEnabled(bool)));
    connect(_doc, SIGNAL(valid(bool)), _editDoc, SLOT(setEnabled(bool)));
    connect(_doc, SIGNAL(valid(bool)), _detachDoc, SLOT(setEnabled(bool)));
  }
}

void Documents::sNewImage()
{
  ParameterList params;
  params.append("mode", "new");
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
/*
    ParameterList params;
int id;
  params.append("mode", "new");

  incident newdlg(this, "", true);
  newdlg.set(params);
  id = newdlg.exec();
  if (id != XDialog::Rejected)
    // function to insert relationship into docass. Pass doc_type and doc_id
    sInsertDocass('INCDT', id);
    sFillList();
 */
}

/*void Documents::sInsertDocass(QString type, int id)
{
// this function will accept 2 arguments, a doc_type and a doc_id and it will insert a row into the docass table
// this function will be called by the New and Attach buttons
// this funtion will use the source type and id of the parent, and will default the doc_purpose to S (sibling)
} */

void Documents::sNewToDo()
{
  if (_guiClientInterface)
  {
    ParameterList params;
    params.append("mode", "new");

    QDialog* newdlg = _guiClientInterface->openDialog("todoItem", params, parentWidget(),Qt::WindowModal);

    if (newdlg->exec() != QDialog::Rejected)
      refresh();
  }
}
/*
void CRMAcctLineEdit::sInfo()
{
  if (_crmacctInfoAction)
    _crmacctInfoAction->crmacctInformation(this, id());
}
*/

void Documents::sNewTask()
{
}

void Documents::sNewOpp()
{
}

void Documents::sNewProj()
{
}

/*
void Documents::sEditFile()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("url_id", _files->id());

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Documents::sViewFile()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("url_id", _files->id());

  file newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Documents::sDeleteFile()
{
  XSqlQuery q;
  q.prepare("DELETE FROM docass"
            " WHERE (docass_id=:docass_id);" );
  q.bindValue(":url_id", _doc->id());
  q.exec();
  refresh();
}

void Documents::sOpenFile()
{
  XSqlQuery q;
  q.prepare("SELECT url_url"
            "  FROM url"
            " WHERE (url_id=:url_id); ");
  q.bindValue(":url_id", _files->id());
  q.exec();
  if(q.first())
  {
    //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(q.value("url_url").toString());
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
  }
}
*/

void Documents::sOpenDoc()
{/*
  //this function needs to figure out the document type, and then open the doc in the appropriate window

  XSqlQuery q;
  q.prepare("SELECT url_url"
            "  FROM url"
            " WHERE (url_id=:url_id); ");
  q.bindValue(":url_id", _doc->altId());
  q.exec();
  if(q.first())
  {
    //If url scheme is missing, we'll assume it is "file" for now.
    QUrl url(q.value("url_url").toString());
    if (url.scheme().isEmpty())
      url.setScheme("file");
    QDesktopServices::openUrl(url);
  }

  ParameterList params;
  params.append("mode", "view");
  params.append("target_id", _doc->targetid());

  imageview newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  */
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

/*
void Documents::sAttachDoc()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}
*/

void Documents::sEditDoc()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("imageass_id", _doc->id());

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
    refresh();
}

void Documents::sViewDoc()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("imageass_id", _doc->id());

  imageAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
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

void Documents::sPrintDoc()
{
  ParameterList params;
  params.append("image_id", _doc->altId());

  orReport report("Image", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
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
  QString sql( "SELECT imageass_id AS id, 1, imageass_image_id AS target_id,"
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
" SELECT url_id AS id, 2, url_id AS target_id,"
       "  :url AS type,"
       "  url_title AS name, url_url AS description,"
       "  :sibling AS doc_purpose"
               " FROM url"
               " WHERE (url_source=:source)"
               " AND (url_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 3, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 4, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 5, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 6, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 7, docass_target_id AS target_id,"
" CASE WHEN (docass_target_type='IMG') THEN 'Image'"
            " WHEN (docass_target_type='INCDT') THEN 'Incident'"
            " WHEN (docass_target_type='TODO') THEN 'To-Do'"
            " WHEN (docass_target_type='TASK') THEN 'Task'"
            " WHEN (docass_target_type='J') THEN 'Project'"
            " WHEN (docass_target_type='I') THEN 'Item'"
            " WHEN (docass_target_type='CRMA') THEN 'CRM Account'"
            " WHEN (docass_target_type='C') THEN 'Customer'"
            " WHEN (docass_target_type='V') THEN 'Vendor'"
            " WHEN (docass_target_type='T') THEN 'Contact'"
            " WHEN (docass_target_type='OPP') THEN 'Opportunity'"
            " WHEN (docass_target_type='URL') THEN 'URL'"
       " END AS type,"
       " item_number AS name, firstline(item_descrip1) AS description,"
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
" SELECT docass_id AS id, 8, docass_target_id AS target_id,"
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
       " crmacct_number AS name, crmacct_name AS description,"
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
" SELECT docass_id AS id, 9, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 10, docass_target_id AS target_id,"
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
       " vend_name AS name, vend_number AS description,"
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
               " FROM docass, vendinfo"
               " WHERE (docass_source_type=:source)"
               " AND (docass_target_type='V')"
               " AND (docass_target_id=vend_id)"
               " AND (docass_source_id=:sourceid)"
" UNION"
" SELECT docass_id AS id, 11, docass_target_id AS target_id,"
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
" SELECT docass_id AS id, 12, docass_target_id AS target_id,"
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
               " AND (docass_source_id=:sourceid);"
               );
  query.prepare(sql);
  query.bindValue(":inventory", tr("Inventory Description"));
  query.bindValue(":product", tr("Product Description"));
  query.bindValue(":engineering", tr("Engineering Reference"));
  query.bindValue(":misc", tr("Miscellaneous"));
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




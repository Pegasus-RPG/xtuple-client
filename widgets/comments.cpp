/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include <QHBoxLayout>
#include <QLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QList>
#include <QTextBrowser>
#include <QDesktopServices>
#include <QDebug>
#include <QScrollBar>

#include <parameter.h>
#include <xsqlquery.h>

#include "comment.h"
#include "comments.h"

void Comments::showEvent(QShowEvent *event)
{
  if (event)
  {
    QScrollBar * scrbar = _browser->verticalScrollBar();
    scrbar->setValue(0);
  }
}


// CAUTION: This will break if the order of this list does not match
//          the order of the enumerated values as defined.
const Comments::CommentMap Comments::_commentMap[] =
  {
    CommentMap( Uninitialized,     " "   ),
    CommentMap( Address,           "ADDR"),
    CommentMap( BBOMHead,          "BBH" ),
    CommentMap( BBOMItem,          "BBI" ),
    CommentMap( BOMHead,           "BMH" ),
    CommentMap( BOMItem,           "BMI" ),
    CommentMap( BOOHead,           "BOH" ),
    CommentMap( BOOItem,           "BOI" ),
    CommentMap( CRMAccount,        "CRMA"),
    CommentMap( Contact,           "T"   ),
    CommentMap( Customer,          "C"   ),
    CommentMap( Employee,          "EMP" ),
    CommentMap( Incident,          "INCDT"),
    CommentMap( Item,              "I"   ),
    CommentMap( ItemSite,          "IS"  ),
    CommentMap( ItemSource,        "IR"  ),
    CommentMap( Location,          "L"   ),
    CommentMap( LotSerial,         "LS"   ),
    CommentMap( Opportunity,       "OPP" ),
    CommentMap( Project,           "J"   ),
    CommentMap( PurchaseOrder,     "P"   ),
    CommentMap( PurchaseOrderItem, "PI"  ),
    CommentMap( ReturnAuth,        "RA"  ),
    CommentMap( ReturnAuthItem,    "RI"  ),
    CommentMap( Quote,             "Q"   ),
    CommentMap( QuoteItem,         "QI"  ),
    CommentMap( SalesOrder,        "S"   ),
    CommentMap( SalesOrderItem,    "SI"  ),
    CommentMap( Task,              "TA"  ),
    CommentMap( TodoItem,          "TD"  ),
    CommentMap( TransferOrder,     "TO"  ),
    CommentMap( TransferOrderItem, "TI"  ),
    CommentMap( Vendor,            "V"   ),
    CommentMap( Warehouse,         "WH"  ),
    CommentMap( WorkOrder,         "W"   ),
  };

Comments::Comments(QWidget *pParent, const char *name) :
  QWidget(pParent, name)
{
  _source = Uninitialized;
  _sourceid = -1;

  _verboseCommentList = false;
  if(_x_metrics)
    _verboseCommentList = _x_metrics->boolean("VerboseCommentList");

  QHBoxLayout *main = new QHBoxLayout(this);
  main->setMargin(0);
  main->setSpacing(7);

  QWidget *buttons = new QWidget(this);
  QVBoxLayout * buttonsLayout = new QVBoxLayout(buttons);
  buttonsLayout->setMargin(0);
  buttonsLayout->setSpacing(0);

  _comment = new XTreeWidget(this);
  _comment->setObjectName("_comment");
  _comment->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _comment->addColumn(tr("Date/Time"), _timeDateColumn, Qt::AlignCenter,true, "comment_date");
  _comment->addColumn(tr("Type"),    _itemColumn, Qt::AlignCenter,true, "type");
  _comment->addColumn(tr("User"),    _userColumn, Qt::AlignCenter,true, "comment_user");
  _comment->addColumn(tr("Comment"), -1,          Qt::AlignLeft,  true, "first");
  main->addWidget(_comment);

  _browser = new QTextBrowser(this);
  _browser->setObjectName("_browser");
  _browser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _browser->setOpenLinks(false);
  main->addWidget(_browser);

  _newComment = new QPushButton(tr("New"), buttons, "_newComment");
  buttonsLayout->addWidget(_newComment);

  _viewComment = new QPushButton(tr("View"), buttons, "_viewComment");
  _viewComment->setEnabled(FALSE);
  buttonsLayout->addWidget(_viewComment);

  _editComment = new QPushButton(tr("Edit"), buttons, "_editComment");
  _editComment->setEnabled(false);
  buttonsLayout->addWidget(_editComment);

  QSpacerItem *_buttonSpacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  buttonsLayout->addItem(_buttonSpacer);
  buttons->setLayout(buttonsLayout);
  main->addWidget(buttons);
  
  _editmap = new QMultiMap<int, bool>();

  setLayout(main);

  connect(_newComment, SIGNAL(clicked()), this, SLOT( sNew()));
  connect(_viewComment, SIGNAL(clicked()), this, SLOT( sView()));
  connect(_editComment, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_comment, SIGNAL(valid(bool)), this, SLOT(sCheckButtonPriv(bool)));
  connect(_comment, SIGNAL(itemSelected(int)), _viewComment, SLOT(animateClick()));
  connect(_browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(anchorClicked(QUrl)));

  setFocusProxy(_comment);

  setVerboseCommentList(_verboseCommentList);
}

void Comments::setType(enum CommentSources pSource)
{
  _source = pSource;
}

void Comments::setId(int pSourceid)
{
  _sourceid = pSourceid;
  refresh();
}

void Comments::setReadOnly(bool pReadOnly)
{
  if (pReadOnly)
    _newComment->setEnabled(FALSE);
  else
    _newComment->setEnabled(TRUE);
}

void Comments::sNew()
{ 
  ParameterList params;
  params.append("mode", "new");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);

  comment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != QDialog::Rejected)
  {
    emit commentAdded();
    refresh();
  }
}

void Comments::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);
  params.append("comment_id", _comment->id());
  params.append("commentIDList", _commentIDList);

  comment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void Comments::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("sourceType", _source);
  params.append("source_id", _sourceid);
  params.append("comment_id", _comment->id());
  params.append("commentIDList", _commentIDList);

  comment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  refresh();
}

void Comments::refresh()
{
  _browser->document()->clear();
  _editmap->clear();
  if(-1 == _sourceid)
  {
    _comment->clear();
    return;
  }

  XSqlQuery comment;
  if(_source != CRMAccount)
  {
    comment.prepare( "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first,"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_user=CURRENT_USER AS self "
                     "FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     "WHERE ( (comment_source=:source)"
                     " AND (comment_source_id=:sourceid) ) "
                     "ORDER BY comment_date DESC;" );
  }
  else
  {
    // If it's CRMAccount we want to do some extra joining in our SQL
    comment.prepare( "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END AS type,"
                     "       comment_user,"
                     "       firstLine(detag(comment_text)) AS first,"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_user=CURRENT_USER AS self "
                     "  FROM comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:source)"
                     "   AND (comment_source_id=:sourceid) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_user=CURRENT_USER AS self "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceCust)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_cust_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_user=CURRENT_USER AS self "
                     "  FROM crmacct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceVend)"
                     "   AND (crmacct_id=:sourceid)"
                     "   AND (comment_source_id=crmacct_vend_id) ) "
                     " UNION "
                     "SELECT comment_id, comment_date,"
                     "       CASE WHEN (cmnttype_name IS NOT NULL) THEN cmnttype_name"
                     "            ELSE :none"
                     "       END,"
                     "       comment_user, firstLine(detag(comment_text)),"
                     "       comment_text, "
                     "       COALESCE(cmnttype_editable,false) AS editable, "
                     "       comment_user=CURRENT_USER AS self "
                     "  FROM cntct, comment LEFT OUTER JOIN cmnttype ON (comment_cmnttype_id=cmnttype_id) "
                     " WHERE((comment_source=:sourceContact)"
                     "   AND (cntct_crmacct_id=:sourceid)"
                     "   AND (comment_source_id=cntct_id) ) "
                     "ORDER BY comment_date DESC;" );
    comment.bindValue(":sourceCust", _commentMap[Customer].ident);
    comment.bindValue(":sourceContact", _commentMap[Contact].ident);
    comment.bindValue(":sourceVend", _commentMap[Vendor].ident);
  }
  comment.bindValue(":none", tr("None"));
  comment.bindValue(":source", _commentMap[_source].ident);
  comment.bindValue(":sourceid", _sourceid);
  comment.exec();

  QString lclHtml = "<body>";
  QRegExp br("\r?\n");
  _commentIDList.clear();
  while(comment.next())
  {
    _editmap->insert(comment.value("comment_id").toInt(),comment.value("editable").toBool());
    _editmap->insert(comment.value("comment_id").toInt(),comment.value("self").toBool());
    
    int cid = comment.value("comment_id").toInt();
    _commentIDList.push_back(cid);
    lclHtml += comment.value("comment_date").toDateTime().toString();
    lclHtml += " ";
    lclHtml += comment.value("type").toString();
    lclHtml += " ";
    lclHtml += comment.value("comment_user").toString();
    if(userCanEdit(cid))
    {
      lclHtml += " <a href=\"edit?id=";
      lclHtml += QString::number(cid);
      lclHtml += "\">edit</a>";
    }
    lclHtml += "<p>\n<blockquote>";
    lclHtml += comment.value("comment_text").toString().replace(br,"<br>\n");
    lclHtml += "</pre></blockquote>\n<hr>\n";
  }
  lclHtml += "</body>";

  _browser->document()->setHtml(lclHtml);
  comment.first();
  _comment->populate(comment);
}

void Comments::setVerboseCommentList(bool vcl)
{
  _verboseCommentList = vcl;
  _comment->setVisible(!vcl);
  _viewComment->setVisible(!vcl);
  _editComment->setVisible(!vcl);
  _browser->setVisible(vcl);
}

void Comments::anchorClicked(const QUrl & url)
{
  if(url.host().isEmpty() && url.path() == "edit")
  {
    int cid = url.queryItemValue("id").toInt();
    if(userCanEdit(cid))
    {
      ParameterList params;
      params.append("mode", "edit");
      params.append("sourceType", _source);
      params.append("source_id", _sourceid);
      params.append("comment_id", cid);
      params.append("commentIDList", _commentIDList);

      comment newdlg(this, "", TRUE);
      newdlg.set(params);
      newdlg.exec();
      refresh();
    }
  }
  else
  {
    QDesktopServices::openUrl(url);
  }
}

void Comments::sCheckButtonPriv(bool pValid)
{
  _viewComment->setEnabled(pValid);
  if(pValid)
  {
    _editComment->setEnabled(userCanEdit(_comment->id()));
  }
  else
    _editComment->setEnabled(false);
}

bool Comments::userCanEdit(int id)
{
  QList<bool> values = _editmap->values(id);
  
  if(values.at(0))
  {
    if(_x_privileges && _x_privileges->check("EditOthersComments"))
      return true;
    if(_x_privileges && _x_privileges->check("EditOwnComments") && values.at(1))
      return true;
  }
  return false;
}

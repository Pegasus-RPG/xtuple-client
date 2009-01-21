/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "boo.h"

#include <QMenu>
#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <metasql.h>
#include <openreports.h>

#include "booItem.h"

boo::boo(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_booitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire()));

  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased | ItemLineEdit::cPlanning | ItemLineEdit::cJob);

  _booitem->addColumn(tr("#"),           _seqColumn,  Qt::AlignCenter,true, "booitem_seqnumber");
  _booitem->addColumn(tr("Std Oper."),   _itemColumn, Qt::AlignLeft,  true, "f_stdopnnumber");
  _booitem->addColumn(tr("Work Cntr."),  _itemColumn, Qt::AlignLeft,  true, "wrkcnt_code");
  _booitem->addColumn(tr("Description"), -1,          Qt::AlignLeft,  true, "description");
  _booitem->addColumn(tr("Effective"),   _dateColumn, Qt::AlignCenter,true, "booitem_effective");
  _booitem->addColumn(tr("Expires"),     _dateColumn, Qt::AlignCenter,true, "booitem_expires");
  _booitem->addColumn(tr("Exec. Day"),   _qtyColumn,  Qt::AlignCenter,true, "booitem_execday");
  
  connect(omfgThis, SIGNAL(boosUpdated(int, bool)), this, SLOT(sFillList()));

  _activate->hide();
  _revision->setMode(RevisionLineEdit::Maintain);
  _revision->setType("BOO");
}

/*
 *  Destroys the object and frees any allocated resources
 */
boo::~boo()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void boo::languageChange()
{
  retranslateUi(this);
}

enum SetResponse boo::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("item_id", &valid);
  if (valid)
    _item->setId(param.toInt());
   {
     param = pParams.value("revision_id", &valid);
     if (valid)
       _revision->setId(param.toInt());
   }

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if ( (param.toString() == "new") || (param.toString() == "edit") )
    {
      connect(_booitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
      connect(_booitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    }

    if (param.toString() == "new")
    {
      _mode = cNew;

      _item->setFocus();
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(TRUE);

      _save->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;

      _item->setReadOnly(TRUE);
      _documentNum->setEnabled(FALSE);
      _revision->setEnabled(FALSE);
      _revisionDate->setEnabled(FALSE);
      _finalLocation->setEnabled(FALSE);
      _closeWO->setEnabled(FALSE);
      _new->setEnabled(FALSE);
      _edit->setEnabled(FALSE);
      _expire->setEnabled(FALSE);
      _moveUp->setEnabled(FALSE);
      _moveDown->setEnabled(FALSE);
      _save->setEnabled(FALSE);

      connect(_booitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

      _close->setFocus();
    }
  }

  return NoError;
}

void boo::sSave()
{
  bool childhasblankloc = false;
  q.prepare("SELECT BOOL_OR(COALESCE(booitem_wip_location_id, -1) = -1) AS blankloc "
            "FROM booitem "
            "WHERE ((booitem_item_id=:item_id)"
            "   AND (booitem_rev_id=:rev_id));");
  q.bindValue(":item_id", _item->id());
  q.bindValue(":rev_id",  _revision->id());
  q.exec();
  if (q.first())
    childhasblankloc = q.value("blankloc").toBool();
  else if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
  if (_finalLocation->id() <= -1 || childhasblankloc)
  {
    q.prepare("SELECT EXISTS(SELECT itemsite_id"
              "              FROM itemsite"
              "              WHERE (itemsite_loccntrl"
              "                 AND itemsite_disallowblankwip"
              "                 AND itemsite_active"
              "                 AND (itemsite_item_id=:item_id))"
              "       ) AS disallowblankwip;");
    q.bindValue(":item_id", _item->id());
    q.exec();
    if (q.first())
    {
      if (q.value("disallowblankwip").toBool() &&
          QMessageBox::question(this, tr("Save anyway?"),
                                (childhasblankloc ?
                                  tr("<p>At least one Bill Of Operations Item "
                                     "has no WIP Location.") :
                                  tr("<p>This Bill Of Operations has no Final "
                                     "Location.")) +
                                  tr(" However, at least one Item Site for "
                                     "this Item requires WIP Locations. Are "
                                     "you sure you want to save this Bill Of "
                                     "Operations?<p>If you say 'Yes' then you "
                                     "should fix the Item Site."),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::No)
          return;
    }
    else if (q.lastError().type() != QSqlError::NoError)
    {
      systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
      return;
    }
  }

  if (_mode == cEdit )
  {
	q.prepare( "UPDATE boohead "
               "SET boohead_docnum=:boohead_docnum, boohead_leadtime=:boohead_leadtime,"
               "    boohead_revision=:boohead_revision, boohead_revisiondate=:boohead_revisiondate,"
               "    boohead_final_location_id=:boohead_final_location_id,"
               "    boohead_closewo=:boohead_closewo "
               "WHERE ((boohead_item_id=:boohead_item_id) "
			   "AND (boohead_rev_id=:boohead_rev_id));" );
    q.bindValue(":boohead_rev_id", _revision->id());
  }
  else
  {
    q.prepare( "INSERT INTO boohead "
               "(boohead_item_id, boohead_docnum, boohead_leadtime, boohead_closewo,"
               " boohead_revision, boohead_revisiondate, boohead_final_location_id) "
               "VALUES "
               "( :boohead_item_id, :boohead_docnum, :boohead_leadtime, :boohead_closewo,"
               "  :boohead_revision, :boohead_revisiondate, :boohead_final_location_id );" );
  }

  q.bindValue(":boohead_item_id", _item->id());
  q.bindValue(":boohead_docnum", _documentNum->text());
  q.bindValue(":boohead_leadtime", _productionLeadTime->text().toInt());
  q.bindValue(":boohead_revision", _revision->number());
  q.bindValue(":boohead_revisiondate", _revisionDate->date());
  q.bindValue(":boohead_final_location_id", _finalLocation->id());
  q.bindValue(":boohead_closewo", QVariant(_closeWO->isChecked()));
  q.exec();

  close();
}

bool boo::setParams(ParameterList &pparams)
{
  pparams.append("item_id",     _item->id());
  pparams.append("revision_id", _revision->id());
  pparams.append("none",        tr("None"));

  if (_showExpired->isChecked())
    pparams.append("showExpired");
  if (_showFuture->isChecked())
    pparams.append("showFuture");

  return true;
}

void boo::sPrint()
{
  ParameterList params;
  if (! setParams(params))
    return;

  orReport report("BillOfOperations", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void boo::sPopulateMenu(QMenu *pMenu)
{
  pMenu->insertItem(tr("View"), this, SLOT(sView()), 0);

  if ( ((_mode == cNew) || (_mode == cEdit)) &&
       (_privileges->check("MaintainBOOs")) )
  {
    pMenu->insertItem(tr("Edit"), this, SLOT(sEdit()), 0);
    pMenu->insertItem(tr("Expire"), this, SLOT(sExpire()), 0);

    pMenu->insertSeparator();

    pMenu->insertItem(tr("Move Up"),   this, SLOT(sMoveUp()), 0);
    pMenu->insertItem(tr("Move Down"), this, SLOT(sMoveDown()), 0);
  }
}

void boo::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sEdit()
{
  if (!checkSitePrivs(_booheadid))
    return;
    
  ParameterList params;
  params.append("mode", "edit");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sView()
{
  if (!checkSitePrivs(_booheadid))
    return;
    
  ParameterList params;
  params.append("mode", "view");
  params.append("booitem_id", _booitem->id());

  booItem newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void boo::sExpire()
{
  if (!checkSitePrivs(_booheadid))
    return;
    
  q.prepare( "UPDATE booitem "
             "SET booitem_expires=CURRENT_DATE "
             "WHERE (booitem_id=:booitem_id);" );
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_item->id(), TRUE);
}

void boo::sMoveUp()
{
  if (!checkSitePrivs(_booheadid))
    return;
    
  q.prepare("SELECT moveBooitemUp(:booitem_id) AS result;");
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_booitem->id(), TRUE);
}

void boo::sMoveDown()
{
  if (!checkSitePrivs(_booheadid))
    return;
    
  q.prepare("SELECT moveBooitemDown(:booitem_id) AS result;");
  q.bindValue(":booitem_id", _booitem->id());
  q.exec();

  omfgThis->sBOOsUpdated(_booitem->id(), TRUE);
}

void boo::sFillList()
{
  if (_item->itemType() == "J")
  {
    _closeWO->setEnabled(FALSE);
    _closeWO->setChecked(FALSE);
  }

  int locid = _finalLocation->id();
  q.prepare("SELECT location_id, (warehous_code || '-' || formatLocationName(location_id)) AS locationname"
            "  FROM location, warehous"
            " WHERE ( (NOT location_restrict)"
            "   AND   (location_warehous_id=warehous_id) ) "
            "UNION "
            "SELECT location_id, (warehous_code || '-' || formatLocationName(location_id)) AS locationname"
            "  FROM location, warehous, locitem"
            " WHERE ( (location_warehous_id=warehous_id)"
            "   AND   (location_restrict)"
            "   AND   (locitem_location_id=location_id)"
            "   AND   (locitem_item_id=:item_id) ) "
            "ORDER BY locationname;");
  q.bindValue(":item_id", _item->id());
  q.exec();
  _finalLocation->populate(q, locid);

  q.prepare( "SELECT boohead_id, boohead_docnum, boohead_revision,"
             "       boohead_revisiondate, boohead_final_location_id,"
             "       boohead_closewo "
             "FROM boohead "
             "WHERE ((boohead_item_id=:item_id) "
             "AND (boohead_rev_id=:revision_id));" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":revision_id", _revision->id());
  q.exec();
  if (q.first())
  {
    _booheadid = q.value("boohead_id").toInt();
    _documentNum->setText(q.value("boohead_docnum").toString());
    _revision->setNumber(q.value("boohead_revision").toString());
    _revisionDate->setDate(q.value("boohead_revisiondate").toDate());
    _finalLocation->setId(q.value("boohead_final_location_id").toInt());
    _closeWO->setChecked(q.value("boohead_closewo").toBool());
  }

  if (_revision->description() == "Inactive")
  {
	  _save->setEnabled(FALSE);
          _new->setEnabled(FALSE);
	  _documentNum->setEnabled(FALSE);
	  _revisionDate->setEnabled(FALSE);
	  _closeWO->setEnabled(FALSE);
	  _finalLocation->setEnabled(FALSE);
	  _booitem->setEnabled(FALSE);
  }

  if ((_revision->description() == "Pending") || (_revision->description() == "Active"))
  {
	  _save->setEnabled(TRUE);
          _new->setEnabled(TRUE);
	  _documentNum->setEnabled(TRUE);
	  _revisionDate->setEnabled(TRUE);
	  _closeWO->setEnabled(TRUE);
	  _finalLocation->setEnabled(TRUE);
	  _booitem->setEnabled(TRUE);
  }

  q.prepare( "SELECT MAX(booitem_execday) AS leadtime "
             "FROM booitem(:item_id,:revision_id);" );
  q.bindValue(":item_id", _item->id());
  q.bindValue(":revision_id",_revision->id());
  q.exec();
  if (q.first())
    _productionLeadTime->setText(q.value("leadtime").toString());

  QString sql( "SELECT booitem_id, booitem_seqnumber,"
               "       COALESCE(stdopn_number, <? value(\"none\")?> ) AS f_stdopnnumber,"
               "       wrkcnt_code, (booitem_descrip1 || ' ' || booitem_descrip2) AS description,"
               "       booitem_effective, booitem_expires,"
               "       booitem_execday,"
	       "       CASE WHEN (booitem_configtype<>'N') THEN 'emphasis'"
               "            WHEN (booitem_expires < CURRENT_DATE) THEN 'expired'"
               "            WHEN (booitem_effective >= CURRENT_DATE) THEN 'future'"
               "       END AS qtforegroundrole,"
	       "       CASE WHEN COALESCE(booitem_effective, startOfTime()) ="
               "                 startOfTime() THEN 'Always'"
	       "       END AS booitem_effective_qtdisplayrole,"
	       "       CASE WHEN COALESCE(booitem_expires, endOfTime()) >="
               "                 endOfTime() THEN 'Never'"
	       "       END AS booitem_expires_qtdisplayrole "
               "FROM wrkcnt,"
	       "     booitem(<? value(\"item_id\") ?>,<? value(\"revision_id\") ?>) LEFT OUTER JOIN stdopn ON (booitem_stdopn_id=stdopn_id) "
               "WHERE ((booitem_wrkcnt_id=wrkcnt_id)"
	       "<? if not exists(\"showExpired\") ?> "
	       " AND (booitem_expires > CURRENT_DATE)"
	       "<? endif ?>"
	       "<? if not exists(\"showFuture\") ?> "
	       " AND (booitem_effective <= CURRENT_DATE)"
	       "<? endif ?>"
	       ") "
	       "ORDER BY booitem_seqnumber, booitem_effective" );
  MetaSQLQuery mql(sql);
  ParameterList params;
  if (! setParams(params))
    return;
  q = mql.toQuery(params);
  _booitem->populate(q, true);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

bool boo::checkSitePrivs(int booid)
{
  if (_preferences->boolean("selectedSites"))
  {
    XSqlQuery check;
    check.prepare("SELECT checkBOOSitePrivs(:booheadid) AS result;");
    check.bindValue(":booheadid", booid);
    check.exec();
    if (check.first())
    {
      if (!check.value("result").toBool())
      {
        QMessageBox::critical(this, tr("Access Denied"),
                              tr("You may not view or edit this BOO Item as it references "
                                 "a Site for which you have not been granted privileges.")) ;
        return false;
      }
    }
  }
  return true;
}

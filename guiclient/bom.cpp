/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "bom.h"

#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QValidator>
#include <QVariant>
#include <QSqlError>

#include <metasql.h>
#include "mqlutil.h"

#include <openreports.h>

#include "bomItem.h"
#include "errorReporter.h"

BOM::BOM(QWidget* parent, const char* name, Qt::WindowFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_moveUp, SIGNAL(clicked()), this, SLOT(sMoveUp()));
  connect(_moveDown, SIGNAL(clicked()), this, SLOT(sMoveDown()));
  connect(_item, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_revision, SIGNAL(newId(int)), this, SLOT(sFillList()));
  connect(_showExpired, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_showFuture, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
  connect(_bomitem, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_expire, SIGNAL(clicked()), this, SLOT(sExpire()));

  _totalQtyPerCache = 0.0;
  
  _item->setType(ItemLineEdit::cGeneralManufactured | ItemLineEdit::cGeneralPurchased |
                 ItemLineEdit::cPhantom | ItemLineEdit::cKit |
                 ItemLineEdit::cPlanning |
				 ItemLineEdit::cTooling);
  _batchSize->setValidator(omfgThis->qtyVal());
  _requiredQtyPer->setValidator(omfgThis->qtyPerVal());
  _nonPickNumber->setPrecision(omfgThis->qtyVal());
  _nonPickQtyPer->setPrecision(omfgThis->qtyPerVal());
  _pickNumber->setPrecision(omfgThis->qtyVal());
  _pickQtyPer->setPrecision(omfgThis->qtyPerVal());
  _totalNumber->setPrecision(omfgThis->qtyVal());
  _totalQtyPer->setPrecision(omfgThis->qtyPerVal());
  _currentStdCost->setPrecision(omfgThis->costVal());
  _currentActCost->setPrecision(omfgThis->costVal());
  _maxCost->setPrecision(omfgThis->costVal());
  
  _bomitem->addColumn(tr("#"),            _seqColumn,   Qt::AlignCenter, true, "bomitem_seqnumber");
  _bomitem->addColumn(tr("Item Number"),  _itemColumn,  Qt::AlignLeft,   true, "item_number");
  _bomitem->addColumn(tr("Description"),  -1,           Qt::AlignLeft,   true, "item_description");
  _bomitem->addColumn(tr("Type"),         _itemColumn,  Qt::AlignCenter, true, "itemtype");
  _bomitem->addColumn(tr("Issue UOM"),    _uomColumn,   Qt::AlignCenter, true, "issueuom");
  _bomitem->addColumn(tr("Issue Method"), _itemColumn,  Qt::AlignCenter, true, "issuemethod");
  _bomitem->addColumn(tr("Fixd. Qty."),   _qtyColumn,   Qt::AlignRight,  true, "bomitem_qtyfxd" );
  _bomitem->addColumn(tr("Qty. Per"),     _qtyColumn,   Qt::AlignRight,  true, "bomitem_qtyper" );
  _bomitem->addColumn(tr("Scrap %"),      _prcntColumn, Qt::AlignRight,  true, "bomitem_scrap" );
  _bomitem->addColumn(tr("Effective"),    _dateColumn,  Qt::AlignCenter, true, "effective");
  _bomitem->addColumn(tr("Expires"),      _dateColumn,  Qt::AlignCenter, true, "expires");
  _bomitem->addColumn(tr("Notes"),        _itemColumn,  Qt::AlignLeft,  false, "bomitem_notes"   );
  _bomitem->addColumn(tr("Reference"),    _itemColumn,  Qt::AlignLeft,  false, "bomitem_ref"   );
  _bomitem->setDragString("bomid=");
  _bomitem->setAltDragString("itemid=");
  
  if (!_privileges->check("ViewCosts"))
  {
    _currentStdCostLit->hide();
    _currentActCostLit->hide();
    _maxCostLit->hide();
    _currentStdCost->hide();
    _currentActCost->hide();
    _maxCost->hide();
  }
  
  connect(omfgThis, SIGNAL(bomsUpdated(int, bool)), SLOT(sFillList(int, bool)));
  _revision->setMode(RevisionLineEdit::Maintain);
  _revision->setType("BOM");
  _bomheadid = -1;
}

BOM::~BOM()
{
  // no need to delete child widgets, Qt does it all for us
}

void BOM::languageChange()
{
  retranslateUi(this);
}

enum SetResponse BOM::set(const ParameterList &pParams)
{
  XWidget::set(pParams);
  XSqlQuery bomet;
  QVariant param;
  bool     valid;
  
  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;
      _new->setEnabled(false);
      _revision->setId(-1);

      bomet.exec("SELECT NEXTVAL('bomhead_bomhead_id_seq') AS bomhead_id");
      if (bomet.first())
      {
        _bomheadid = bomet.value("bomhead_id").toInt();
        _documents->setId(_bomheadid);
        _comments->setId(_bomheadid);
      }
      else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving BOM Information"),
                                    bomet, __FILE__, __LINE__))
      {
        return UndefinedError;
      }
      
      emit newMode(_mode);
      emit newId(_bomheadid);
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _item->setReadOnly(true);
      
      emit newMode(_mode);
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _new->setEnabled(false);
      _item->setReadOnly(true);
      _documentNum->setEnabled(false);
//      _revision->setEnabled(false);
      _revisionDate->setEnabled(false);
      _batchSize->setEnabled(false);
      _doRequireQtyPer->setEnabled(false);
      _requiredQtyPer->setEnabled(false);
      _documents->setReadOnly(true);
      _save->setEnabled(false);
      _comments->setReadOnly(true);
      
      connect(_bomitem, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));
      
      emit newMode(_mode);
    }

    param = pParams.value("item_id", &valid);
    if (valid)
      _item->setId(param.toInt());
     {
       param = pParams.value("revision_id", &valid);
       if (valid)
         _revision->setId(param.toInt());
     }
  }
  
  return NoError;
}

int BOM::id() const
{
  return _bomheadid;
}

int BOM::mode() const
{
  return _mode;
}

bool BOM::sSave()
{
  if (save(false))
  {
    emit saved(_bomheadid);
    close();
    return true;
  }
  else
    return false;
}

bool BOM::save(bool partial)
{
  XSqlQuery BSave;
  if(_item->id() == -1)
  {
    QMessageBox::warning( this, tr("Item Number Required"),
      tr("You must specify a valid item number to continue."));
    return false;
  }
  
  if(_batchSize->text().length() == 0)
    _batchSize->setDouble(1.0);
  else if(_batchSize->toDouble() == 0.0)
  {
    QMessageBox::warning( this, tr("Batch Size Error"),
                         tr("<p>The Batch Size quantity must be greater than zero.") );
    return false;
  }
  
  if (!partial)
  {
    if(!sCheckRequiredQtyPer())
      return false;
  }
  
  if (_mode == cEdit)
  {   
    BSave.prepare( "UPDATE bomhead "
               "SET bomhead_docnum=:bomhead_docnum,"
               "    bomhead_revision=:bomhead_revision,"
               "    bomhead_revisiondate=:bomhead_revisiondate,"
               "    bomhead_batchsize=:bomhead_batchsize,"
               "    bomhead_requiredqtyper=:bomhead_requiredqtyper "
               "WHERE (bomhead_id=:bomhead_id);" );
  }
  else
  {
    BSave.prepare( "INSERT INTO bomhead "
               "( bomhead_id, bomhead_item_id, bomhead_docnum,"
               "  bomhead_revision, bomhead_revisiondate,"
               "  bomhead_batchsize, bomhead_requiredqtyper ) "
               "VALUES "
               "( :bomhead_id, :bomhead_item_id, :bomhead_docnum,"
               "  :bomhead_revision, :bomhead_revisiondate, "
               "  :bomhead_batchsize, :bomhead_requiredqtyper ) " );
  }
  
  BSave.bindValue(":bomhead_id", _bomheadid);
  BSave.bindValue(":bomhead_item_id", _item->id());
  BSave.bindValue(":bomhead_rev_id", _revision->id());
  BSave.bindValue(":bomhead_docnum", _documentNum->text());
  BSave.bindValue(":bomhead_revision", _revision->number());
  BSave.bindValue(":bomhead_revisiondate", _revisionDate->date());
  BSave.bindValue(":bomhead_batchsize", _batchSize->toDouble());
  if(_doRequireQtyPer->isChecked())
    BSave.bindValue(":bomhead_requiredqtyper", _requiredQtyPer->text().toDouble());
  BSave.exec();

  if (_mode == cNew)
  {
    _mode=cEdit;
    emit newMode(_mode);
  }

  omfgThis->sBOMsUpdated(_bomheadid, true);
  return true;
}

bool BOM::setParams(ParameterList &pParams)
{
  pParams.append("item_id",     _item->id());
  pParams.append("revision_id", _revision->id());
  pParams.append("push",        tr("Push"));
  pParams.append("pull",        tr("Pull"));
  pParams.append("mixed",       tr("Mixed"));
  pParams.append("error",       tr("Error"));
  pParams.append("always",      tr("'Always'"));
  pParams.append("never",       tr("'Never'"));

  if (_showExpired->isChecked())
  {
    pParams.append("showExpired");
    pParams.append("expiredDays", 999);
  }
  
  if (_showFuture->isChecked())
  {
    pParams.append("showFuture");
    pParams.append("futureDays", 999);
  }
  
  pParams.append("purchased", tr("Purchased"));
  pParams.append("manufactured", tr("Manufactured"));
  pParams.append("job", tr("Job"));
  pParams.append("phantom", tr("Phantom"));
  pParams.append("breeder", tr("Breeder"));
  pParams.append("coProduct", tr("Co-Product"));
  pParams.append("byProduct", tr("By-Product"));
  pParams.append("reference", tr("Reference"));
  pParams.append("costing", tr("Costing"));
  pParams.append("tooling", tr("Tooling"));
  pParams.append("outside", tr("Outside Process"));
  pParams.append("planning", tr("Planning"));
  pParams.append("assortment", tr("Assortment"));
  pParams.append("kit", tr("Kit"));
  pParams.append("error", tr("Error"));

  return true;
}

void BOM::sPrint()
{
  ParameterList params;
  setParams(params);
  orReport report("SingleLevelBOM", params);
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void BOM::sPopulateMenu(QMenu *menuThis)
{
  menuThis->addAction(tr("View"), this, SLOT(sView()));
  
  if ((_mode == cNew) || (_mode == cEdit))
  {
    menuThis->addAction(tr("Edit"), this, SLOT(sEdit()));
    menuThis->addAction(tr("Expire"), this, SLOT(sExpire()));
    menuThis->addAction(tr("Replace"), this, SLOT(sReplace()));

    if (_metrics->boolean("AllowBOMItemDelete"))
    {
      menuThis->addSeparator();

      menuThis->addAction(tr("Delete"), this, SLOT(sDelete()));
    }
    
    menuThis->addSeparator();
    
    menuThis->addAction(tr("Move Up"),   this, SLOT(sMoveUp()));
    menuThis->addAction(tr("Move Down"), this, SLOT(sMoveDown()));
  }
}

void BOM::sNew()
{
  if(_mode == cNew)
  {
    if(!save(true))
      return;
  }
  
  ParameterList params;
  params.append("mode", "new");
  params.append("bomhead_id", _bomheadid);
  params.append("item_id", _item->id());
  params.append("revision_id", _revision->id());
  
  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("bomitem_id", _bomitem->id());
  
  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("bomitem_id", _bomitem->id());
  
  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sExpire()
{
  XSqlQuery BExpire;
  BExpire.prepare( "UPDATE bomitem "
             "SET bomitem_expires=CURRENT_DATE "
             "WHERE (bomitem_id=:bomitem_id);" );
  BExpire.bindValue(":bomitem_id", _bomitem->id());
  BExpire.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), true);
}

void BOM::sDelete()
{
  XSqlQuery BDelete;
  if (QMessageBox::question(this, tr("Delete Item?"),
                            tr("<p>This action can not be undone. "
                               "Are you sure you want to delete this Item?"),
                            QMessageBox::Yes,
                            QMessageBox::No | QMessageBox::Default) == QMessageBox::Yes)
  {
    BDelete.prepare( "DELETE FROM bomitem "
               "WHERE (bomitem_id=:bomitem_id);" );
    BDelete.bindValue(":bomitem_id", _bomitem->id());
    BDelete.exec();

    omfgThis->sBOMsUpdated(_item->id(), true);
  }
}

void BOM::sReplace()
{
  ParameterList params;
  params.append("mode", "replace");
  params.append("bomitem_id", _bomitem->id());
  params.append("revision_id", _revision->id());
  
  bomItem newdlg(this, "", true);
  newdlg.set(params);
  newdlg.exec();
}

void BOM::sMoveUp()
{
  XSqlQuery BMoveUp;
  BMoveUp.prepare("SELECT moveBomitemUp(:bomitem_id) AS result;");
  BMoveUp.bindValue(":bomitem_id", _bomitem->id());
  BMoveUp.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), true);
}

void BOM::sMoveDown()
{
  XSqlQuery BMoveDown;
  BMoveDown.prepare("SELECT moveBomitemDown(:bomitem_id) AS result;");
  BMoveDown.bindValue(":bomitem_id", _bomitem->id());
  BMoveDown.exec();
  
  omfgThis->sBOMsUpdated(_item->id(), true);
}

void BOM::sFillList()
{
  XSqlQuery BFillList;
  sFillList(_item->id(), true);
}

void BOM::sFillList(int pItemid, bool)
{
  XSqlQuery BFillList;
  if (_item->isValid() && (pItemid == _item->id()))
  {
    BFillList.prepare( "SELECT * "
               "FROM bomhead "
               "WHERE ( (bomhead_item_id=:item_id) "
               "AND (bomhead_rev_id=:revision_id) );" );
    BFillList.bindValue(":item_id", _item->id());
    BFillList.bindValue(":revision_id", _revision->id());
    BFillList.exec();
    if (BFillList.first())
    {
      _bomheadid = BFillList.value("bomhead_id").toInt();
      if (_mode == cNew)
      {
        _mode = cEdit;
        emit newMode(_mode);
        emit newId(_bomheadid);
      }
      _comments->setId(_bomheadid);
      _documents->setId(_bomheadid);
      _documentNum->setText(BFillList.value("bomhead_docnum"));
      _revisionDate->setDate(BFillList.value("bomhead_revisiondate").toDate());
      _batchSize->setDouble(BFillList.value("bomhead_batchsize").toDouble());
      if(BFillList.value("bomhead_requiredqtyper").toDouble()!=0)
      {
        _doRequireQtyPer->setChecked(true);
        _requiredQtyPer->setDouble(BFillList.value("bomhead_requiredqtyper").toDouble());
      }
      _revision->setNumber(BFillList.value("bomhead_revision").toString());
      if ( (_revision->description() == "Inactive") || (_mode == cView) )
      {
        _save->setEnabled(false);
        _documentNum->setEnabled(false);
        _revisionDate->setEnabled(false);
        _batchSize->setEnabled(false);
        _new->setEnabled(false);
        _edit->setEnabled(false);
        _expire->setEnabled(false);
        _moveUp->setEnabled(false);
        _moveDown->setEnabled(false);
        disconnect(_bomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
        disconnect(_bomitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
        disconnect(_bomitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
        disconnect(_bomitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
        disconnect(_bomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      }
      else
      {
        _save->setEnabled(true);
        _documentNum->setEnabled(true);
        _revisionDate->setEnabled(true);
        _batchSize->setEnabled(true);
        _new->setEnabled(true);
        _edit->setEnabled(false);
        _expire->setEnabled(false);
        _moveUp->setEnabled(false);
        _moveDown->setEnabled(false);
        connect(_bomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
      }
    }
    else
    {
      if (_mode != cView)
      {
        _save->setEnabled(true);
        _documentNum->setEnabled(true);
        _revisionDate->setEnabled(true);
        _batchSize->setEnabled(true);
        _new->setEnabled(true);
        _edit->setEnabled(false);
        _expire->setEnabled(false);
        _moveUp->setEnabled(false);
        _moveDown->setEnabled(false);
        connect(_bomitem, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _expire, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _moveUp, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(valid(bool)), _moveDown, SLOT(setEnabled(bool)));
        connect(_bomitem, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
        _documentNum->clear();
        _revisionDate->clear();
        _batchSize->clear();
      }
    }
    
    ParameterList params;
    setParams(params);
    
    MetaSQLQuery mql = mqlLoad("bomItems", "detail");
    BFillList = mql.toQuery(params);
    _bomitem->populate(BFillList);
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving BOM Information"),
                                  BFillList, __FILE__, __LINE__))
    {
      return;
    }
    
    MetaSQLQuery picklistmql("SELECT item_picklist,"
          "       COUNT(*) AS total,"
          "       COALESCE(SUM(bomitem_qtyper * (1 + bomitem_scrap))) AS qtyper "
          "FROM bomitem(<? value(\"item_id\") ?>,"
          "             <? value(\"revision_id\") ?>), item "
          "WHERE ( (bomitem_item_id=item_id)"
          "<? if not exists(\"showExpired\") ?>"
          " AND (bomitem_expires > CURRENT_DATE)"
          "<? endif ?>"
          "<? if not exists(\"showFuture\") ?>"
          " AND (bomitem_effective <= CURRENT_DATE)"
          "<? endif ?>"
          " ) "
          "GROUP BY item_picklist;");
    BFillList = picklistmql.toQuery(params);
    
    bool   foundPick    = false;
    bool   foundNonPick = false;
    int    totalNumber  = 0;
    double totalQtyPer  = 0.0;
    while (BFillList.next())
    {
      totalNumber += BFillList.value("total").toInt();
      totalQtyPer += BFillList.value("qtyper").toDouble();
      
      if (BFillList.value("item_picklist").toBool())
      {
        foundPick = true;
        _pickNumber->setDouble(BFillList.value("total").toDouble());
        _pickQtyPer->setDouble(BFillList.value("qtyper").toDouble());
      }
      else
      {
        foundNonPick = true;
        _nonPickNumber->setDouble(BFillList.value("total").toDouble());
        _nonPickQtyPer->setDouble(BFillList.value("qtyper").toDouble());
      }
    }
    if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving BOM Information"),
                                  BFillList, __FILE__, __LINE__))
    {
      return;
    }
    
    if (!foundPick)
    {
      _pickNumber->setDouble(0);
      _pickQtyPer->setDouble(0.0);
    }
    
    if (!foundNonPick)
    {
      _nonPickNumber->setDouble(0);
      _nonPickQtyPer->setDouble(0.0);
    }
    
    _totalNumber->setDouble(totalNumber);
    _totalQtyPer->setDouble(totalQtyPer);
    _totalQtyPerCache = totalQtyPer;
    
    if (_privileges->check("ViewCosts"))
    {
      MetaSQLQuery costsmql("SELECT p.item_maxcost,"
            "       COALESCE(SUM(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * stdCost(c.item_id))) AS stdcost,"
            "       COALESCE(SUM(itemuomtouom(bomitem_item_id, bomitem_uom_id, NULL, bomitem_qtyper * (1 + bomitem_scrap)) * ROUND(actCost(c.item_id),4))) AS actcost "
            "FROM bomitem(<? value(\"item_id\") ?>,"
            "             <? value(\"revision_id\") ?>), item AS c, item AS p "
            "WHERE ( (bomitem_item_id=c.item_id)"
            " AND (p.item_id=<? value(\"item_id\") ?>)"
            "<? if not exists(\"showExpired\") ?>"
            " AND (bomitem_expires > CURRENT_DATE)"
            "<? endif ?>"
            "<? if not exists(\"showFuture\") ?>"
            " AND (bomitem_effective <= CURRENT_DATE)"
            "<? endif ?>"
            " ) "
            "GROUP BY p.item_maxcost;");
      BFillList = costsmql.toQuery(params);
      if (BFillList.first())
      {
        _currentStdCost->setDouble(BFillList.value("stdcost").toDouble());
        _currentActCost->setDouble(BFillList.value("actcost").toDouble());
        _maxCost->setDouble(BFillList.value("item_maxcost").toDouble());
      }
      if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving BOM Information"),
                                    BFillList, __FILE__, __LINE__))
      {
        return;
      }
    }
    
    emit populated();
  }
  else if (!_item->isValid())
  {
    _documentNum->clear();
    _revision->clear();
    _revisionDate->clear();
    _batchSize->clear();
    
    _bomitem->clear();
  }
}

bool BOM::sCheckRequiredQtyPer()
{
  if(cView == _mode || !_doRequireQtyPer->isChecked() || _totalQtyPerCache == 0.0)
    return true;

  if(_requiredQtyPer->toDouble() != _totalQtyPerCache)
  {
    QMessageBox::warning( this, tr("Total Qty. Per Required"),
      tr("<p>A required total Qty. Per was specified but not met. "
         "Please correct the problem before continuing.") );
    return false;
  }
  
  return true;
}

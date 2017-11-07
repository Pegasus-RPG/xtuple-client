/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "createLotSerial.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QValidator>
#include <QVariant>

#include <parameter.h>
#include <openreports.h>
#include "errorReporter.h"
#include "guiErrorCheck.h"

createLotSerial::createLotSerial(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
    : XDialog(parent, name, modal, fl),
      _lotsFound(false)
{
  setupUi(this);

  connect(_assign, SIGNAL(clicked()), this, SLOT(sAssign()));
  connect(_lotSerial, SIGNAL(textChanged(QString)), this, SLOT(sHandleLotSerial()));
  connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sHandleCharacteristics()));

  _item->setReadOnly(true);

  _serial = false;
  _itemsiteid = -1;
  _preassigned = false;
  adjustSize();

  _qtyToAssign->setValidator(omfgThis->qtyVal());
  _qtyRemaining->setPrecision(omfgThis->qtyVal());

  _charWidgets = LotSerialUtils::addLotCharsToGridLayout(this, gridLayout, _lschars);
}

createLotSerial::~createLotSerial()
{
  // no need to delete child widgets, Qt does it all for us
}

void createLotSerial::languageChange()
{
  retranslateUi(this);
}

enum SetResponse createLotSerial::set(const ParameterList &pParams)
{
  XSqlQuery createet;
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("itemloc_series", &valid);
  if (valid)
    _itemlocSeries = param.toInt();

  param = pParams.value("itemlocdist_id", &valid);
  if (valid)
  {
    _itemlocdistid = param.toInt();
    createet.prepare("SELECT item_fractional, itemsite_controlmethod, itemsite_item_id, "
                      " itemsite_id, itemsite_perishable, itemsite_warrpurc, "
                      " COALESCE(itemsite_lsseq_id,-1) AS itemsite_lsseq_id, "
                      " formatOrderLineItemNumber(itemlocdist_order_type, itemlocdist_order_id) AS ordnumber, "
                      " itemlocdist_transtype AS transtype, "
                      " itemlocdist_order_type AS ordtype "
                      "FROM itemlocdist "
                      " JOIN itemsite ON itemlocdist_itemsite_id = itemsite_id "
                      " JOIN item ON itemsite_item_id = item_id "
                      "WHERE itemlocdist_id = :itemlocdist_id;");
    createet.bindValue(":itemlocdist_id", _itemlocdistid);
    createet.exec();
    if (createet.first())
    {
      if (createet.value("itemsite_controlmethod").toString() == "S")
      {
        _serial = true;
        _qtyToAssign->setDouble(1.0);
        _qtyToAssign->setEnabled(false);
      }
      else
        _serial = false;

      _item->setItemsiteid(createet.value("itemsite_id").toInt());
      _itemsiteid = createet.value("itemsite_id").toInt();
      _expiration->setEnabled(createet.value("itemsite_perishable").toBool());
      _warranty->setEnabled(createet.value("itemsite_warrpurc").toBool() && createet.value("ordtype").toString() == "PO");
      _fractional = createet.value("item_fractional").toBool();

      // Auto sequence
      if (createet.value("itemsite_lsseq_id").toInt() != -1)
      {
        XSqlQuery fetchlsnum;
        fetchlsnum.prepare("SELECT fetchlsnumber(:lsseq_id) AS lotserial;");
        fetchlsnum.bindValue(":lsseq_id", createet.value("itemsite_lsseq_id").toInt());
        fetchlsnum.exec();
        if (fetchlsnum.first())
        {
          _lotSerial->setAllowNull(true);
          _lotSerial->setText(fetchlsnum.value("lotserial").toString());
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot/Serial Information"),
                                      fetchlsnum, __FILE__, __LINE__))
        {
          return UndefinedError;
        }
      }
      else
      {
        // First check for "pre-assigned/associated" lots. A couple example use cases:
        // 1) Receiving TO - get lot(s) from issue to shipping trans. 2) Receiving RA - get lot(s) from original SO (if exists).
        XSqlQuery origTransLots;
        origTransLots.prepare("SELECT ls_number, ls_number FROM ls WHERE ls_id = ANY(getAssocLotSerialIds(pItemlocdistId := :itemlocdist_id));");
        origTransLots.bindValue(":itemlocdist_id", _itemlocdistid);
        origTransLots.exec();
        if (origTransLots.first())
        {
          _lotSerial->setAllowNull(true);
          _lotSerial->populate(origTransLots);
          _preassigned = true;
          _lotSerial->setEditable(_privileges->check("EnterUnassociatedLotSerialNumbers"));
          connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sLotSerialSelected()));
        }
        else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot/Serial Information"),
                                      origTransLots, __FILE__, __LINE__))
        {
          return UndefinedError;
        }
        // No pre-assigned/associated transactions so check if there are lot's in stock to populate menu
        else if (!_serial)
        {
          XSqlQuery lots;
          lots.prepare("SELECT itemloc_id, ls_number FROM itemloc JOIN ls on itemloc_ls_id=ls_id WHERE itemloc_itemsite_id=:itemsite_id;");
          lots.bindValue(":itemsite_id", _itemsiteid);
          lots.exec();
          if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot/Serial Information"),
                                        lots, __FILE__, __LINE__))
          {
            return UndefinedError;
          }
          _lotSerial->setAllowNull(true);
          _lotSerial->populate(lots);
          connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sLotSerialSelected()));
        }
      }
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Retrieving Lot/Serial Information"),
                                  createet, __FILE__, __LINE__))
    {
      return UndefinedError;
    }
  }

  param = pParams.value("qtyRemaining", &valid);
  if (valid)
    _qtyRemaining->setDouble(param.toDouble());

  return NoError;
}

void createLotSerial::sHandleLotSerial()
{
    _lotSerial->setText(_lotSerial->currentText().toUpper());
}

void createLotSerial::sHandleCharacteristics()
{
    if (_lotSerial->currentText().length() == 0)
    {
        clearCharacteristics();
        return;
    }

    int ls_id = -1;
    XSqlQuery lotcharq;
    lotcharq.prepare(QString("SELECT ls_id FROM ls WHERE ls_item_id=:ls_item_id AND ls_number=:ls_number"));
    lotcharq.bindValue(":ls_number", _lotSerial->currentText());
    lotcharq.bindValue(":ls_item_id", _item->id());
    bool success = lotcharq.exec();
    if (!success)
    {
        qDebug() << __FUNCTION__ << __LINE__ << lotcharq.lastError().text();
        return;
    }
    if( lotcharq.first() )
    {
        ls_id = lotcharq.value("ls_id").toInt();
    }
    _lotsFound = ls_id > -1;
    if (_lotsFound)
    {
        XSqlQuery charQuery;
        charQuery.prepare(QString("SELECT DISTINCT char.char_name, char.char_type, charass.charass_value "
                          " FROM charass "
                          " JOIN char ON (charass_char_id = char_id) "
                          " JOIN charuse ON (charuse_char_id = char_id) "
                          " WHERE ((charass.charass_target_type = 'LS') "
                          "  AND   (charass.charass_target_id=%1) "
                          "  AND   (charuse_target_type = 'LS')) "
                          " ORDER BY char.char_name ASC").arg(ls_id));
        success = charQuery.exec();
        if (!success)
        {
            qDebug() << __FUNCTION__ << __LINE__ << charQuery.lastError().text();
        }
        for (int i = 0; charQuery.next() && i < _charWidgets.length(); i++)
        {
            QString charass_value = charQuery.value("charass_value").toString();
            int char_type = charQuery.value("char_type").toInt();
            if (char_type == 2)
            {
                DLineEdit *l = qobject_cast<DLineEdit *>(_charWidgets.at(i));
                if (l)
                    l->setDate(QDate::fromString(charass_value, "yyyy-MM-dd"));
            }
            else if (char_type == 1)
            {
                XComboBox *x = qobject_cast<XComboBox *>(_charWidgets.at(i));
                if (x) {
                  int index = x->findText(charass_value);
                  if (index > -1) {
                    x->setCurrentIndex(index);
                  }
                }
            }
            else
            {
                QLineEdit *l = qobject_cast<QLineEdit *>(_charWidgets.at(i));
                if (l)
                    l->setText(charass_value);
            }
        }
    }
    else
    {
      clearCharacteristics();
    }
    foreach (QWidget *w, _charWidgets)
    {
        w->setEnabled(!_lotsFound);
    }
}

void createLotSerial::clearCharacteristics()
{
  QList<int> char_types = _lschars.getLotCharTypes();
  for (int i=0; i < _lschars.numLotChars(); i++)
  {
      if (char_types.at(i) == 2)
      {
          DLineEdit *l = qobject_cast<DLineEdit *>(_charWidgets.at(i));
          if (l)
            l->clear();
      }
      else if (char_types.at(i) == 1)
      {
          XComboBox *x = qobject_cast<XComboBox *>(_charWidgets.at(i));
          if (x)
            x->setCurrentIndex(0);
      }
      else
      {
          QLineEdit *l = qobject_cast<QLineEdit *>(_charWidgets.at(i));
          if (l)
            l->clear();
      }
   }
}

void createLotSerial::sAssign()
{
  XSqlQuery createAssign;

  QString lsnum = _lotSerial->currentText().trimmed().toUpper();
  if (lsnum.contains(QRegExp("\\s")) &&
      QMessageBox::question(this, tr("Lot/Serial Number Contains Spaces"),
                            tr("<p>The Lot/Serial Number contains spaces. Do "
                               "you want to save it anyway?"),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::No)
  {
    _lotSerial->setFocus();
    return;
  }

  QList<GuiErrorCheck>errors;
  float decimals = _qtyToAssign->toDouble() - floor(_qtyToAssign->toDouble());
  errors<<GuiErrorCheck(lsnum.isEmpty() && _lotSerial->isEditable(), _lotSerial,
                        tr("<p>You must enter a Lot/Serial number."))
        <<GuiErrorCheck(lsnum.isEmpty() && ! _lotSerial->isEditable(), _lotSerial,
                        tr("<p>You must select a preassigned Lot/Serial number."))
        <<GuiErrorCheck(_qtyToAssign->toDouble() == 0.0, _qtyToAssign,
                        tr("<p>You must enter a positive value to assign to "
                           "this Lot/Serial number."))
        <<GuiErrorCheck((_expiration->isEnabled()) && (!_expiration->isValid()), _expiration,
                        tr("<p>You must enter an expiration date to this "
                           "Perishable Lot/Serial number."))
        <<GuiErrorCheck((_warranty->isEnabled()) && (!_warranty->isValid()), _warranty,
                        tr("<p>You must enter a warranty expiration date for this "
                           "Lot/Serial number."))
        <<GuiErrorCheck((!_fractional) && (decimals > 0), _qtyToAssign,
                        tr("<p>The Item in question is not stored in "
                           "fractional quantities. You must enter a "
                           "whole value to assign to this Lot/Serial "
                           "number."))
  ;

  if(GuiErrorCheck::reportErrors(this,tr("Cannot Assign Lot/Serial number"),errors))
      return;

  // If serial, validate the serial number.
  if (_serial)
  {
    createAssign.prepare("SELECT true "
                         "FROM itemsite "
                         "  JOIN itemloc ON itemloc_itemsite_id = itemsite_id "
                         "  JOIN ls ON ls_id = itemloc_ls_id "
                         "WHERE itemsite_item_id = :item_id "
                         "  AND UPPER(ls_number) = UPPER(:lotserial) "
                         "UNION "
                         "SELECT true "
                         "FROM itemsite "
                         "  JOIN itemlocdist ON itemlocdist_itemsite_id = itemsite_id "
                         "  JOIN ls ON ls_id = itemlocdist_ls_id "
                         "WHERE itemsite_item_id = :item_id "
                         "  AND UPPER(ls_number) = UPPER(:lotserial)"
                         "  AND itemlocdist_source_type = 'D';");
    createAssign.bindValue(":item_id", _item->id());
    createAssign.bindValue(":lotserial", lsnum);
    createAssign.exec();
    if (createAssign.first() && !_preassigned) // Allow re-using of "pre-assigned" serial numbers.
    {
      QMessageBox::critical(this, tr("Duplicate Serial Number"),
                            tr("This Serial Number has already been used "
                               "and cannot be reused."));

      return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Lot/Serial Information"),
                                  createAssign, __FILE__, __LINE__))
    {
      return;
    }
  }
  else
  {
    createAssign.prepare("SELECT ls_id, ls_number "
              "FROM ls,itemsite "
              "WHERE ( (UPPER(ls_number)=UPPER(:lotserial)) "
              "AND (ls_item_id=itemsite_item_id) "
              "AND (itemsite_id=:itemsiteid) );");
    createAssign.bindValue(":itemsiteid", _itemsiteid);
    createAssign.bindValue(":lotserial", lsnum);
    createAssign.exec();
    if (createAssign.first())
    {
      if (!_serial && !_preassigned)
        if (QMessageBox::question(this, tr("Use Existing?"),
                                  tr("<p>A record with for lot number %1 for this item already exists.  "
                                     "Reference existing lot?").arg(createAssign.value("ls_number").toString().toUpper()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes) == QMessageBox::No)
          return;
    }
    else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Lot/Serial Information"),
                                  createAssign, __FILE__, __LINE__))
    {
      return;
    }
  }

  QString sql;
  int itemlocdist_id = -1;
  sql = "SELECT createlotserial(:itemsite_id,:lotserial,:itemlocseries,'I',NULL,itemlocdist_id,:qty,:expiration,:warranty) AS id "
        "FROM itemlocdist "
        "WHERE (itemlocdist_id=:itemlocdist_id);";

  createAssign.prepare(sql);
  createAssign.bindValue(":itemsite_id", _itemsiteid);
  createAssign.bindValue(":lotserial",   lsnum);
  createAssign.bindValue(":itemlocseries", _itemlocSeries);
  createAssign.bindValue(":lsdetail_id", _lotSerial->id());
  createAssign.bindValue(":qty", _qtyToAssign->toDouble());
  if (_expiration->isEnabled())
    createAssign.bindValue(":expiration", _expiration->date());
  else
    createAssign.bindValue(":expiration", omfgThis->endOfTime());
  if (_warranty->isEnabled())
    createAssign.bindValue(":warranty", _warranty->date());
  createAssign.bindValue(":itemlocdist_id", _itemlocdistid);
  createAssign.exec();
  if (createAssign.first())
    itemlocdist_id = createAssign.value("id").toInt();
  if (ErrorReporter::error(QtCriticalMsg, this, tr("Error Saving Lot/Serial Information"),
                                createAssign, __FILE__, __LINE__))
  {
    return;
  }

  if (!_lotsFound)
  {
      /* createlotserial calls "NEXTVAL('ls_ls_id_seq') which will advance the
       * next ls_id.  Fetch the last ls_id */
      int ls_id = -1;
      XSqlQuery ls_id_query;
      ls_id_query.prepare("SELECT itemlocdist_ls_id FROM itemlocdist WHERE itemlocdist_id=:itemlocdist_id");
      ls_id_query.bindValue(":itemlocdist_id", itemlocdist_id);
      ls_id_query.exec();
      if (ls_id_query.first()) {
          ls_id = ls_id_query.value("itemlocdist_ls_id").toInt();
          _lschars.updateLotCharacteristics(ls_id, _charWidgets);
      }
  }

  accept();
}

void createLotSerial::sLotSerialSelected()
{
    if (!_expiration->isEnabled() &&
        !_warranty->isEnabled())
      return;

    if (_lotSerial->currentText().length() == 0)
    {
        _expiration->clear();
        _warranty->clear();
        return;
    }

    XSqlQuery itemloc;
    itemloc.prepare("SELECT itemloc_expiration, itemloc_warrpurc "
                    "FROM itemloc "
                    "WHERE itemloc_id=:itemloc_id; ");
    itemloc.bindValue(":itemloc_id", _lotSerial->id());
    itemloc.exec();
    if (itemloc.first()) {
      if (_expiration->isEnabled() &&
          itemloc.value("itemloc_expiration").toDate() != omfgThis->endOfTime())
        _expiration->setDate(itemloc.value("itemloc_expiration").toDate());
      if (_warranty->isEnabled())
        _warranty->setDate(itemloc.value("itemloc_warrpurc").toDate());
    }
}

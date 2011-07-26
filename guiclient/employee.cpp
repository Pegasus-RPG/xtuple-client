/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QSqlError>
#include <QVariant>

#include <parameter.h>

#include "characteristicAssignment.h"
#include "employee.h"
#include "empGroup.h"
#include "empgroupcluster.h"
#include "errorReporter.h"
#include "guiErrorCheck.h"
#include "salesRep.h"
#include "vendor.h"
#include "storedProcErrorLookup.h"
#include "user.h"

#define DEBUG   false

// TODO: XDialog should have a default implementation that returns FALSE
bool employee::userHasPriv(const int pMode)
{
  if (DEBUG)
    qDebug("employee::userHasPriv(%d)", pMode);
  bool retval = false;
  switch (pMode)
  {
    case cView:
      retval = _privileges->check("ViewEmployees") ||
               _privileges->check("MaintainEmployees");
      break;
    case cNew:
    case cEdit:
      retval = _privileges->check("MaintainEmployees");
      break;
    default:
      retval = false;
      break;
  }
  if (DEBUG)
    qDebug("employee::userHasPriv(%d) returning %d", pMode, retval);
  return retval;
}

// TODO: this code really belongs in XDialog
void employee::setVisible(bool visible)
{
  if (DEBUG)
    qDebug("employee::setVisible(%d) called with mode() == %d",
           visible, _mode);
  if (! visible)
    XDialog::setVisible(false);

  else if (! userHasPriv(_mode))
  {
    ErrorReporter::error(QtCriticalMsg, this, tr("No Privileges"),
                          tr("You do not have sufficient privilege to view "
                             "this window"));
    reject();
  }
  else
    XDialog::setVisible(true);
}

employee::employee(QWidget* parent, const char * name, Qt::WindowFlags fl)
    : XDialog(parent, name, fl)
{
  setupUi(this);

  connect(_attachGroup,   SIGNAL(clicked()), this, SLOT(sAttachGroup()));
  connect(_code,  SIGNAL(editingFinished()), this, SLOT(sHandleButtons()));
  connect(_deleteCharass, SIGNAL(clicked()), this, SLOT(sDeleteCharass()));
  connect(_detachGroup,   SIGNAL(clicked()), this, SLOT(sDetachGroup()));
  connect(_editCharass,   SIGNAL(clicked()), this, SLOT(sEditCharass()));
  connect(_editGroup,     SIGNAL(clicked()), this, SLOT(sEditGroup()));
  connect(_newCharass,    SIGNAL(clicked()), this, SLOT(sNewCharass()));
  connect(_salesrepButton,SIGNAL(clicked()), this, SLOT(sSalesrep()));
  connect(_vendorButton,  SIGNAL(clicked()), this, SLOT(sVendor()));
  connect(_save,          SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_userButton,    SIGNAL(clicked()), this, SLOT(sUser()));
  connect(_viewGroup,     SIGNAL(clicked()), this, SLOT(sViewGroup()));

  XSqlQuery xtmfg;
  xtmfg.exec("SELECT pkghead_name FROM pkghead WHERE pkghead_name='xtmfg'");
  if (xtmfg.first())
  {
    _shift->setEnabled(true);
    _shift->setVisible(true);
    shiftLit->setVisible(true);
  } else {
    _shift->setEnabled(false);
    _shift->setVisible(false);
    shiftLit->setVisible(false);
  }

  _charass->addColumn(tr("Characteristic"), _itemColumn, Qt::AlignLeft, true, "char_name");
  _charass->addColumn(tr("Value"),          -1,          Qt::AlignLeft, true, "charass_value");

  _groups->addColumn(tr("Name"), _itemColumn, Qt::AlignLeft, true, "empgrp_name");
  _groups->addColumn(tr("Description"),   -1, Qt::AlignLeft, true, "empgrp_descrip");

  if (_privileges->check("MaintainSalesReps") ||
      _privileges->check("ViewSalesReps"))
    connect(_salesrep, SIGNAL(toggled(bool)), _salesrepButton, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainVendors") ||
      _privileges->check("ViewVendors"))
    connect(_vendor, SIGNAL(toggled(bool)), _vendorButton, SLOT(setEnabled(bool)));
  if (_privileges->check("MaintainUsers"))
    connect(_user, SIGNAL(toggled(bool)), _userButton, SLOT(setEnabled(bool)));

  _wagetype->setAllowNull(false);
  _wagetype->append(0, tr("Hourly"),      "H");
  _wagetype->append(1, tr("Salaried"),    "S");

  _per->setAllowNull(false);
  _per->append(0, tr("Hour"),      "H");
  _per->append(1, tr("Day"),       "D");
  _per->append(2, tr("Week"),      "W");
  _per->append(3, tr("Bi-Weekly"), "BW");
  _per->append(4, tr("Month"),     "M");
  _per->append(5, tr("Year"),      "Y");

  _per->setAllowNull(false);
  _perExt->append(0, tr("Hour"),      "H");
  _perExt->append(1, tr("Day"),       "D");
  _perExt->append(2, tr("Week"),      "W");
  _perExt->append(3, tr("Bi-Weekly"), "BW");
  _perExt->append(4, tr("Month"),     "M");
  _perExt->append(4, tr("Year"),      "Y");

  _comments->setId(-1);
  _comments->setReadOnly(true);

  _createUsers= false;
  _crmacctid  = -1;
  _empid      = -1;
  _salesrepid = -1;
  _username   = "";
  _vendid     = -1;
  _NumberGen  = -1;
  _mode     = cView;
  _origmode = cView;
}

employee::~employee()
{
  // no need to delete child widgets, Qt does it all for us
}

void employee::languageChange()
{
  retranslateUi(this);
}

enum SetResponse employee::set(const ParameterList &pParams)
{
  XDialog::set(pParams);
  QVariant param;
  bool     valid;

  param = pParams.value("crmacct_id", &valid);
  if (valid)
    _crmacctid = param.toInt();

  param = pParams.value("emp_id", &valid);
  if (valid)
    _empid   = param.toInt();

  if (_empid > 0 || _crmacctid > 0)
    if (! sPopulate())
      return UndefinedError;;

  param = pParams.value("mode", &valid);
  if (valid)
  {
    if (param.toString() == "new")
    {
      _mode = cNew;

      if(((_metrics->value("CRMAccountNumberGeneration") == "A") ||
          (_metrics->value("CRMAccountNumberGeneration") == "O"))
       && _code->text().isEmpty() )
      {
        XSqlQuery numq;
        numq.exec("SELECT fetchCRMAccountNumber() AS number;");
        if (numq.first())
        {
          _code->setText(numq.value("number"));
          _NumberGen = numq.value("number").toInt();
        }
      }
      else
        _code->setFocus();

      _salesrep->setEnabled(_privileges->check("MaintainSalesReps"));
      _vendor->setEnabled(_privileges->check("MaintainVendors"));
    }
    else if (param.toString() == "edit")
    {
      _mode = cEdit;
      _number->setFocus();
    }
    else if (param.toString() == "view")
    {
      _mode = cView;
      _save->hide();
      _close->setText(tr("&Close"));
      _close->setFocus();
    }
  }

  bool editing = (_mode == cNew || _mode == cEdit);
  if (editing)
  {
    connect(_charass,SIGNAL(valid(bool)), _deleteCharass, SLOT(setEnabled(bool)));
    connect(_charass,SIGNAL(valid(bool)), _editCharass, SLOT(setEnabled(bool)));
    connect(_charass,SIGNAL(itemSelected(int)), _editCharass, SLOT(animateClick()));
    connect(_groups, SIGNAL(valid(bool)), _detachGroup, SLOT(setEnabled(bool)));
    _attachGroup->setEnabled(true);
    if (empGroup::userHasPriv(cEdit))
    {
      connect(_groups, SIGNAL(valid(bool)), _editGroup,   SLOT(setEnabled(bool)));
    }
    if (empGroup::userHasPriv(cView))
      connect(_groups, SIGNAL(valid(bool)), _viewGroup,   SLOT(setEnabled(bool)));
  }

  _code->setEnabled(editing);
  _number->setEnabled(editing);
  _name->setEnabled(editing);
  _startDate->setEnabled(editing);
  _active->setEnabled(editing);
  _contact->setEnabled(editing);
  _site->setEnabled(editing);
  _mgr->setEnabled(editing);
  _wagetype->setEnabled(editing);
  _rate->setEnabled(editing);
  _per->setEnabled(editing);
  _externalRate->setEnabled(editing);
  _perExt->setEnabled(editing);
  _dept->setEnabled(editing);
  _shift->setEnabled(editing);
  _notes->setEnabled(editing);
  _image->setEnabled(editing);
  _comments->setEnabled(editing);
  _save->setEnabled(editing);
  _newCharass->setEnabled(editing);

  _origmode = _mode;
  if (DEBUG)
    qDebug("employee::set() returning with _mode %d and _origmode %d",
           _mode, _origmode);

  return NoError;
}

bool employee::sSave(const bool pClose)
{
  bool dupCode   = false;
  bool dupNumber = false;

  XSqlQuery dupq;
  dupq.prepare("SELECT emp_id"
               "  FROM emp"
               " WHERE(emp_code=:code) AND (emp_id != :id);");
  dupq.bindValue(":code", _code->text());
  dupq.bindValue(":id",   _empid);
  dupq.exec();
  if(dupq.first())
    dupCode = true;
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                dupq, __FILE__, __LINE__))
    return false;

  dupq.prepare("SELECT emp_id"
               "  FROM emp"
               " WHERE(emp_number=:number) AND (emp_id != :id);");
  dupq.bindValue(":number", _number->text());
  dupq.bindValue(":id",     _empid);
  dupq.exec();
  if(dupq.first())
    dupNumber = true;
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Database Error"),
                                dupq, __FILE__, __LINE__))
    return false;

  QList<GuiErrorCheck> errors;
  errors << GuiErrorCheck(_code->text().isEmpty(), _code,
                          tr("You must enter a valid Employee Code."))
         << GuiErrorCheck(dupCode, _code,
                          tr("An Employee already exists for the Code specified."))
         << GuiErrorCheck(_code->text() == _mgr->number(), _number,
                          tr("An Employee already exists for the Number specified."))
         << GuiErrorCheck(_code->text() == _mgr->number(), _mgr,
                          tr("An Employee cannot be his or her own Manager."))
    ;
  if (GuiErrorCheck::reportErrors(this, tr("Cannot Save Employee"), errors))
    return false;

  _contact->check();

  XSqlQuery upsq;
  if (_mode == cNew)
    upsq.prepare("INSERT INTO emp ("
                 " emp_code,        emp_number,   emp_active,       emp_cntct_id,"
                 " emp_warehous_id, emp_mgr_emp_id,"
                 " emp_wage_type,   emp_wage,     emp_wage_curr_id, emp_wage_period,"
                 " emp_dept_id,     emp_shift_id, emp_notes,        emp_image_id,"
                 " emp_extrate,     emp_extrate_period, emp_startdate, emp_name"
                 ") VALUES ("
                 " :code,        :number,   :active,       :cntct_id,"
                 " :warehous_id, :mgr_emp_id,"
                 " :wage_type,   :wage,     :wage_curr_id, :wage_period,"
                 " :dept_id,     :shift_id, :notes,        :image_id,"
                 " :extrate,     :extrate_period, :startdate, :name"
                 ") RETURNING emp_id;");

  else if (_mode == cEdit)
  {
    upsq.prepare("UPDATE emp SET"
                 " emp_code=:code,"
                 " emp_number=:number,"
                 " emp_active=:active,"
                 " emp_cntct_id=:cntct_id,"
                 " emp_warehous_id=:warehous_id,"
                 " emp_mgr_emp_id=:mgr_emp_id,"
                 " emp_wage_type=:wage_type,"
                 " emp_wage=:wage,"
                 " emp_wage_curr_id=:wage_curr_id,"
                 " emp_wage_period=:wage_period,"
                 " emp_dept_id=:dept_id,"
                 " emp_shift_id=:shift_id,"
                 " emp_notes=:notes,"
                 " emp_image_id=:image_id,"
                 " emp_extrate=:extrate,"
                 " emp_extrate_period=:extrate_period,"
                 " emp_startdate=:startdate,"
                 " emp_name=:name"
              " WHERE (emp_id=:emp_id)"
              " RETURNING emp_id;" );
    upsq.bindValue(":emp_id", _empid);
  }

  upsq.bindValue(":code",           _code->text());
  upsq.bindValue(":number",         _number->text());
  upsq.bindValue(":active",         _active->isChecked());
  if (_contact->isValid())
    upsq.bindValue(":cntct_id",     _contact->id());
  if (_site->isValid())
    upsq.bindValue(":warehous_id",  _site->id());
  if (_mgr->isValid())
    upsq.bindValue(":mgr_emp_id",   _mgr->id());
  upsq.bindValue(":wage_type",      _wagetype->code());
  upsq.bindValue(":wage",           _rate->localValue());
  if (_rate->id() > 0)
    upsq.bindValue(":wage_curr_id", _rate->id());
  upsq.bindValue(":wage_period",    _per->code());
  if (_dept->isValid())
    upsq.bindValue(":dept_id",      _dept->id());
  if (_shift->isValid())
    upsq.bindValue(":shift_id",     _shift->id());
  upsq.bindValue(":notes",          _notes->toPlainText());
  if (_image->isValid())
    upsq.bindValue(":image_id",     _image->id());
  upsq.bindValue(":extrate",        _externalRate->localValue());
  upsq.bindValue(":extrate_period", _perExt->code());
  upsq.bindValue(":startdate",      _startDate->date());
  upsq.bindValue(":name",           _name->text());

  upsq.exec();
  if (upsq.first())
    _empid = upsq.value("emp_id").toInt();
  else if (ErrorReporter::error(QtCriticalMsg, this, tr("Error saving Employee"),
                                upsq, __FILE__, __LINE__))
    return false;

  emit saved();
  omfgThis->sEmployeeUpdated(_empid);

  if (pClose)
    done(_empid);
  else
    sPopulate();

  return true;
}

void employee::reject()
{
  if (DEBUG)
    qDebug("employee::reject() entered with _mode %d and _origmode %d",
           _mode, _origmode);
  if (_origmode == cNew)
  {
    XSqlQuery delq;
    delq.prepare("DELETE FROM emp WHERE (emp_id=:empid);");
    delq.bindValue(":empid", _empid);
    delq.exec();
    ErrorReporter::error(QtCriticalMsg, this, tr("Error deleting Employee"),
                         delq, __FILE__, __LINE__);
  }

  XDialog::reject();
}

bool employee::sPopulate()
{
  XSqlQuery getq;
  if (_empid > 0)
  {
    getq.prepare("SELECT emp.*, crmacct_id, crmacct_salesrep_id,"
                 "       crmacct_usr_username, crmacct_vend_id"
                 "  FROM emp JOIN crmacct ON (emp_id=crmacct_emp_id)"
                 " WHERE (emp_id=:id);");
    getq.bindValue(":id", _empid);
  }
  else if (_crmacctid > 0)
  {
    getq.prepare("SELECT crmacct_number     AS emp_code, NULL AS emp_number,"
                 "       crmacct_name       AS emp_name,"
                 "       crmacct_active     AS emp_active,"
                 "       crmacct_cntct_id_1 AS emp_cntct_id,"
                 "       NULL AS emp_startdate,      NULL AS emp_mgr_emp_id,"
                 "       NULL AS emp_warehous_id,    NULL AS emp_wage_type,"
                 "       NULL AS emp_wage,           NULL AS emp_wage_curr_id,"
                 "       NULL AS emp_extrate,        NULL AS emp_wage_period,"
                 "       NULL AS emp_extrate_period, NULL AS emp_dept_id,"
                 "       NULL AS emp_shift_id,       NULL AS emp_notes,"
                 "       NULL AS emp_image_id,       crmacct_id,"
                 "       crmacct_salesrep_id,        crmacct_usr_username,"
                 "       crmacct_vend_id"
                 "  FROM crmacct"
                 " WHERE (crmacct_id=:id);");
    getq.bindValue(":id", _crmacctid);
  }

  getq.exec();
  if (getq.first())
  {
    _code->setText(getq.value("emp_code").toString());
    _name->setText(getq.value("emp_name").toString());
    _number->setText(getq.value("emp_number").toString());
    _active->setChecked(getq.value("emp_active").toBool());
    _startDate->setDate(getq.value("emp_startdate").toDate());
    _contact->setId(getq.value("emp_cntct_id").toInt());
    _site->setId(getq.value("emp_warehous_id").toInt());
    _mgr->setId(getq.value("emp_mgr_emp_id").toInt());
    _wagetype->setCode(getq.value("emp_wage_type").toString());
    _rate->set(getq.value("emp_wage").toDouble(),
               getq.value("emp_wage_curr_id").toInt(),
               QDate::currentDate());
    _externalRate->set(getq.value("emp_extrate").toDouble(),
                       getq.value("emp_wage_curr_id").toInt(),
                       QDate::currentDate());
    _per->setCode(getq.value("emp_wage_period").toString());
    _perExt->setCode(getq.value("emp_extrate_period").toString());
    _dept->setId(getq.value("emp_dept_id").toInt());
    _shift->setId(getq.value("emp_shift_id").toInt());
    _notes->setText(getq.value("emp_notes").toString());
    _image->setId(getq.value("emp_image_id").toInt());

    _crmacctid  = getq.value("crmacct_id").toInt();
    _salesrepid = getq.value("crmacct_salesrep_id").toInt();
    _username   = getq.value("crmacct_usr_username").toString();
    _vendid     = getq.value("crmacct_vend_id").toInt();

    _salesrep->setChecked(_salesrepid > 0);
    _salesrep->setEnabled(_privileges->check("MaintainSalesReps") &&
                          ! _salesrep->isChecked());
    _salesrepButton->setEnabled((_privileges->check("MaintainSalesReps") ||
                                 _privileges->check("ViewSalesReps")) &&
                                _salesrep->isChecked());
    _user->setChecked(! _username.isEmpty());
    _user->setEnabled(_privileges->check("Maintainusers") &&
                          ! _user->isChecked());
    _userButton->setEnabled((_privileges->check("MaintainUsers") ||
                             _privileges->check("ViewUsers")) &&
                            _user->isChecked());
    _vendor->setChecked(_vendid > 0);
    _vendor->setEnabled(_privileges->check("MaintainVendors") &&
                        ! _vendor->isChecked());
    _vendorButton->setEnabled((_privileges->check("MaintainVendors") ||
                               _privileges->check("ViewVendors")) &&
                              _vendor->isChecked());

    if (DEBUG)
      qDebug("image %s and %s",
             qPrintable(getq.value("image").toString()),
             qPrintable(_image->number()));

    sFillCharassList();
    sFillGroupsList();
    _comments->setId(_empid);
    _comments->setReadOnly(_empid==-1);
    emit populated();
  }
  else if (ErrorReporter::error(QtCriticalMsg, this,
                                tr("Error getting Employee"),
                                getq, __FILE__, __LINE__))
    return false;

  sHandleButtons();
  return true;
}

void employee::sDeleteCharass()
{
  XSqlQuery delq;
  delq.prepare("DELETE FROM charass WHERE (charass_id=:charass_id);");
  delq.bindValue(":charass_id", _charass->id());
  delq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error deleting Characteristic Assignment"),
                           delq, __FILE__, __LINE__))
    return;

  sFillCharassList();
}

void employee::sEditCharass()
{
  ParameterList params;
  params.append("mode",       "edit");
  params.append("charass_id", _charass->id());

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharassList();
}

void employee::sNewCharass()
{
  if (_mode == cNew)
  {
    if (!sSave(false))
      return;
  }
  ParameterList params;
  params.append("mode",   "new");
  params.append("emp_id", _empid);

  characteristicAssignment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillCharassList();
}

void employee::sFillCharassList()
{
  XSqlQuery getq;
  getq.prepare( "SELECT charass_id, char_name, "
             " CASE WHEN char_type < 2 THEN "
             "   charass_value "
             " ELSE "
             "   formatDate(charass_value::date) "
             "END AS charass_value "
             "FROM charass, char "
             "WHERE ((charass_target_type='EMP')"
             " AND   (charass_char_id=char_id)"
             " AND   (charass_target_id=:emp_id) ) "
             "ORDER BY char_order, char_name;" );
  getq.bindValue(":emp_id", _empid);
  getq.exec();
  _charass->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error getting Characteristic Assignments"),
                           getq, __FILE__, __LINE__))
    return;
}

void employee::sFillGroupsList()
{
  XSqlQuery getq;
  getq.prepare( "SELECT empgrp.* "
             "FROM empgrp, empgrpitem "
             "WHERE ((empgrp_id=empgrpitem_empgrp_id)"
             "  AND  (empgrpitem_emp_id=:emp_id) ) "
             "ORDER BY empgrp_name;" );
  getq.bindValue(":emp_id", _empid);
  getq.exec();
  _groups->populate(getq);
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error getting Employee Groups"),
                           getq, __FILE__, __LINE__))
    return;
}

void employee::sSalesrep()
{
  if (cEdit == _mode || cNew == _mode)
    if (!sSave(false))
      return;

  ParameterList params;

  if (_salesrepid < 0)
  {
    if (cView == _mode && _privileges->check("ViewSalesReps"))
    {
      QMessageBox::information(this, tr("No Sales Rep"),
                               tr("<p>There does not appear to be a Sales "
                                  "Rep associated with this Employee."));
      return;
    }
    if (_privileges->check("MaintainSalesReps") &&
        QMessageBox::question(this, tr("Create Sales Rep?"),
                              tr("<p>There does not appear to be a Sales "
                                 "Rep associated with this Employee. "
                                 "Would you like to create a new Sales Rep?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::No)
      return;

    params.append("crmacct_id", _crmacctid);
    params.append("new");
  }
  else if (_privileges->check("MaintainSalesReps"))
  {
    params.append("salesrep_id", _salesrepid);
    params.append("mode", "edit");
  }
  else if (_privileges->check("ViewSalesReps"))
  {
    params.append("salesrep_id", _salesrepid);
    params.append("mode", "view");
  }

  salesRep newdlg(this, "", TRUE);
  newdlg.set(params);
  if (newdlg.exec() == QDialog::Accepted)
    sPopulate();
}

void employee::sVendor()
{
  if (cEdit == _mode || cNew == _mode)
    if (!sSave(false))
      return;

  ParameterList params;

  if (_vendid < 0)
  {
    if (cView == _mode && _privileges->check("ViewVendors"))
    {
      QMessageBox::information(this, tr("No Vendor"),
                               tr("<p>There does not appear to be a Vendor "
                                  "associated with this Employee."));
      return;
    }
    if (_privileges->check("MaintainVendors") &&
        QMessageBox::question(this, tr("Create Vendor?"),
                              tr("<p>There does not appear to be a Vendor "
                                 "associated with this Employee. "
                                 "Would you like to create a new Vendor?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::No)
      return;

    params.append("crmacct_id", _crmacctid);
    params.append("mode",     "new");
  }
  else if (_privileges->check("MaintainVendors"))
  {
    params.append("vend_id", _vendid);
    params.append("mode", "edit");
  }
  else if (_privileges->check("ViewVendors"))
  {
    params.append("vend_id", _vendid);
    params.append("mode", "view");
  }

  vendor *newdlg = new vendor(this);
  newdlg->set(params);
  omfgThis->handleNewWindow(newdlg);
}

void employee::sUser()
{
  if (cEdit == _mode || cNew == _mode)
    if (!sSave(false))
      return;

  ParameterList params;

  if (_username.isEmpty())
  {
    if (cView == _mode && _privileges->check("ViewUsers"))
    {
      QMessageBox::information(this, tr("No User"),
                               tr("<p>There does not appear to be a User "
                                  "associated with this Employee."));
      return;
    }
    if (_privileges->check("MaintainUsers") &&
        QMessageBox::question(this, tr("Create User?"),
                              tr("<p>There does not appear to be a User "
                                 "associated with this Employee. "
                                 "Would you like to create a new User?"),
                              QMessageBox::Yes | QMessageBox::Default,
                              QMessageBox::No) == QMessageBox::No)
      return;

    params.append("crmacct_id", _crmacctid);
    params.append("mode",     "new");
  }
  else if (_privileges->check("MaintainUsers"))
  {
    params.append("username", _username);
    params.append("mode",     "edit");
  }
  else if (_privileges->check("ViewUsers"))
  {
    params.append("username", _username);
    params.append("mode", "view");
  }

  user *newdlg = new user(this);
  newdlg->set(params);
  if (newdlg->exec() == QDialog::Accepted)
    sPopulate();
}

void employee::sAttachGroup()
{
  if (!sSave(false))
    return;

  int empgrpid = EmpGroupClusterLineEdit::idFromList(this);
  if (empgrpid != XDialog::Rejected && empgrpid != -1)
  {
    XSqlQuery grpq;
    grpq.prepare("SELECT * FROM empgrpitem"
              " WHERE((empgrpitem_empgrp_id=:empgrpid)"
              "   AND (empgrpitem_emp_id=:empid));");
    grpq.bindValue(":empgrpid", empgrpid);
    grpq.bindValue(":empid",    _empid);
    grpq.exec();
    if(grpq.first())
    {
      QMessageBox::information(this, tr("Employee in Group"),
        tr("The employee is already in the selected group.") );
      return;
    }
    grpq.prepare("INSERT INTO empgrpitem (empgrpitem_empgrp_id, empgrpitem_emp_id)"
              " VALUES "
              "(:empgrpid, :empid);");
    grpq.bindValue(":empgrpid", empgrpid);
    grpq.bindValue(":empid",    _empid);
    grpq.exec();
    if (ErrorReporter::error(QtCriticalMsg, this,
                             tr("Error attaching to Employee Group"),
                             grpq, __FILE__, __LINE__))
      return;
    sFillGroupsList();
  }
}

void employee::sDetachGroup()
{
  XSqlQuery detq;
  detq.prepare("DELETE FROM empgrpitem "
            "WHERE ((empgrpitem_empgrp_id=:grpid)"
            "  AND  (empgrpitem_emp_id=:empid));");
  detq.bindValue(":grpid", _groups->id());
  detq.bindValue(":empid", _empid);
  detq.exec();
  if (ErrorReporter::error(QtCriticalMsg, this,
                           tr("Error detaching from Employee Group"),
                           detq, __FILE__, __LINE__))
    return;
  sFillGroupsList();
}

void employee::sEditGroup()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("empgrp_id", _groups->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
  sFillGroupsList();
}

void employee::sViewGroup()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("empgrp_id", _groups->id());

  empGroup newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void employee::sHandleButtons()
{
  _salesrep->setChecked(_salesrepid > 0);
  _salesrepButton->setEnabled(_salesrepid > 0 &&
                              (_privileges->check("MaintainSalesReps") ||
                               _privileges->check("ViewSalesReps")));

  _user->setChecked(! _username.isEmpty());
  _userButton->setEnabled(! _username.isEmpty() &&
                          (_privileges->check("MaintainUsers") ||
                           _privileges->check("ViewUsers")));

  _vendor->setChecked(_vendid > 0);
  _vendorButton->setEnabled(_vendid > 0 &&
                            (_privileges->check("MaintainVendors") ||
                             _privileges->check("ViewVendors")));
}

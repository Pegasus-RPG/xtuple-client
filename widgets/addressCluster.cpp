/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

// AddressCluster.cpp
// Created 8/05/2006 GJM
// Copyright (c) 2006-2007, OpenMFG, LLC

#include <QMessageBox>
#include <QGridLayout>
#include <QSqlError>

#include <metasql.h>

#include "xsqlquery.h"

#include "addresscluster.h"

void AddressCluster::init()
{
    _titleSingular	= tr("Address");
    _titlePlural	= tr("Addresses");
    _query = "SELECT * FROM addr ";

    // handle differences between VirtualCluster and AddressCluster
    _grid->removeWidget(_label);
    _grid->removeWidget(_description);
    _grid->removeWidget(_list);
    _grid->removeWidget(_info);
    delete _description;

    _addrLit		= new QLabel(tr("Street\nAddress:"), this);
    _addr1		= new QLineEdit(this);
    _addr2		= new QLineEdit(this);
    _addr3		= new QLineEdit(this);
    _cityLit		= new QLabel(tr("City:"), this);
    _city		= new QLineEdit(this);
    _stateLit		= new QLabel(tr("State:"));
    _state		= new QComboBox(this);
    _postalcodeLit	= new QLabel(tr("Postal Code:"));
    _postalcode		= new QLineEdit(this);
    _countryLit		= new QLabel(tr("Country:"));
    _country		= new QComboBox(this);
    _active		= new QCheckBox(tr("Active"), this);

    _addrLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _cityLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _stateLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _state->setEditable(true);
    _country->setEditable(true);
    _country->setMaximumWidth(250);
    _postalcodeLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _countryLit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _grid->setMargin(0);
    _grid->setSpacing(2);
    _grid->addWidget(_label,	0, 0, 1, -1);
    _grid->addWidget(_addrLit,  1, 0, 3, 1);
    _grid->addWidget(_addr1,	1, 1, 1, -1);
    _grid->addWidget(_addr2,	2, 1, 1, -1);
    _grid->addWidget(_addr3,	3, 1, 1, -1);
    _grid->addWidget(_cityLit,	4, 0);
    _grid->addWidget(_city,	4, 1);
    _grid->addWidget(_stateLit,	4, 2);
    _grid->addWidget(_state,	4, 3);
    _grid->addWidget(_postalcodeLit,	4, 4);
    _grid->addWidget(_postalcode,	4, 5, 1, 2);
    _grid->addWidget(_countryLit,	5, 0);
    _grid->addWidget(_country,		5, 1, 1, 3);
    _grid->addWidget(_active,		5, 4);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(_list);
    hbox->addWidget(_info);
    _grid->addLayout(hbox, 5, 5, 1, -1, Qt::AlignRight);

    _grid->setColumnStretch(0, 0);
    _grid->setColumnStretch(1, 3);
    _grid->setColumnStretch(2, 0);
    _grid->setColumnStretch(3, 1);
    _grid->setColumnStretch(4, 0);
    _grid->setColumnStretch(5, 2);

    connect(_list,	SIGNAL(clicked()),	this, SLOT(sEllipses()));
    connect(_info,	SIGNAL(clicked()),	this, SLOT(sInfo()));
    connect(_addr1,	SIGNAL(textChanged(const QString&)),	this, SIGNAL(changed()));
    connect(_addr2,	SIGNAL(textChanged(const QString&)),	this, SIGNAL(changed()));
    connect(_addr3,	SIGNAL(textChanged(const QString&)),	this, SIGNAL(changed()));
    connect(_city,	SIGNAL(textChanged(const QString&)),	this, SIGNAL(changed()));
    connect(_state,	SIGNAL(editTextChanged(const QString&)),this, SIGNAL(changed()));
    connect(_postalcode,SIGNAL(textChanged(const QString&)),	this, SIGNAL(changed()));
    connect(_country,	SIGNAL(editTextChanged(const QString&)),this, SIGNAL(changed()));

    setFocusProxy(_addr1);
    setFocusPolicy(Qt::StrongFocus);
    setLabel("");
    setActiveVisible(false);
    setInfoVisible(false);	// TODO - remove this and implement Info button
    silentSetId(-1);
}

AddressCluster::AddressCluster(QWidget* pParent, const char* pName) :
    VirtualCluster(pParent, pName)
{
    init();
}

void AddressCluster::populateStateComboBox()
{
  _state->clear();
  XSqlQuery state;
  state.prepare("SELECT DISTINCT addr_state AS state "
		"FROM addr ORDER BY state;");
  state.exec();
  while (state.next())
    _state->addItem(state.value("state").toString());
}

void AddressCluster::populateCountryComboBox()
{
  _country->clear();
  XSqlQuery country;
  country.prepare("SELECT country_name "
		  "FROM country ORDER BY country_name;");
  country.exec();
  while (country.next())
    _country->addItem(country.value("country_name").toString());
}

void AddressCluster::setId(const int pId)
{
    if (pId == _id)
	return;
    else
    {
	silentSetId(pId);
	emit newId(pId);
    }
}

void AddressCluster::silentSetId(const int pId)
{
    if (pId == -1)
    {
      _id = pId;
      _valid = false;
      clear();
    }
    else if (pId == _id)
      return;
    else
    {
	clear();
	XSqlQuery idQ;
	idQ.prepare(_query + " WHERE addr_id = :id;");
	idQ.bindValue(":id", pId);
	idQ.exec();
	if (idQ.first())
	{
	    _id = pId;
	    _valid = true;
	    _addr1->setText(idQ.value("addr_line1").toString());
	    _addr2->setText(idQ.value("addr_line2").toString());
	    _addr3->setText(idQ.value("addr_line3").toString());
	    _city->setText(idQ.value("addr_city").toString());
	    _postalcode->setText(idQ.value("addr_postalcode").toString());
	    _state->setEditText(idQ.value("addr_state").toString());
	    _country->setEditText(idQ.value("addr_country").toString());
	    _active->setChecked(idQ.value("addr_active").toBool());
	    _notes = idQ.value("addr_notes").toString();

	    c_addr1	= _addr1->text();
	    c_addr2	= _addr2->text();
	    c_addr3	= _addr3->text();
	    c_city	= _city->text();
	    c_state	= _state->currentText();
	    c_postalcode = _postalcode->text();
	    c_country	= _country->currentText();
	    c_active	= _active->isChecked();
	    c_notes	= _notes;
	}
	else if (idQ.lastError().type() != QSqlError::None)
	    QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
					  .arg(__FILE__)
					  .arg(__LINE__),
				  idQ.lastError().databaseText());
    }

    // _parsed = TRUE;
}

void AddressCluster::clear()
{
  _id = -1;
  _valid = false;

  populateStateComboBox();
  populateCountryComboBox();

  // reset cache
  c_active	= true;
  c_addr1	= "";
  c_addr2	= "";
  c_addr3	= "";
  c_city	= "";
  c_state	= "";
  c_postalcode	= "";
  c_country	= "";
  c_notes	= "";

  _addr1->clear();
  _addr2->clear();
  _addr3->clear();
  _city->clear();
  _state->clearEditText();
  _postalcode->clear();
  _country->clearEditText();
  _active->setChecked(c_active);
  _selected = false;
}

/* this should probably be a simple call to a saveAddr stored procedure
   with the logic moved from here to that stored procedure
   return +N addr_id if save is successful or if found another addr with the same info
   return  0 if there is no address to save
   return -1 if there was an error
   return -2 if there are N contacts sharing this address
 */
int AddressCluster::save(enum SaveFlags flag)
{
  QString new_addr1	= _addr1->text();
  QString new_addr2	= _addr2->text();
  QString new_addr3	= _addr3->text();
  QString new_city	= _city->text();
  QString new_state	= _state->currentText();
  QString new_postalcode = _postalcode->text();
  QString new_country	= _country->currentText();
  bool	  new_active	= _active->isChecked();
  QString new_notes	= _notes;

  if (new_addr1 == "" && new_addr2 == "" &&
      new_addr3 == "" && new_city == "" &&
      new_state == "" && new_postalcode == "" &&
      new_country == "")
  {
    silentSetId(-1);
    return 0;
  }

  // if nothing changed, there's nothing to do
  if (c_addr1 == new_addr1 && c_addr2 == new_addr2 &&
      c_addr3 == new_addr3 && c_city == new_city &&
      c_state == new_state && c_postalcode == new_postalcode &&
      c_country == new_country && c_active == new_active &&
      c_notes == new_notes)
    return id();

  // look for existing records that match the new values
  XSqlQuery dupcheck;
  dupcheck.prepare("SELECT addr_id, addr_notes "
		   "FROM addr "
		   "WHERE ((addr_id <> :addr_id)"
		   "  AND  (UPPER(addr_line1) = UPPER(:addr1))"
		   "  AND  (UPPER(addr_line2) = UPPER(:addr2))"
		   "  AND  (UPPER(addr_line3) = UPPER(:addr3))"
		   "  AND  (UPPER(addr_city) = UPPER(:city))"
		   "  AND  (UPPER(addr_state) = UPPER(:state))"
		   "  AND  (UPPER(addr_postalcode) = UPPER(:postalcode))"
		   "  AND  (UPPER(addr_country) = UPPER(:country)));");
  dupcheck.bindValue(":addr_id", id());
  dupcheck.bindValue(":addr1", new_addr1);
  dupcheck.bindValue(":addr2", new_addr2);
  dupcheck.bindValue(":addr3", new_addr3);
  dupcheck.bindValue(":city", new_city);
  dupcheck.bindValue(":state", new_state);
  dupcheck.bindValue(":postalcode", new_postalcode);
  dupcheck.bindValue(":country", new_country);
  dupcheck.exec();
  if (dupcheck.first())
  {
    int dupId = dupcheck.value("addr_id").toInt();
    QString dupNotes = dupcheck.value("addr_notes").toString();
    if (dupNotes != _notes)
    {
      _notes = dupNotes.append("\n").append(_notes);
      XSqlQuery writeNotes;
      writeNotes.prepare("UPDATE addr SET addr_notes = :notes "
			 "WHERE addr_id = :addr_id;");
      writeNotes.bindValue(":notes", _notes);
      writeNotes.bindValue(":addr_id", dupId);
      if (! writeNotes.exec())
	return -1;	// error
    }

    silentSetId(dupId);
    return dupId;	// a matching addr is already in the db
  }
  else if (dupcheck.lastError().type() != QSqlError::None)
    return -1;	// error


  if (id() <= 0) // this is a new address
    flag = CHANGEONE;

  if (flag == CHECK)
  {
    int numUses = 0;
    XSqlQuery numUsesQ;
    numUsesQ.prepare("SELECT COUNT(*) AS cnt FROM ("
		     "   SELECT cntct_id, 'cntct' FROM cntct "
		     "		WHERE cntct_addr_id = :addr_id "
		     "   UNION "
		     "   SELECT vend_id, 'vendinfo' FROM vendinfo"
		     "		WHERE vend_addr_id = :addr_id "
		     "   UNION "
		     "   SELECT shipto_id, 'shiptoinfo' FROM shiptoinfo"
		     "		WHERE shipto_addr_id = :addr_id "
		     "   UNION "
		     "   SELECT vendaddr_id, 'vendaddrinfo' FROM vendaddrinfo"
		     "		WHERE vendaddr_addr_id = :addr_id "
		     "   UNION "
		     "   SELECT warehous_id, 'whsinfo' FROM whsinfo"
		     "		WHERE warehous_addr_id = :addr_id) AS useQ;");
    numUsesQ.bindValue(":addr_id", id());
    numUsesQ.exec();
    if (numUsesQ.first())
      numUses = numUsesQ.value("cnt").toInt();
    else
      return -1;	// error

    if (numUses > 1)
      return -2;	// ask user if s/he wants to overwrite or save new
    else if (1 == numUses && _selected)
      flag = CHANGEONE;	// maybe the user copied an address to change one part
    else		// this will be caught when purging if not earlier
      flag = CHANGEALL;
  }

  QString   sql;
  switch (flag)
  {
    case CHANGEALL:
      sql = "UPDATE addr SET "
	    "    addr_line1 = :addr1, addr_line2 = :addr2, addr_line3 = :addr3, "
	    "    addr_city = :city, addr_state = :state, "
	    "    addr_postalcode = :postalcode, addr_country = :country, "
	    "    addr_active = :active, addr_notes = :notes "
	    "WHERE addr_id = :addr_id;";
      break;
    case CHANGEONE:
      sql = "INSERT INTO addr ("
	    "  addr_line1, addr_line2, addr_line3, "
	    "  addr_city, addr_state, addr_postalcode, addr_country, "
	    "  addr_active, addr_notes "
	    ") VALUES ("
	    "  :addr1, :addr2, :addr3, "
	    "  :city, :state, :postalcode, :country, "
	    "  :active, :notes);";
      break;
    default:
      return -1;	// error
  }

  XSqlQuery datamodQ;
  datamodQ.prepare(sql);
  datamodQ.bindValue(":addr_id", id());
  datamodQ.bindValue(":addr1", _addr1->text());
  datamodQ.bindValue(":addr2", _addr2->text());
  datamodQ.bindValue(":addr3", _addr3->text());
  datamodQ.bindValue(":city", _city->text());
  datamodQ.bindValue(":state", _state->currentText());
  datamodQ.bindValue(":postalcode", _postalcode->text());
  datamodQ.bindValue(":country", _country->currentText());
  datamodQ.bindValue(":active", QVariant(_active->isChecked(), 0));
  datamodQ.bindValue(":notes", _notes);
  if (!datamodQ.exec())
    return -1;		// error
  else if (flag == CHANGEONE)
  {
    datamodQ.exec("SELECT CURRVAL('addr_addr_id_seq') AS id;");
    if (datamodQ.first())
      _id = datamodQ.value("id").toInt();
    else
      return -1;	// error
  }
  _selected = FALSE;
  _valid = true;
  return id();
}

void AddressCluster::sEllipses()
{
    if (_x_preferences && _x_preferences->value("DefaultEllipsesAction") == "search")
	sSearch();
    else
	sList();
}

void AddressCluster::sInfo()
{
    /*
    AddressInfo* newdlg = new AddressInfo(this, "AddressInfo", true));
    if (newdlg)
    {
	int id = newdlg->exec();
	setId(id);
    }
    else
    */
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("%1::sInfo() not yet defined").arg(className()));
}

void AddressCluster::sList()
{
    AddressList* newdlg = new AddressList(this, "AddressList", true);
    if (newdlg)
    {
	int id = newdlg->exec();
	if (id != QDialog::Rejected)
	{
	  setId(id);
	  _selected = true;
	}
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("Could not instantiate a List Dialog"));
}

void AddressCluster::sSearch()
{
    AddressSearch* newdlg = new AddressSearch(this);
    if (newdlg)
    {
	int id = newdlg->exec();
	if (id != QDialog::Rejected)
	{
	  setId(id);
	  _selected = true;
	}
    }
    else
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      tr("Could not instantiate a Search Dialog"));
}

void AddressCluster::setActiveVisible(const bool p)
{
  _active->setHidden(!p);
  if (p)
  {
    _grid->removeWidget(_country);
    _grid->addWidget(_active,	5, 4);
    _grid->addWidget(_country,	5, 1, 1, 3);
  }
  else
  {
    _grid->removeWidget(_active);
    _grid->removeWidget(_country);
    _grid->addWidget(_country,	5, 1, 1, 4);
  }
}

///////////////////////////////////////////////////////////////////////////

AddressList::AddressList(QWidget* pParent, const char* pName, bool, Qt::WFlags)
{
    _parent = (AddressCluster*)(pParent);
    if (!pName)
	setName("AddressList");
    setCaption(_parent->_titlePlural);

    _listTab->addColumn(tr("Line 1"),      -1, Qt::AlignLeft );
    _listTab->addColumn(tr("Line 2"),      75, Qt::AlignLeft );
    _listTab->addColumn(tr("Line 3"),      75, Qt::AlignLeft );
    _listTab->addColumn(tr("City"),        75, Qt::AlignLeft );
    _listTab->addColumn(tr("State"),       50, Qt::AlignLeft );
    _listTab->addColumn(tr("Country"),     50, Qt::AlignLeft );
    _listTab->addColumn(tr("Postal Code"), 50, Qt::AlignLeft );

    resize(700, size().height());

    sFillList();
}

void AddressList::sFillList()
{
    _listTab->clear();
    XSqlQuery query;
    query.prepare(_parent->_query +
		  _parent->_extraClause +
		  " ORDER BY addr_country, addr_state, addr_postalcode;");
    query.exec();
    query.first();
    if (query.lastError().type() != QSqlError::None)
    {
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
			    query.lastError().databaseText());
      return;
    }
    else if (query.size() < 1)	// no rows found with limit so try without
    {
      query.prepare(_parent->_query +
		  " ORDER BY addr_country, addr_state, addr_postalcode;");
      query.exec();
      query.first();
      if (query.lastError().type() != QSqlError::None)
      {
	QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				      .arg(__FILE__)
				      .arg(__LINE__),
			      query.lastError().databaseText());
	return;
      }
    }

    XTreeWidgetItem *last = NULL;
    do {
      last = new XTreeWidgetItem(_listTab, last,
			       query.value("addr_id").toInt(), 0,
			       query.value("addr_line1"),
			       query.value("addr_line2"),
			       query.value("addr_line3"),
			       query.value("addr_city"),
			       query.value("addr_state"),
			       query.value("addr_country"),
			       query.value("addr_postalcode"));
    } while (query.next());
}

/* do this differently than VirtualList::sSearch(QString&):
   look for any consecutive characters that match, not just the
   first field
 */
void AddressList::sSearch(const QString& pTarget)
{

  QTreeWidgetItem *target = 0;
  for (int i = 0; i < _listTab->topLevelItemCount(); i++)
  {
    target = _listTab->topLevelItem(i);
    if (target == NULL ||
	(target->text(0) + " " + target->text(1) + " " +
	 target->text(2) + " " + target->text(3) + " " +
	 target->text(4) + " " + target->text(5)).contains(pTarget.upper(),
							   Qt::CaseInsensitive))
      break;
  }

  if (target)
  {
    _listTab->setCurrentItem(target);
    _listTab->scrollToItem(target);
  }
}

///////////////////////////////////////////////////////////////////////////

AddressSearch::AddressSearch(QWidget* pParent, Qt::WindowFlags pFlags)
    : VirtualSearch(pParent, pFlags)
{
    // remove the stuff we won't use
    disconnect(_searchNumber,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    disconnect(_searchName,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    disconnect(_searchDescrip,	SIGNAL(toggled(bool)), this, SLOT(sFillList()));

    selectorsLyt->removeWidget(_searchNumber);
    selectorsLyt->removeWidget(_searchName);
    selectorsLyt->removeWidget(_searchNumber);

    delete _searchNumber;
    delete _searchName;
    delete _searchDescrip;

    _listTab->setColumnCount(0);

    _searchStreet	= new QCheckBox(tr("Search Street Address"));
    _searchCity		= new QCheckBox(tr("Search City"));
    _searchState	= new QCheckBox(tr("Search State"));
    _searchCountry	= new QCheckBox(tr("Search Country"));
    _searchPostalCode	= new QCheckBox(tr("Search Postal Code"));
    _searchInactive	= new QCheckBox(tr("Show Inactive Addresses"));

    selectorsLyt->addWidget(_searchStreet,     0, 0);
    selectorsLyt->addWidget(_searchCity,       1, 0);
    selectorsLyt->addWidget(_searchState,      2, 0);
    selectorsLyt->addWidget(_searchCountry,    3, 0);
    selectorsLyt->addWidget(_searchPostalCode, 4, 0);
    selectorsLyt->addWidget(_searchInactive,   0, 1);

    connect(_searchStreet,     SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchCity,       SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchState,      SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchCountry,    SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchPostalCode, SIGNAL(toggled(bool)), this, SLOT(sFillList()));
    connect(_searchInactive,   SIGNAL(toggled(bool)), this, SLOT(sFillList()));

    _listTab->addColumn(tr("Line 1"),      -1, Qt::AlignLeft );
    _listTab->addColumn(tr("Line 2"),      75, Qt::AlignLeft );
    _listTab->addColumn(tr("Line 3"),      75, Qt::AlignLeft );
    _listTab->addColumn(tr("City"),        75, Qt::AlignLeft );
    _listTab->addColumn(tr("State"),       50, Qt::AlignLeft );
    _listTab->addColumn(tr("Country"),     50, Qt::AlignLeft );
    _listTab->addColumn(tr("Postal Code"), 50, Qt::AlignLeft );

    resize(700, size().height());

    _parent = (AddressCluster*)(pParent);
    setObjectName("virtualSearch");
    setCaption(_parent->_titlePlural);
    _titleLit->setText(_parent->_titlePlural);
    _id = _parent->_id;
}

void AddressSearch::sFillList()
{
    _listTab->clear();
    if (_search->text().isEmpty() ||
	(!_searchStreet->isChecked() && !_searchCity->isChecked() &&
	 !_searchState->isChecked()  && !_searchCountry->isChecked() &&
	 !_searchPostalCode->isChecked() ))
      return;

    QString sql = _parent->_query +
		  "<? if exists(\"extraClause\") ?> "
		  "  <? value(\"extraClause\") ?> "
		  "<? else ?>"
		  "  WHERE "
		  "<? endif ?> "
		  "<? if exists(\"searchInactive\") ?> "
		  "   true "
		  "<? else ?>"
		  "   addr_active "
		  "<? endif ?>"
		  "<? if reExists(\"search[CPS]\") ?> "
		  "  AND ("
		  "  <? if exists(\"searchStreet\") ?> "
		  "    addr_line1 || '\n' || addr_line2 || '\n' || addr_line3 || '\n' "
		  "  <? else ?>"
		  "    '\n' "
		  "  <? endif ?>"
		  "  <? if exists(\"searchCity\") ?> "
		  "     || addr_city || '\n' "
		  "  <? endif ?>"
		  "  <? if exists(\"searchState\") ?> "
		  "     || addr_state || '\n' "
		  "  <? endif ?>"
		  "  <? if exists(\"searchCountry\") ?> "
		  "     || addr_country || '\n' "
		  "  <? endif ?>"
		  "  <? if exists(\"searchPostalCode\") ?> "
		  "     || addr_postalcode || '\n' "
		  "  <? endif ?>"
		  "  ~* <? value(\"searchText\") ?> )"
		  "<? endif ?>"
		  "ORDER BY addr_country, addr_state, addr_postalcode;";
    ParameterList params;
    if (_searchStreet->isChecked())
      params.append("searchStreet");
    if (_searchCity->isChecked())
      params.append("searchCity");
    if (_searchState->isChecked())
      params.append("searchState");
    if (_searchCountry->isChecked())
      params.append("searchCountry");
    if (_searchPostalCode->isChecked())
      params.append("searchPostalCode");
    if (_searchInactive->isChecked())
      params.append("searchInactive");
    if (! _parent->_extraClause.isEmpty())
      params.append("extraClause", _parent->_extraClause);

    params.append("searchText", _search->text());

    MetaSQLQuery mql(sql);
    XSqlQuery query = mql.toQuery(params);
    query.exec();
    XTreeWidgetItem *last = 0;
    while (query.next()) {
      last = new XTreeWidgetItem(_listTab, last,
			       query.value("addr_id").toInt(), 0,
			       query.value("addr_line1"),
			       query.value("addr_line2"),
			       query.value("addr_line3"),
			       query.value("addr_city"),
			       query.value("addr_state"),
			       query.value("addr_country"),
			       query.value("addr_postalcode"));
    }
    if (query.lastError().type() != QSqlError::None)
    {
      QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
				    .arg(__FILE__)
				    .arg(__LINE__),
			    query.lastError().databaseText());
      return;
    }

}

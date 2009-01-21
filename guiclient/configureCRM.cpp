/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureCRM.h"

#include <QVariant>
#include <QValidator>

#include "guiclient.h"

/*
 *  Constructs a configureCRM as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureCRM::configureCRM(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  _nextInNumber->setValidator(omfgThis->orderVal());
  _nextAcctNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT orderseq_number AS innumber "
          "  FROM orderseq"
          " WHERE (orderseq_name='IncidentNumber');" );
  if (q.first())
    _nextInNumber->setText(q.value("innumber"));
    
  q.exec( "SELECT orderseq_number AS acnumber "
          "  FROM orderseq"
          " WHERE (orderseq_name='CRMAccountNumber');" );
  if (q.first())
    _nextAcctNumber->setText(q.value("acnumber"));

  QString metric = _metrics->value("CRMAccountNumberGeneration");
  if (metric == "M")
    _acctGeneration->setCurrentIndex(0);
  else if (metric == "A")
    _acctGeneration->setCurrentIndex(1);
  else if (metric == "O")
    _acctGeneration->setCurrentIndex(2);

    
  _useProjects->setChecked(_metrics->boolean("UseProjects"));
  _autoCreate->setChecked(_metrics->boolean("AutoCreateProjectsForOrders"));
  if (_metrics->boolean("EnableBatchManager"))
  {
    _incdtEmailProfile->populate("SELECT ediprofile_id, ediprofile_name "
                                 "FROM ediprofile "
                                 "WHERE (ediprofile_type='email');");
    _incdtEmailProfile->setId(_metrics->value("CRMIncidentEmailProfile").toInt());
    _incdtCreated->setChecked(_metrics->boolean("CRMIncidentEmailCreated"));
    _incdtAssigned->setChecked(_metrics->boolean("CRMIncidentEmailAssigned"));
    _incdtStatus->setChecked(_metrics->boolean("CRMIncidentEmailStatus"));
    _incdtUpdated->setChecked(_metrics->boolean("CRMIncidentEmailUpdated"));
    _incdtComments->setChecked(_metrics->boolean("CRMIncidentEmailComments"));
    _incdtPreview->setChecked(_metrics->boolean("CRMIncidentEmailPreview"));
  }
  else
  {
    _incdtEmailProfileLit->hide();
    _incdtEmailProfile->hide();
    _incdtDelGroup->hide();
  }
      
  adjustSize();
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureCRM::~configureCRM()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureCRM::languageChange()
{
    retranslateUi(this);
}

void configureCRM::sSave()
{
  const char *numberGenerationTypes[] = { "M", "A", "O" };

  q.prepare( "SELECT setNextIncidentNumber(:innumber);" );
  q.bindValue(":innumber", _nextInNumber->text().toInt());
  q.exec();
  
  q.prepare( "SELECT setNextCRMAccountNumber(:acnumber);" );
  q.bindValue(":acnumber", _nextAcctNumber->text().toInt());
  q.exec();

  _metrics->set("CRMAccountNumberGeneration", QString(numberGenerationTypes[_acctGeneration->currentIndex()]));
  
  _metrics->set("UseProjects", _useProjects->isChecked());
  _metrics->set("AutoCreateProjectsForOrders", (_autoCreate->isChecked() && _useProjects->isChecked()));
  
  if (_metrics->boolean("EnableBatchManager"))
  {
    _metrics->set("CRMIncidentEmailProfile", _incdtEmailProfile->id());
    _metrics->set("CRMIncidentEmailCreated"   , _incdtCreated->isChecked());
    _metrics->set("CRMIncidentEmailAssigned"  , _incdtAssigned->isChecked());
    _metrics->set("CRMIncidentEmailStatus"    , _incdtStatus->isChecked());
    _metrics->set("CRMIncidentEmailUpdated"   , _incdtUpdated->isChecked());
    _metrics->set("CRMIncidentEmailComments"  , _incdtComments->isChecked());
    _metrics->set("CRMIncidentEmailPreview"   , _incdtPreview->isChecked());
  }
  
  _metrics->load();

  accept();
}


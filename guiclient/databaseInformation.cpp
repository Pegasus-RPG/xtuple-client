/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "databaseInformation.h"

#include <QVariant>
#include <QDebug>

#include <dbtools.h>

databaseInformation::databaseInformation(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));

  QString server;
  QString database;
  QString port;

  _description->setFocus();

  _description->setText(_metrics->value("DatabaseName"));
  _comments->setText(_metrics->value("DatabaseComments"));
  _version->setText(_metrics->value("ServerVersion"));
  _patch->setText(_metrics->value("ServerPatchVersion"));
  _disallowMismatchClient->setChecked(_metrics->boolean("DisallowMismatchClientVersion"));

  QString access = _metrics->value("AllowedUserLogins");
qDebug() << "loading with " << access;
  if("AdminOnly" == access)
    _access->setCurrentIndex(1);
  else if("Any" == access)
    _access->setCurrentIndex(2);
  else
    _access->setCurrentIndex(0);

  int val = _metrics->value("updateTickInterval").toInt();
  if(val < 1) val = 1;
  _interval->setValue(val);

   //Disable batch manager if PostBooks 	 
  if ( (_metrics->value("Application") != "Manufacturing") 	 
    && (_metrics->value("Application") != "Standard") ) 	 
  { 	 
    _enableBatchManager->setChecked(FALSE); 	 
    _enableBatchManager->hide(); 	 
    _defaultFromAddress->setText(""); 	 
    _defaultFromAddress->hide(); 	 
    _defaultFromAddressLit->hide(); 	 
    _purgeDaysLit->hide(); 	 
    _purgeDays->hide(); 	 
    _purgeDaysDaysLit->hide(); 	 
  } 	 
  else 	 
  {
    _defaultFromAddress->setText(_metrics->value("DefaultBatchFromEmailAddress"));
    _enableBatchManager->setChecked(_metrics->boolean("EnableBatchManager"));
    _purgeDays->setValue(_metrics->value("BatchManagerPurgeDays").toInt());
  }

  QString protocol;
  parseDatabaseURL(omfgThis->databaseURL(), protocol, server, database, port);
  _application->setText(_metrics->value("Application"));
  _server->setText(server);
  _name->setText(database);
  
  q.exec( "SELECT numOfDatabaseUsers() AS databaseusers,"
          "       numOfServerUsers() AS serverusers;" );
  if (q.first())
  {
    _numOfDatabaseUsers->setText(q.value("databaseusers").toString());
    _numOfServerUsers->setText(q.value("serverusers").toString());
  }
//  ToDo

  if (!_privileges->check("ConfigDatabaseInfo"))
  {
    _description->setEnabled(FALSE);
    _defaultFromAddress->setEnabled(FALSE);
    _comments->setEnabled(FALSE);
    _close->setText(tr("&Close"));
    _save->hide();
  }
}

databaseInformation::~databaseInformation()
{
  // no need to delete child widgets, Qt does it all for us
}

void databaseInformation::languageChange()
{
  retranslateUi(this);
}

void databaseInformation::sSave()
{
  _metrics->set("DatabaseName", _description->text().trimmed());
  _metrics->set("DefaultBatchFromEmailAddress", _defaultFromAddress->text().trimmed());
  _metrics->set("DatabaseComments", _comments->toPlainText().trimmed());
  _metrics->set("DisallowMismatchClientVersion", _disallowMismatchClient->isChecked());

  _metrics->set("EnableBatchManager", _enableBatchManager->isChecked());
  _metrics->set("BatchManagerPurgeDays", _purgeDays->value());
  _metrics->set("updateTickInterval", _interval->value());

qDebug() << "CurrentIndex " << _access->currentIndex();
  if(_access->currentIndex() == 1)
    _metrics->set("AllowedUserLogins", QString("AdminOnly"));
  else if(_access->currentIndex() == 2)
    _metrics->set("AllowedUserLogins", QString("Any"));
  else
    _metrics->set("AllowedUserLogins", QString("ActiveOnly"));

  _metrics->load();

  omfgThis->setWindowTitle();

  accept();
}

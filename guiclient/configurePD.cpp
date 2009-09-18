/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configurePD.h"

#include <QVariant>
#include <QMessageBox>
#include <QSqlError>
#include "storedProcErrorLookup.h"
#include "guiclient.h"

configurePD::configurePD(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  if (_metrics->value("TrackMachineOverhead") == "M")
    _machineOverhead->setChecked(TRUE);
  else
    _generalOverhead->setChecked(TRUE);

  _inactiveBomItems->setChecked(_metrics->boolean("AllowInactiveBomItems"));
  _exclusive->setChecked(_metrics->boolean("DefaultSoldItemsExclusive"));
  _changeLog->setChecked(_metrics->boolean("ItemChangeLog"));

  QString issueMethod = _metrics->value("DefaultWomatlIssueMethod");
  if (issueMethod == "S")
    _issueMethod->setCurrentIndex(0);
  else if (issueMethod == "L")
    _issueMethod->setCurrentIndex(1);
  else if (issueMethod == "M")
    _issueMethod->setCurrentIndex(2);
    
  
  if (_metrics->value("Application") != "Manufacturing")
  {
    _routings->hide();
    _routings->setChecked(FALSE);
    _bbom->hide();
  }
  else
  {
    q.exec("SELECT booitem_id FROM booitem "
             "UNION "
             "SELECT wooper_id FROM wooper, wo "
             "WHERE ((wo_id=wooper_wo_id) "
             "AND (wo_status <> 'C')) "
             "LIMIT 1;");
    if (q.first())
    {
      _routings->setCheckable(FALSE);
      _routings->setTitle("Work Center Routings");
    } 
    else
      _routings->setChecked(_metrics->boolean("Routings"));
      
    q.exec("SELECT item_id FROM item WHERE (item_type IN ('B','C','Y')) LIMIT 1;");
    if (q.first())
    {
      _bbom->setChecked(TRUE);
      _bbom->setEnabled(FALSE);
    }
    else
      _bbom->setChecked(_metrics->boolean("BBOM"));
  }
  
  
  if(_metrics->value("Application") == "PostBooks")
  {
    _revControl->hide();
    _transforms->hide();
  }
  else
  {
    q.exec("SELECT * FROM itemtrans LIMIT 1;");
    if (q.first())
    {
      _transforms->setChecked(TRUE);
      _transforms->setEnabled(FALSE);
    }
    else 
      _transforms->setChecked(_metrics->boolean("Transforms"));

    q.exec("SELECT * FROM rev LIMIT 1;");
    if (q.first())
    {
      _revControl->setChecked(TRUE);
      _revControl->setEnabled(FALSE);
    }
    else 
      _revControl->setChecked(_metrics->boolean("RevControl"));
  }
  
  this->setWindowTitle("Products Configuration");

  adjustSize();
}

configurePD::~configurePD()
{
  // no need to delete child widgets, Qt does it all for us
}

void configurePD::languageChange()
{
  retranslateUi(this);
}

void configurePD::sSave()
{
  if (!_metrics->boolean("RevControl") && (_revControl->isChecked()))
  {
    if (QMessageBox::warning(this, tr("Enable Revision Control"),
      tr("Enabling revision control will create control records "
         "for products that contain revision number data.  This "
         "change can not be undone.  Do you wish to proceed?"),
        QMessageBox::Yes | QMessageBox::Default,
        QMessageBox::No  | QMessageBox::Escape) == QMessageBox::Yes)
    {
      _metrics->set("RevControl", TRUE);
      
      QString rsql = "SELECT createbomrev(bomhead_item_id,bomhead_revision) AS result "
                     "  FROM bomhead "
                     " WHERE((COALESCE(bomhead_revision,'') <> '') "
                     "   AND (bomhead_rev_id=-1))";
      if (_metrics->value("Application") == "Manufacturing")
        rsql += " UNION "
                "SELECT createboorev(boohead_item_id,boohead_revision) "
                "  FROM boohead "
                " WHERE((COALESCE(boohead_revision,'') <> '') "
                "   AND (boohead_rev_id=-1));";
      q.exec(rsql);
      if (q.first() && (q.value("result").toInt() < 0))
      {
        systemError(this, storedProcErrorLookup("CreateRevision", q.value("result").toInt()),
            __FILE__, __LINE__);
        _metrics->set("RevControl", FALSE);
        return;
      }
      if (q.lastError().type() != QSqlError::NoError)
      {
        QMessageBox::critical(this, tr("A System Error Occurred at %1::%2.")
          .arg(__FILE__)
          .arg(__LINE__),
          q.lastError().databaseText());
        _metrics->set("RevControl", FALSE);
        return;
      }
    }
    else
      return;
  }

  _metrics->set("TrackMachineOverhead", ((_machineOverhead->isChecked()) ? QString("M") : QString("G")));
  _metrics->set("Routings", ((_routings->isChecked()) || (!_routings->isCheckable())));
  _metrics->set("BBOM", ((_bbom->isChecked()) && (!_bbom->isHidden())));
  _metrics->set("Transforms", ((_transforms->isChecked()) && (!_transforms->isHidden())));
  _metrics->set("RevControl", ((_revControl->isChecked()) && (!_revControl->isHidden())));
  _metrics->set("AllowInactiveBomItems", _inactiveBomItems->isChecked());
  _metrics->set("DefaultSoldItemsExclusive", _exclusive->isChecked());
  _metrics->set("ItemChangeLog", _changeLog->isChecked());
  
  if (_issueMethod->currentIndex() == 0)
    _metrics->set("DefaultWomatlIssueMethod", QString("S"));
  else if (_issueMethod->currentIndex() == 1)
    _metrics->set("DefaultWomatlIssueMethod", QString("L"));
  else if (_issueMethod->currentIndex() == 2)
    _metrics->set("DefaultWomatlIssueMethod", QString("M"));

  _metrics->load();
  _privileges->load();
  omfgThis->saveToolbarPositions();
  _preferences->load();
  omfgThis->initMenuBar();

  accept();
}

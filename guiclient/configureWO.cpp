/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureWO.h"

#include <QVariant>
#include <QValidator>
#include "guiclient.h"

/*
 *  Constructs a configureWO as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureWO::configureWO(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);


  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
  connect(_autoExplode, SIGNAL(toggled(bool)), _WOExplosionGroup, SLOT(setDisabled(bool)));
  connect(_autoExplode, SIGNAL(toggled(bool)), _multiLevel, SLOT(setChecked(bool)));

  _nextWoNumber->setValidator(omfgThis->orderVal());

  q.exec( "SELECT orderseq_number "
          "FROM orderseq "
          "WHERE (orderseq_name='WoNumber')" );
  if (q.first())
    _nextWoNumber->setText(q.value("orderseq_number").toString());

  _autoExplode->setChecked(_metrics->boolean("AutoExplodeWO"));
  _workOrderChangeLog->setChecked(_metrics->boolean("WorkOrderChangeLog"));
  _postopFillQty->setChecked(_metrics->boolean("AutoFillPostOperationQty"));

  _woNumGeneration->insertItem(tr("Automatic"));
  _woNumGeneration->insertItem(tr("Manual"));
  _woNumGeneration->insertItem(tr("Automatic, Allow Override"));

  if (_metrics->value("WONumberGeneration") == "A")
    _woNumGeneration->setCurrentIndex(0);
  else if (_metrics->value("WONumberGeneration") == "M")
    _woNumGeneration->setCurrentIndex(1);
  else if (_metrics->value("WONumberGeneration") == "O")
    _woNumGeneration->setCurrentIndex(2);

  if (_metrics->value("ExplodeWOEffective") == "E")
    _explodeDateEffective->setChecked(TRUE);
  else
    _startDateEffective->setChecked(TRUE);

  if (_metrics->value("WOExplosionLevel") == "S")
    _singleLevel->setChecked(TRUE);
  else if (_metrics->value("WOExplosionLevel") == "M")
    _multiLevel->setChecked(TRUE);

  _materialVariances->setChecked(_metrics->boolean("PostMaterialVariances"));
  
  if (_metrics->boolean("Routings"))
    _laborVariances->setChecked(_metrics->boolean("PostLaborVariances"));
  else
  {
    _laborVariances->setChecked(FALSE);
    _laborVariances->hide();
    _wotcPostStyle->hide();
    _postopFillQty->hide();
  }

  if (_metrics->value("WOTCPostStyle") == "Production")
    _production->setChecked(TRUE);
  else
    _operations->setChecked(TRUE);

  if (_metrics->value("JobItemCosDefault") == "P")
    _proportional->setChecked(TRUE);
  else
    _todate->setChecked(TRUE);
    
  this->setWindowTitle("Manufacture Configuration");
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureWO::~configureWO()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureWO::languageChange()
{
  retranslateUi(this);
}

void configureWO::sSave()
{
  q.prepare("SELECT setNextWoNumber(:woNumber) AS result;");
  q.bindValue(":woNumber", _nextWoNumber->text().toInt());
  q.exec();

  _metrics->set("AutoExplodeWO", _autoExplode->isChecked());
  _metrics->set("WorkOrderChangeLog", _workOrderChangeLog->isChecked());
  _metrics->set("AutoFillPostOperationQty", _postopFillQty->isChecked());

  if (_woNumGeneration->currentIndex() == 0)
    _metrics->set("WONumberGeneration", QString("A"));
  else if (_woNumGeneration->currentIndex() == 1)
    _metrics->set("WONumberGeneration", QString("M"));
  else if (_woNumGeneration->currentIndex() == 2)
    _metrics->set("WONumberGeneration", QString("O"));

  _metrics->set("ExplodeWOEffective", ((_explodeDateEffective->isChecked()) ? QString("E") : QString("S")));
  _metrics->set("WOExplosionLevel", ((_singleLevel->isChecked()) ? QString("S") : QString("M")));
  _metrics->set("PostMaterialVariances", _materialVariances->isChecked());
  _metrics->set("PostLaborVariances", _laborVariances->isChecked());

  if (_production->isChecked())
    _metrics->set("WOTCPostStyle", QString("Production"));
  else if (_operations->isChecked())
    _metrics->set("WOTCPostStyle", QString("Operations"));

  if (_todate->isChecked())
    _metrics->set("JobItemCosDefault", QString("D"));
  else 
    _metrics->set("JobItemCosDefault", QString("P"));

  _metrics->load();

  accept();
}


/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "whseWeek.h"

#include <QMessageBox>
#include <QVariant>

/*
 *  Constructs a whseWeek as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
whseWeek::whseWeek(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
  connect(_close, SIGNAL(clicked()), this, SLOT(sClose()));
  connect(_warehouse, SIGNAL(updated()), this, SLOT(sPopulate()));
  connect(_sunday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_monday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_tuesday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_wednesday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_thursday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_friday, SIGNAL(toggled(bool)), this, SLOT(sChange()));
  connect(_saturday, SIGNAL(toggled(bool)), this, SLOT(sChange()));

  _warehousid = -1;
  _dirty = false;

  sPopulate();
}

/*
 *  Destroys the object and frees any allocated resources
 */
whseWeek::~whseWeek()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void whseWeek::languageChange()
{
    retranslateUi(this);
}

void whseWeek::sPopulate()
{
  if(!checkAndSave())
  {
    if(-1 == _warehousid)
      _warehouse->setAll();
    else
      _warehouse->setId(_warehousid);
    return;
  }

  _warehousid = _warehouse->id();

  q.prepare("SELECT TRUE AS result"
            "  FROM whsewk"
            " WHERE ((whsewk_warehous_id=:warehous_id)"
            "   AND  (whsewk_weekday=:wkday));");

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 0);
  q.exec();
  bool result = false;
  if(q.first())
    result = q.value("result").toBool();
  _sunday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 1);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _monday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 2);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _tuesday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 3);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _wednesday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 4);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _thursday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 5);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _friday->setChecked(result);

  q.bindValue(":warehous_id", _warehousid);
  q.bindValue(":wkday", 6);
  q.exec();
  result = false;
  if(q.first())
    result = q.value("result").toBool();
  _saturday->setChecked(result);

  _dirty = false;
}

void whseWeek::sSave()
{
  if(save(_warehouse->id()))
    accept();
}

bool whseWeek::save(int whsid)
{
  q.prepare("SELECT setWhseWkday(:whsid, 0, :sunday),"
            "       setWhseWkday(:whsid, 1, :monday),"
            "       setWhseWkday(:whsid, 2, :tuesday),"
            "       setWhseWkday(:whsid, 3, :wednesday),"
            "       setWhseWkday(:whsid, 4, :thursday),"
            "       setWhseWkday(:whsid, 5, :friday),"
            "       setWhseWkday(:whsid, 6, :saturday);");
  q.bindValue(":whsid", whsid);
  q.bindValue(":sunday", QVariant(_sunday->isChecked()));
  q.bindValue(":monday", QVariant(_monday->isChecked()));
  q.bindValue(":tuesday", QVariant(_tuesday->isChecked()));
  q.bindValue(":wednesday", QVariant(_wednesday->isChecked()));
  q.bindValue(":thursday", QVariant(_thursday->isChecked()));
  q.bindValue(":friday", QVariant(_friday->isChecked()));
  q.bindValue(":saturday", QVariant(_saturday->isChecked()));

  if(!q.exec())
  {
    QMessageBox::critical(this, tr("Error Saving"),
      tr("There was an error saving the information.") );
    return false;
  }

  if(whsid == -1)
  {
    q.prepare("SELECT setWhseWkday(warehous_id, 0, :sunday),"
              "       setWhseWkday(warehous_id, 1, :monday),"
              "       setWhseWkday(warehous_id, 2, :tuesday),"
              "       setWhseWkday(warehous_id, 3, :wednesday),"
              "       setWhseWkday(warehous_id, 4, :thursday),"
              "       setWhseWkday(warehous_id, 5, :friday),"
              "       setWhseWkday(warehous_id, 6, :saturday)"
              "  FROM warehous;");
    q.bindValue(":sunday", QVariant(_sunday->isChecked()));
    q.bindValue(":monday", QVariant(_monday->isChecked()));
    q.bindValue(":tuesday", QVariant(_tuesday->isChecked()));
    q.bindValue(":wednesday", QVariant(_wednesday->isChecked()));
    q.bindValue(":thursday", QVariant(_thursday->isChecked()));
    q.bindValue(":friday", QVariant(_friday->isChecked()));
    q.bindValue(":saturday", QVariant(_saturday->isChecked()));
    q.exec();
  }

  _dirty = false;
  return true;
}

void whseWeek::sClose()
{
  if(checkAndSave())
    reject();
}

bool whseWeek::checkAndSave()
{
  if (_dirty)
  {
    switch(QMessageBox::question(this, tr("Unsaved Changes"),
             tr("There are unsaved changes. Would you like to save before continuing?"),
             QMessageBox::Yes | QMessageBox::Default,
             QMessageBox::No,
             QMessageBox::Cancel | QMessageBox::Escape))
    {
      case QMessageBox::Cancel:
        return false;
      case QMessageBox::Yes:
        return save(_warehousid);
    }
  }

  return true;
}

void whseWeek::sChange()
{
  _dirty = true;
}


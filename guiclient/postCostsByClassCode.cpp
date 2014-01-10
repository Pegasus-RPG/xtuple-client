/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "postCostsByClassCode.h"

#include <QSqlError>
#include <QVariant>

postCostsByClassCode::postCostsByClassCode(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
  setupUi(this);

  // signals and slots connections
  connect(_post, SIGNAL(clicked()), this, SLOT(sPost()));
  connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
  connect(_close, SIGNAL(clicked()), this, SLOT(reject()));

  _classCode->setType(ParameterGroup::ClassCode);

  if (!_metrics->boolean("Routings"))
  {
    _directLabor->hide();
    _lowerDirectLabor->hide();
    _overhead->hide();
    _lowerOverhead->hide();
    _machOverhead->hide();
    _lowerMachOverhead->hide();
  }
  else if (_metrics->value("TrackMachineOverhead") != "M")
  {
    _machOverhead->setEnabled(FALSE);
    _machOverhead->setChecked(TRUE);
    _lowerMachOverhead->setEnabled(FALSE);
    _lowerMachOverhead->setChecked(TRUE);
  }
}

postCostsByClassCode::~postCostsByClassCode()
{
  // no need to delete child widgets, Qt does it all for us
}

void postCostsByClassCode::languageChange()
{
  retranslateUi(this);
}

void postCostsByClassCode::sSelectAll()
{
  _material->setChecked(TRUE);
  _lowerMaterial->setChecked(TRUE);
  _user->setChecked(TRUE);
  _lowerUser->setChecked(TRUE);
  if (_metrics->boolean("Routings"))
  {
    _directLabor->setChecked(TRUE);
    _lowerDirectLabor->setChecked(TRUE);
    _overhead->setChecked(TRUE);
    _lowerOverhead->setChecked(TRUE);
    if (_metrics->value("TrackMachineOverhead") == "M")
    {
      _machOverhead->setChecked(TRUE);
      _lowerMachOverhead->setChecked(TRUE);
    }
  }
}

void postCostsByClassCode::sPost()
{
  XSqlQuery postPost;
  QString sql = "SELECT doPostCosts(item_id, TRUE, :material, :lowMaterial, "
                "       :directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
                "       :machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp) "
                "  FROM item, classcode "
                " WHERE(item_classcode_id=classcode_id)";

  if (_classCode->isSelected())
    sql += " AND (item_classcode_id=:classcode_id);";
  else if (_classCode->isPattern())
    sql += " AND (classcode_code ~ :classcode_pattern);";
  else
    sql = "SELECT doPostCosts(:material, :lowMaterial, "
          "    :directLabor, :lowDirectLabor, :overhead, :lowOverhead, "
          "    :machOverhead, :lowMachOverhead, :user, :lowUser, :rollUp);";

  postPost.prepare(sql);
  postPost.bindValue(":material",        _material->isChecked()          ? "t" : "f");
  postPost.bindValue(":lowMaterial",     _lowerMaterial->isChecked()     ? "t" : "f");
  postPost.bindValue(":directLabor",     _directLabor->isChecked()       ? "t" : "f");
  postPost.bindValue(":lowDirectLabor",  _lowerDirectLabor->isChecked()  ? "t" : "f");
  postPost.bindValue(":overhead",        _overhead->isChecked()          ? "t" : "f");
  postPost.bindValue(":lowOverhead",     _lowerOverhead->isChecked()     ? "t" : "f");
  postPost.bindValue(":machOverhead",    _machOverhead->isChecked()      ? "t" : "f");
  postPost.bindValue(":lowMachOverhead", _lowerMachOverhead->isChecked() ? "t" : "f");
  postPost.bindValue(":user",            _user->isChecked()              ? "t" : "f");
  postPost.bindValue(":lowUser",         _lowerUser->isChecked()         ? "t" : "f");
  postPost.bindValue(":rollUp",          _rollUp->isChecked()            ? "t" : "f");

  _classCode->bindValue(postPost);
  postPost.exec();
  if (postPost.lastError().type() != QSqlError::NoError)
  {
    systemError(this, postPost.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  accept();
}

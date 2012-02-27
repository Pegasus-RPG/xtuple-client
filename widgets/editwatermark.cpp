/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2012 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "editwatermark.h"

#include <QVariant>

/**
  @class EditWatermark

  @brief The EditWatermark class is for internal use only.

  This is a helper dialog for XDocCopySetter.
  */

EditWatermark::EditWatermark(QWidget* parent, Qt::WindowFlags fl)
  : QDialog(parent, fl)
{
  setupUi(this);
}

EditWatermark::~EditWatermark()
{
  // no need to delete child widgets, Qt does it all for us
}

void EditWatermark::languageChange()
{
  retranslateUi(this);
}

bool EditWatermark::set(const ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("watermark", &valid);
  if (valid)
    _watermark->setText(param.toString());

  param = pParams.value("showPrices", &valid);
  if (valid)
    _showPrices->setChecked(param.toBool());

  return true;
}


QString EditWatermark::watermark()
{
  return _watermark->text();
}


bool EditWatermark::showPrices()
{
  return _showPrices->isChecked();
}

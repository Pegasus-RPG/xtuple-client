/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2011 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "welcomeStub.h"

#include <QStyle>
#include <QPixmap>
#include <QIcon>

welcomeStub::welcomeStub(QWidget* parent, Qt::WindowFlags f)
  : QDialog(parent, f)
{
  setupUi(this);

  QString url = "<a href=\"http://www.xtuple.com/translation/\">www.xtuple.com/translation</a>";

  int iconSize = style()->pixelMetric(QStyle::PM_MessageBoxIconSize);
  iconLabel->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxInformation).pixmap(iconSize, iconSize));
  label->setText(tr("Some text goes here with a Url here %1 and maybe some other words around it").arg(url));

  adjustSize();
}

welcomeStub::~welcomeStub()
{
  // no need to delete child widgets, Qt does it all for us
}

void welcomeStub::languageChange()
{
  retranslateUi(this);
}


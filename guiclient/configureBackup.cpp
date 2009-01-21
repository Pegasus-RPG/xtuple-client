/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "configureBackup.h"

#include <qvariant.h>
#include <q3filedialog.h>
#include "guiclient.h"

/*
 *  Constructs a configureBackup as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
configureBackup::configureBackup(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_save, SIGNAL(clicked()), this, SLOT(sSave()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
configureBackup::~configureBackup()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void configureBackup::languageChange()
{
    retranslateUi(this);
}


void configureBackup::init()
{
  _account->setText(_metrics->value("BackupAccountName"));
}

void configureBackup::sSave()
{
  _metrics->set("BackupAccountName", _account->text().trimmed());

  accept();
}


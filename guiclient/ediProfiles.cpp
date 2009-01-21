/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "ediProfiles.h"

#include <qvariant.h>
#include <qmessagebox.h>
//#include <qstatusbar.h>
#include <qworkspace.h>
#include <openreports.h>
#include "ediProfile.h"

/*
 *  Constructs a ediProfiles as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
ediProfiles::ediProfiles(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_ediprofile, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_ediprofile, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_ediprofile, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
ediProfiles::~ediProfiles()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ediProfiles::languageChange()
{
    retranslateUi(this);
}


void ediProfiles::init()
{
//  statusBar()->hide();
  
  _ediprofile->addColumn(tr("Name"), -1, Qt::AlignLeft, true, "ediprofile_name" );
  _ediprofile->addColumn(tr("Type"), _itemColumn, Qt::AlignLeft, true, "ediprofile_type" );

  sFillList();
}

void ediProfiles::sPrint()
{
  orReport report("EDIProfilesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void ediProfiles::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  ediProfile newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfiles::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("ediprofile_id", _ediprofile->id());

  ediProfile newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void ediProfiles::sDelete()
{
  q.prepare("SELECT deleteEDIProfile(:ediprofile_id) AS RESULT;");
  q.bindValue(":ediprofile_id", _ediprofile->id());
  q.exec();
  if(q.first())
  {
    switch(q.value("result").toInt())
    {
      case -1:
        QMessageBox::critical( this, tr("Cannot Delete EDI Profile"),
          tr("The select EDI Profile cannot be deleted because it is currently\n"
             "being referenced by one or more records.") );
        return;
      default:
        sFillList();
    }
  }
  else
    systemError( this, tr("A System Error occurred at ediProfiles::%1.")
                       .arg(__LINE__) );
}

void ediProfiles::sFillList()
{
  _ediprofile->populate( "SELECT ediprofile_id, ediprofile_name, ediprofile_type "
                         "  FROM ediprofile "
                         "ORDER BY ediprofile_name" );
}

/*
 * Common Public Attribution License Version 1.0. 
 * 
 * The contents of this file are subject to the Common Public Attribution 
 * License Version 1.0 (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License 
 * at http://www.xTuple.com/CPAL.  The License is based on the Mozilla 
 * Public License Version 1.1 but Sections 14 and 15 have been added to 
 * cover use of software over a computer network and provide for limited 
 * attribution for the Original Developer. In addition, Exhibit A has 
 * been modified to be consistent with Exhibit B.
 * 
 * Software distributed under the License is distributed on an "AS IS" 
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See 
 * the License for the specific language governing rights and limitations 
 * under the License. 
 * 
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
 * 
 * Contributor(s): ______________________.
 * 
 * Alternatively, the contents of this file may be used under the terms 
 * of the xTuple End-User License Agreeement (the xTuple License), in which 
 * case the provisions of the xTuple License are applicable instead of 
 * those above.  If you wish to allow use of your version of this file only 
 * under the terms of the xTuple License and not to allow others to use 
 * your version of this file under the CPAL, indicate your decision by 
 * deleting the provisions above and replace them with the notice and other 
 * provisions required by the xTuple License. If you do not delete the 
 * provisions above, a recipient may use your version of this file under 
 * either the CPAL or the xTuple License.
 * 
 * EXHIBIT B.  Attribution Information
 * 
 * Attribution Copyright Notice: 
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
 * 
 * Attribution URL: www.xtuple.org 
 * (to be included in the "Community" menu of the application if possible)
 * 
 * Graphic Image as provided in the Covered Code, if any. 
 * (online at www.xtuple.com/poweredby)
 * 
 * Display of Attribution Information is required in Larger Works which 
 * are defined in the CPAL as a work which combines Covered Code or 
 * portions thereof with code not governed by the terms of the CPAL.
 */

#include "printAPChecksReview.h"

#include <qvariant.h>

/*
 *  Constructs a printAPChecksReview as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
printAPChecksReview::printAPChecksReview(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_complete, SIGNAL(clicked()), this, SLOT(sComplete()));
    connect(_unmark, SIGNAL(clicked()), this, SLOT(sUnmark()));
    connect(_printed, SIGNAL(clicked()), this, SLOT(sMarkPrinted()));
    connect(_voided, SIGNAL(clicked()), this, SLOT(sMarkVoided()));
    connect(_replace, SIGNAL(clicked()), this, SLOT(sMarkReplaced()));
    connect(_selectAll, SIGNAL(clicked()), this, SLOT(sSelectAll()));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
printAPChecksReview::~printAPChecksReview()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void printAPChecksReview::languageChange()
{
    retranslateUi(this);
}


static const int ActionUnmark = -1;
static const int ActionPrinted = 1;
static const int ActionVoided = 2;
static const int ActionReplaced = 3;

void printAPChecksReview::init()
{
  _checks->addColumn(tr("Check Number"),          -1, Qt::AlignLeft );
  _checks->addColumn(tr("Action"),       _itemColumn, Qt::AlignLeft );
}

void printAPChecksReview::sComplete()
{
  XListViewItem *cursor = _checks->firstChild();

  XSqlQuery checkPrint;
  checkPrint.prepare( "UPDATE apchk "
                      "SET apchk_printed=TRUE "
                      "WHERE (apchk_id=:apchk_id);" );

  XSqlQuery checkVoid;
  checkVoid.prepare( "SELECT voidAPCheck(:apchk_id) AS result;" );

  XSqlQuery checkReplace;
  checkReplace.prepare( "SELECT replaceVoidedAPCheck(:apchk_id) AS result;" );

  while( cursor )
  {
    switch(cursor->altId())
    {
      case ActionPrinted:
        checkPrint.bindValue(":apchk_id", cursor->id());
        checkPrint.exec();
        break;
      case ActionVoided:
        checkVoid.bindValue(":apchk_id", cursor->id());
        checkVoid.exec();
        break;
      case ActionReplaced:
        checkVoid.bindValue(":apchk_id", cursor->id());
        checkVoid.exec();
        checkReplace.bindValue(":apchk_id", cursor->id());
        checkReplace.exec();
        break;
    }
    cursor = cursor->nextSibling();
  }

  close();
}

void printAPChecksReview::sUnmark()
{
  markSelected(ActionUnmark);
}

void printAPChecksReview::sMarkPrinted()
{
  markSelected(ActionPrinted);
}

void printAPChecksReview::sMarkVoided()
{
  markSelected(ActionVoided);
}

void printAPChecksReview::sMarkReplaced()
{
  markSelected(ActionReplaced);
}

void printAPChecksReview::sSelectAll()
{
  _checks->selectAll(TRUE);
}

void printAPChecksReview::markSelected( int actionId )
{
  QString action;
  switch(actionId)
  {
    case ActionPrinted:
      action = tr("Printed");
      break;
    case ActionVoided:
      action = tr("Voided");
      break;
    case ActionReplaced:
      action = tr("Replace");
      break;
    case ActionUnmark:
    default:
      actionId = -1;
      action = "";
  };

  XListViewItem *cursor = _checks->firstChild();
  while( cursor )
  {
    if(_checks->isSelected(cursor))
    {
      cursor->setText(1, action);
      cursor->setAltId(actionId);
    }
    cursor = cursor->nextSibling();
  }
}


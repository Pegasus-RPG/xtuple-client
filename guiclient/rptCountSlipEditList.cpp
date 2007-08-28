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

#include "rptCountSlipEditList.h"

#include <qvariant.h>
#include <qmessagebox.h>
#include <openreports.h>
#include "countTagList.h"

/*
 *  Constructs a rptCountSlipEditList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptCountSlipEditList::rptCountSlipEditList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_item, SIGNAL(warehouseIdChanged(int)), _warehouse, SLOT(setId(int)));
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_countTagList, SIGNAL(clicked()), this, SLOT(sCountTagList()));
    init();
    
    //If not multi-warehouse hide whs control
    if (!_metrics->boolean("MultiWhs"))
    {
      _warehouseLit->hide();
      _warehouse->hide();
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
rptCountSlipEditList::~rptCountSlipEditList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptCountSlipEditList::languageChange()
{
    retranslateUi(this);
}


void rptCountSlipEditList::init()
{
  _item->setReadOnly(TRUE);

#ifdef Q_WS_MAC
  _countTagList->setMaximumWidth(50);
#else
  _countTagList->setMaximumWidth(25);
#endif
}

enum SetResponse rptCountSlipEditList::set(ParameterList &pParams)
{
  _captive = TRUE;

  QVariant param;
  bool     valid;

  param = pParams.value("cnttag_id", &valid);
  if (valid)
  {
    _cnttagid = param.toInt();
    populate();
  }

  if (pParams.inList("print"))
  {
    sPrint();
    return NoError_Print;
  }

  return NoError;
}

void rptCountSlipEditList::sCountTagList()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);
  params.append("tagType", cUnpostedCounts);

  countTagList newdlg(this, "", TRUE);
  newdlg.set(params);

  int _newCnttagid;
  if ((_newCnttagid = newdlg.exec()) != QDialog::Rejected)
  {
    _cnttagid = _newCnttagid;
    populate();
  }
}

void rptCountSlipEditList::sPrint()
{
  ParameterList params;
  params.append("cnttag_id", _cnttagid);

  orReport report("CountSlipEditList", params);
  if (report.isValid())
    report.print();
  else
  {
    report.reportError(this);
    reject();
  }

  if (_captive)
    accept();
}

void rptCountSlipEditList::populate()
{
  q.prepare( "SELECT invcnt_tagnumber, invcnt_itemsite_id "
             "FROM invcnt "
             "WHERE (invcnt_id=:cnttag_id);" );
  q.bindValue(":cnttag_id", _cnttagid);
  q.exec();
  if (q.first())
  {
    _countTagNumber->setText(q.value("invcnt_tagnumber").toString());
    _item->setItemsiteid(q.value("invcnt_itemsite_id").toInt());
  }
}


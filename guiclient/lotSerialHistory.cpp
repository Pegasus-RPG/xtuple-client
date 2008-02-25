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
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
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

#include "lotSerialHistory.h"

#include <QSqlError>
#include <QVariant>

#include <comment.h>
#include <parameter.h>

lotSerialHistory::lotSerialHistory(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : XDialog(parent, name, modal, fl)
{
    setupUi(this);

    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_item, SIGNAL(newId(int)), this, SLOT(sPopulateLotSerial(int)));
    connect(_lotSerial, SIGNAL(newID(int)), this, SLOT(sFillList()));

    _lshist->addColumn(tr("Date"),    _dateColumn, Qt::AlignCenter );
    _lshist->addColumn(tr("User"),    _userColumn, Qt::AlignCenter );
    _lshist->addColumn(tr("Comment"), -1,          Qt::AlignLeft   );
}

lotSerialHistory::~lotSerialHistory()
{
    // no need to delete child widgets, Qt does it all for us
}

void lotSerialHistory::languageChange()
{
    retranslateUi(this);
}

void lotSerialHistory::sPopulateLotSerial(int pItemid)
{
  q.prepare( "SELECT lsdetail_id, (lsdetail_lotserial || ' (' || :created || formatDate(lsdetail_created) || ')') "
             "FROM lsdetail, itemsite "
             "WHERE ( (lsdetail_itemsite_id=itemsite_id)"
             " AND (itemsite_item_id=:item_id) );" );
  q.bindValue(":item_id", pItemid);
  q.bindValue(":created", tr("Created"));
  q.exec();
  _lotSerial->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

void lotSerialHistory::sNew()
{
  ParameterList params;
  params.append("mode", "new");
  params.append("lsdetail_id", _lotSerial->id());

  comment newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void lotSerialHistory::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("comment_id", _lshist->id());

  comment newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void lotSerialHistory::sFillList()
{
  q.prepare( "SELECT comment_id, formatDate(comment_date),"
             "       comment_user, firstLine(detag(comment_text)) "
             "FROM comment "
             "WHERE ( (comment_source='L')"
             " AND (comment_source_id=:comment_source_id) ) "
             "ORDER BY comment_date;" );
  q.bindValue(":comment_source_id", _lotSerial->id());
  q.exec();
  _lshist->populate(q);
  if (q.lastError().type() != QSqlError::None)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

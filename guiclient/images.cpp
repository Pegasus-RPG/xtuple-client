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
 * The Original Code is xTuple ERP: PostBooks Edition 
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
 * Powered by xTuple ERP: PostBooks Edition
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

#include "images.h"

#include <QMenu>
#include <QSqlError>

#include <parameter.h>

#include "image.h"
#include "guiclient.h"

images::images(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
  setupUi(this);

  connect(_image, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
  connect(_image, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
  connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
  connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
  connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
  connect(_image, SIGNAL(populateMenu(QMenu *, QTreeWidgetItem *, int)), this, SLOT(sPopulateMenu(QMenu*)));
  connect(_close, SIGNAL(clicked()), this, SLOT(close()));
  connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
  connect(_image, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));

  _image->addColumn(tr("Name"),  _itemColumn, Qt::AlignLeft, true, "image_name");
  _image->addColumn(tr("Description"),    -1, Qt::AlignLeft, true, "image_descrip");
  _image->addColumn(tr("Size"),   _qtyColumn, Qt::AlignRight,true, "image_size");
  _image->addColumn(tr("Package"),_qtyColumn, Qt::AlignRight,false,"nspname");

  connect(_image, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));

  sFillList();
}

images::~images()
{
  // no need to delete child widgets, Qt does it all for us
}

void images::languageChange()
{
  retranslateUi(this);
}

void images::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  image newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void images::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("image_id", _image->id());

  image newdlg(this, "", TRUE);
  newdlg.set(params);

  if (newdlg.exec() != XDialog::Rejected)
    sFillList();
}

void images::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("image_id", _image->id());

  image newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void images::sDelete()
{
  q.prepare( "DELETE FROM image "
             "WHERE (image_id=:image_id);" );
  q.bindValue(":image_id", _image->id());
  q.exec();
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }

  sFillList();
}

void images::sFillList()
{
  q.exec("SELECT image.*, LENGTH(image_data) AS image_size, "
         "       CASE WHEN nspname='public' THEN ''"
         "            ELSE nspname END AS nspname"
         "  FROM image, pg_class, pg_namespace "
         " WHERE ((image.tableoid=pg_class.oid)"
         "   AND  (relnamespace=pg_namespace.oid))"
         "ORDER BY image_name;" );
  _image->populate(q);
  if (q.lastError().type() != QSqlError::NoError)
  {
    systemError(this, q.lastError().databaseText(), __FILE__, __LINE__);
    return;
  }
}

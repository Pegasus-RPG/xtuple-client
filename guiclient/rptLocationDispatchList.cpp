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

#include "rptLocationDispatchList.h"

#include <QVariant>
#include <openreports.h>
#include <QMessageBox>
#include "OpenMFGGUIClient.h"

/*
 *  Constructs a rptLocationDispatchList as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
rptLocationDispatchList::rptLocationDispatchList(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(_close, SIGNAL(clicked()), this, SLOT(reject()));
    connect(_warehouse, SIGNAL(newID(int)), this, SLOT(sPopulateLocations(int)));
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
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
rptLocationDispatchList::~rptLocationDispatchList()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void rptLocationDispatchList::languageChange()
{
    retranslateUi(this);
}


void rptLocationDispatchList::init()
{
  _location->addColumn( tr("Location"),    _itemColumn, Qt::AlignLeft   );
  _location->addColumn( tr("Description"), -1,          Qt::AlignLeft   );
  _location->setSelectionMode(QAbstractItemView::ExtendedSelection);

  sPopulateLocations(_warehouse->id());
}

void rptLocationDispatchList::sPrint()
{
  QString locationids;

  QList<QTreeWidgetItem*> selected = _location->selectedItems();
  for (int i = 0; i < selected.size(); i++)
  {
    if (locationids.length())
      locationids += ", ";

    locationids += QString("%1").arg(((XTreeWidgetItem *)selected[i])->id());
  }

  if (locationids.length())
  {
    ParameterList params;
    params.append( "warehous_id",      _warehouse->id() );
    params.append( "location_id_list", locationids      );

    orReport report("LocationDispatchList", params);
    if (report.isValid())
      report.print();
    else
      report.reportError(this);
  }
  else
    QMessageBox::warning( this, tr("Cannot Print Location Dispatch List"),
                          tr("You must select one or more Locations to print on the requested Location Dispatch List.") );
}

void rptLocationDispatchList::sPopulateLocations(int pWarehousid)
{
  q.prepare( "SELECT location_id, formatLocationName(location_id) AS locationname, location_descrip "
             "FROM location "
             "WHERE (location_warehous_id=:warehous_id) "
             "ORDER BY locationname;" );
  q.bindValue(":warehous_id", pWarehousid);
  q.exec();
  _location->populate(q);
}


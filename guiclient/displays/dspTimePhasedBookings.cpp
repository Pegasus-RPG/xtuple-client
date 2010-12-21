/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "dspTimePhasedBookings.h"

#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QVariant>

#include "guiclient.h"
#include "dspBookings.h"
#include "parameterwidget.h"

dspTimePhasedBookings::dspTimePhasedBookings(QWidget* parent, const char*, Qt::WFlags fl)
  : displayTimePhased(parent, "dspTimePhasedBookings", fl)
{
  setupUi(optionsWidget());
  setWindowTitle(tr("Time-Phased Bookings"));
  setReportName("TimePhasedBookings");
  setMetaSQLOptions("timePhasedBookings", "detail");
  setUseAltId(true);
  setParameterWidgetVisible(true);

  parameterWidget()->append(tr("Customer"),   "cust_id",   ParameterWidget::Customer);
  parameterWidget()->appendComboBox(tr("Item"), "item_id", ParameterWidget::Item);
  parameterWidget()->appendComboBox(tr("Product Category"), "prodcat_id", XComboBox::ProductCategories);
  parameterWidget()->append(tr("Product Category Pattern"), "prodcat_pattern", ParameterWidget::Text);
  if (_metrics->boolean("MultiWhs"))
    parameterWidget()->append(tr("Site"), "warehous_id", ParameterWidget::Site);

  list()->addColumn(tr("Cust. #"),  _orderColumn, Qt::AlignLeft,  true,  "cust_number" );
  list()->addColumn(tr("Customer"), 180,          Qt::AlignLeft,  true,  "cust_name" );
}

void dspTimePhasedBookings::sViewBookings()
{
  if (_column > 1)
  {
    ParameterList params;
    params.append("cust_id", list()->id());
    params.append("startDate", _columnDates[_column - 2].startDate);
    params.append("endDate", _columnDates[_column - 2].endDate);
    params.append("run");

    dspBookings *newdlg = new dspBookings();
    newdlg->set(params);
    omfgThis->handleNewWindow(newdlg);
  }
}

void dspTimePhasedBookings::sPopulateMenu(QMenu *pMenu, QTreeWidgetItem *pSelected, int pColumn)
{
  QAction *menuItem;

  _column = pColumn;

  if (((XTreeWidgetItem *)pSelected)->id() != -1)
    menuItem = pMenu->addAction(tr("View Bookings..."), this, SLOT(sViewBookings()));
}

bool dspTimePhasedBookings::setParamsTP(ParameterList & params)
{
  if (_customer->isChecked())
    params.append("byCust");
  if (_prodcat->isChecked())
    params.append("byProdcat");
  if (_item->isChecked())
    params.append("byItem");

  parameterWidget()->appendValue(params);

   return true;
}

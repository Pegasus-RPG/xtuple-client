/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "productCategories.h"

#include <QVariant>
#include <QMessageBox>
#include <parameter.h>
//#include <QStatusBar>
#include <openreports.h>
#include "productCategory.h"

/*
 *  Constructs a productCategories as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
productCategories::productCategories(QWidget* parent, const char* name, Qt::WFlags fl)
    : XWidget(parent, name, fl)
{
    setupUi(this);

//    (void)statusBar();

    // signals and slots connections
    connect(_print, SIGNAL(clicked()), this, SLOT(sPrint()));
    connect(_new, SIGNAL(clicked()), this, SLOT(sNew()));
    connect(_edit, SIGNAL(clicked()), this, SLOT(sEdit()));
    connect(_prodcat, SIGNAL(populateMenu(QMenu*,QTreeWidgetItem*,int)), this, SLOT(sPopulateMenu(QMenu*)));
    connect(_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(_delete, SIGNAL(clicked()), this, SLOT(sDelete()));
    connect(_deleteUnused, SIGNAL(clicked()), this, SLOT(sDeleteUnused()));
    connect(_view, SIGNAL(clicked()), this, SLOT(sView()));
    connect(_prodcat, SIGNAL(valid(bool)), _view, SLOT(setEnabled(bool)));
    init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
productCategories::~productCategories()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void productCategories::languageChange()
{
    retranslateUi(this);
}

//Added by qt3to4:
#include <QMenu>


void productCategories::init()
{
//  statusBar()->hide();
  
  _prodcat->addColumn(tr("Category"),    70, Qt::AlignLeft, true, "prodcat_code" );
  _prodcat->addColumn(tr("Description"), -1, Qt::AlignLeft, true, "prodcat_descrip" );

  if (_privileges->check("MaintainProductCategories"))
  {
    connect(_prodcat, SIGNAL(valid(bool)), _edit, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(valid(bool)), _delete, SLOT(setEnabled(bool)));
    connect(_prodcat, SIGNAL(itemSelected(int)), _edit, SLOT(animateClick()));
  }
  else
  {
    connect(_prodcat, SIGNAL(itemSelected(int)), _view, SLOT(animateClick()));

    _new->setEnabled(FALSE);
    _deleteUnused->setEnabled(FALSE);
  }

  sFillList(-1);
}

void productCategories::sDelete()
{
  q.prepare("SELECT deleteProductCategory(:prodcat_id) AS result;");
  q.bindValue(":prodcat_id", _prodcat->id());
  q.exec();
  if (q.first())
  {
    switch (q.value("result").toInt())
    {
      case -1:
        QMessageBox::warning( this, tr("Cannot Delete Product Category"),
                              tr( "You cannot delete the selected Product Category because there are currently items assigned to it.\n"
                                  "You must first re-assign these items before deleting the selected Product Category." ) );
        return;
    }

    sFillList(-1);
  }
  else
    systemError(this, tr("A System Error occurred at %1::%2.")
                      .arg(__FILE__)
                      .arg(__LINE__) );
}

void productCategories::sNew()
{
  ParameterList params;
  params.append("mode", "new");

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void productCategories::sEdit()
{
  ParameterList params;
  params.append("mode", "edit");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  
  int result = newdlg.exec();
  if (result != XDialog::Rejected)
    sFillList(result);
}

void productCategories::sView()
{
  ParameterList params;
  params.append("mode", "view");
  params.append("prodcat_id", _prodcat->id());

  productCategory newdlg(this, "", TRUE);
  newdlg.set(params);
  newdlg.exec();
}

void productCategories::sPopulateMenu( QMenu * menu )
{
  int menuItem;

  menuItem = menu->insertItem("Edit Product Cateogry...", this, SLOT(sEdit()), 0);
  if (!_privileges->check("MaintainProductCategories"))
    menu->setItemEnabled(menuItem, FALSE);

  menuItem = menu->insertItem("Delete Product Category...", this, SLOT(sDelete()), 0);
  if (!_privileges->check("MaintainProductCategories"))
    menu->setItemEnabled(menuItem, FALSE);
}

void productCategories::sPrint()
{
  orReport report("ProductCategoriesMasterList");
  if (report.isValid())
    report.print();
  else
    report.reportError(this);
}

void productCategories::sDeleteUnused()
{
  if ( QMessageBox::warning( this, tr("Delete Unused Product Categories"),
                             tr("Are you sure that you wish to delete all unused Product Categories?"),
                             tr("&Yes"), tr("&No"), QString::null, 0, 1 ) == 0 )
  {
    q.exec("SELECT deleteUnusedProductCategories() AS result;");
    sFillList(-1);
  }
}

void productCategories::sFillList(int pId)
{
  _prodcat->populate( "SELECT prodcat_id, prodcat_code, prodcat_descrip "
                      "FROM prodcat "
                      "ORDER BY prodcat_code;", pId );
}


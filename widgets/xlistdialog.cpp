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

#include "xlistdialog.h"

#include <qpushbutton.h>
#include <qlabel.h>
#include <q3frame.h>
#include <qlayout.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "xlistview.h"

XListDialog::XListDialog( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) :
  QDialog( parent, name, modal, fl )
{
  if ( !name )
    setName( "XListDialog" );

  Q3VBoxLayout * XListDialogLayout = new Q3VBoxLayout( this, 11, 6, "XListDialogLayout"); 

  Q3HBoxLayout * layout13 = new Q3HBoxLayout( 0, 0, 6, "layout13"); 

  _altFrame = new Q3Frame( this, "_altFrame" );
  _altFrame->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)1, 0, 0, _altFrame->sizePolicy().hasHeightForWidth() ) );
  _altFrame->setFrameShape( Q3Frame::NoFrame );
  _altFrame->setFrameShadow( Q3Frame::Raised );
  layout13->addWidget( _altFrame );

  Q3VBoxLayout * layout12 = new Q3VBoxLayout( 0, 0, 6, "layout12"); 

  _close = new QPushButton( tr( "&Close" ), this, "_close" );
  _close->setAccel( QKeySequence( tr( "Alt+C" ) ) );
  layout12->addWidget( _close );

  _select = new QPushButton( tr( "&Select" ), this, "_select" );
  _select->setAccel( QKeySequence( tr( "Alt+S" ) ) );
  _select->setEnabled(FALSE);
  layout12->addWidget( _select );
  QSpacerItem* spacer = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Minimum );
  layout12->addItem( spacer );
  layout13->addLayout( layout12 );
  XListDialogLayout->addLayout( layout13 );

  _listLabel = new QLabel( tr( "List:" ), this, "_listLabel" );
  XListDialogLayout->addWidget( _listLabel );

  _list = new XListView( this, "_list" );
  XListDialogLayout->addWidget( _list );

  setCaption( tr( "XListDialog" ) );
  resize( QSize(420, 380).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  setTabOrder(_list, _select);
  setTabOrder(_select, _close);
  setTabOrder(_close, _list);
  _select->setFocus();

  // signals and slots connections
  connect( _close,  SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _list,   SIGNAL( itemSelected(int) ), _select, SLOT( animateClick() ) );
  connect( _list,   SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );
}

void XListDialog::sClose()
{
  reject();
}

void XListDialog::sSelect()
{
  accept();
}

void XListDialog::sFillList()
{
}


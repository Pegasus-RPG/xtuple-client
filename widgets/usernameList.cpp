/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */


#include "usernameList.h"

#include <QVariant>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>

#include <parameter.h>

#include "usernamecluster.h"
#include "xtreewidget.h"

usernameList::usernameList( QWidget* parent, const char* name, bool modal, Qt::WFlags fl ) : QDialog( parent, name, modal, fl )
{
  if ( !name )
    setObjectName( "usernameList" );

  _id = -1;
  _type = UsernameLineEdit::UsersAll;

  setWindowTitle(tr("Username List"));

  QVBoxLayout *usernameListLayout = new QVBoxLayout( this, 5, 5, "usernameListLayout"); 
  QHBoxLayout *Layout69 = new QHBoxLayout( 0, 0, 0, "Layout69"); 
  QHBoxLayout *Layout72 = new QHBoxLayout( 0, 0, 7, "Layout72"); 
  QVBoxLayout *Layout71 = new QVBoxLayout( 0, 0, 0, "Layout71"); 
  QVBoxLayout *Layout70 = new QVBoxLayout( 0, 0, 5, "Layout70"); 
  QHBoxLayout *Layout5 = new QHBoxLayout( 0, 0, 5, "Layout5"); 
  QVBoxLayout *Layout18 = new QVBoxLayout( 0, 0, 5, "Layout18"); 
  QVBoxLayout *Layout20 = new QVBoxLayout( 0, 0, 0, "Layout20"); 

  QLabel *_searchForLit = new QLabel(tr("S&earch for:"), this, "_searchForLit");
  _searchForLit->setAlignment( int( Qt::AlignVCenter | Qt::AlignRight ) );
  Layout5->addWidget( _searchForLit );

  _searchFor = new QLineEdit( this, "_searchFor" );
  _searchForLit->setBuddy(_searchFor);
  Layout5->addWidget( _searchFor );
  Layout70->addLayout( Layout5 );

  QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
  Layout69->addItem( spacer );
  Layout70->addLayout( Layout69 );
  Layout71->addLayout( Layout70 );

  QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred );
  Layout71->addItem( spacer_2 );
  Layout72->addLayout( Layout71 );

  _close = new QPushButton(tr("&Cancel"), this, "_close");
  Layout18->addWidget( _close );

  _select = new QPushButton(tr("&Select"), this, "_select");
  _select->setEnabled( FALSE );
  _select->setAutoDefault( TRUE );
  _select->setDefault( TRUE );
  Layout18->addWidget( _select );
  Layout72->addLayout( Layout18 );
  usernameListLayout->addLayout( Layout72 );

  QLabel *_usernamesLit = new QLabel(tr("&Usernames:"), this, "_usernamesLit");
  Layout20->addWidget( _usernamesLit );

  _user = new XTreeWidget(this);
  _user->setObjectName("_user");
  _usernamesLit->setBuddy(_user);
  Layout20->addWidget(_user);
  usernameListLayout->addLayout( Layout20 );

  resize( QSize(467, 393).expandedTo(minimumSizeHint()) );
  //clearWState( WState_Polished );

  connect( _user, SIGNAL( itemSelected(int) ), this, SLOT( sSelect() ) );
  connect( _select, SIGNAL( clicked() ), this, SLOT( sSelect() ) );
  connect( _close, SIGNAL( clicked() ), this, SLOT( sClose() ) );
  connect( _searchFor, SIGNAL( textChanged(const QString&) ), this, SLOT( sSearch(const QString&) ) );
  connect( _user, SIGNAL( valid(bool) ), _select, SLOT( setEnabled(bool) ) );

  _user->addColumn(tr("Username"), 100, Qt::AlignLeft, true, "usr_username");
  _user->addColumn(tr("Real Name"), -1, Qt::AlignLeft, true, "usr_propername");
}

void usernameList::set(ParameterList &pParams)
{
  QVariant param;
  bool     valid;

  param = pParams.value("id", &valid);
  if (valid)
    _id = param.toInt();
  else
    _id = -1;

  param = pParams.value("type", &valid);
  if (valid)
  {
    _type = param.toUInt();
    if(UsernameLineEdit::UsersActive)
      setWindowTitle(tr("Active Usernames"));
    else if(UsernameLineEdit::UsersInactive)
      setWindowTitle(tr("Inactive Usernames"));
  }
  else
    _type = UsernameLineEdit::UsersAll;

  param = pParams.value("caption", &valid);
  if (valid)
    setWindowTitle(param.toString());

  sFillList();
}

void usernameList::sClose()
{
  done(_id);
}

void usernameList::sSelect()
{
  done(_user->id());
}

void usernameList::sSearch(const QString &pTarget)
{
  _user->clearSelection();
  int i;
  for (i = 0; i < _user->topLevelItemCount(); i++)
  {
    if (_user->topLevelItem(i)->text(0).startsWith(pTarget, Qt::CaseInsensitive))
      break;
  }

  if (i < _user->topLevelItemCount())
  {
    _user->setCurrentItem(_user->topLevelItem(i));
    _user->scrollToItem(_user->topLevelItem(i));
  }
}

void usernameList::sFillList()
{
  QString sql("SELECT usr_id, usr_username, usr_propername"
              "  FROM usr");

  if(UsernameLineEdit::UsersActive)
    sql += " WHERE (usr_active)";
  else if(UsernameLineEdit::UsersInactive)
    sql += " WHERE (NOT usr_active)";

  sql += " ORDER BY usr_username;";

  _user->populate(sql, _id);
}

void usernameList::reject()
{
  done(_id);
}

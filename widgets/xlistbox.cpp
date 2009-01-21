/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xlistbox.h"

XListBoxText::XListBoxText( Q3ListBox* listbox, const QString & text, int id )
  : Q3ListBoxText(listbox, text)
{
  _id = id;
}

XListBoxText::XListBoxText( const QString & text, int id )
  : Q3ListBoxText(text)
{
  _id = id;
}

XListBoxText::XListBoxText( Q3ListBox* listbox, const QString & text, Q3ListBoxItem *after )
  : Q3ListBoxText(listbox, text, after)
{
  _id = -1;
}

XListBoxText::XListBoxText( Q3ListBox* listbox, const QString & text, int id, Q3ListBoxItem *after )
  : Q3ListBoxText(listbox, text, after)
{
  _id = id;
}

XListBoxText::~XListBoxText()
{
}

void XListBoxText::setId(int id)
{
  _id = id;
}

int XListBoxText::RTTI = 8152005;
int XListBoxText::rtti() const
{
  return RTTI;
}

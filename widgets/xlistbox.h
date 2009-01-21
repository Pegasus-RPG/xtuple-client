/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef __XLISTBOX_H__
#define __XLISTBOX_H__

#include <Q3ListBox>

class XListBoxText : public Q3ListBoxText
{
  public:
    XListBoxText( Q3ListBox* listbox, const QString & text=QString::null, int id=-1 );
    XListBoxText( const QString & text=QString::null, int id=-1 );
    XListBoxText( Q3ListBox* listbox, const QString & text, Q3ListBoxItem *after );
    XListBoxText( Q3ListBox* listbox, const QString & text, int id, Q3ListBoxItem *after );
    virtual ~XListBoxText();

    void setId(int id);
    int id() const { return _id; }

    virtual int rtti() const;
    static int RTTI;

  private:
    int _id;
};

#endif


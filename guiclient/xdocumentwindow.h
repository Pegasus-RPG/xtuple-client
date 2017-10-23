/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */
#ifndef __XDOCUMENTWINDOW_H__
#define __XDOCUMENTWINDOW_H__

#include "xwidget.h"

class XDocumentWindowPrivate;

class XDocumentWindow : public XWidget
{
  Q_OBJECT

  public:
    XDocumentWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);
    XDocumentWindow(QWidget * parent, const char * name, Qt::WindowFlags flags = 0);
    ~XDocumentWindow();

    Q_INVOKABLE QWidget * widget();

  private:
    XDocumentWindowPrivate * _data;
};

#endif // __XDOCUMENTWINDOW_H__

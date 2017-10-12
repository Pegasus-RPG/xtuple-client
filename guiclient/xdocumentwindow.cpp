/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2017 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xdocumentwindow.h"
#include "ui_xdocumentwindow.h"

class XDocumentWindowPrivate : public Ui::XDocumentWindow
{
public:
  XDocumentWindowPrivate(::XDocumentWindow * parent) : _parent(parent)
  {
    setupUi(_parent);
  }

private:
  ::XDocumentWindow * _parent;
};

XDocumentWindow::XDocumentWindow(QWidget *parent, Qt::WindowFlags flags)
  : XWidget(parent, flags)
{
  _data = new XDocumentWindowPrivate(this);
}

XDocumentWindow::XDocumentWindow(QWidget * parent, const char * name, Qt::WindowFlags flags)
  : XWidget(parent, name, flags)
{
  _data = new XDocumentWindowPrivate(this);
}

XDocumentWindow::~XDocumentWindow()
{
  delete _data;
  _data = 0;
}

QWidget* XDocumentWindow::widget()
{
  return _data->_widget;
}

void XDocumentWindow::closeDocument()
{
  close();
}

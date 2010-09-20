/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "xabstractconfigure.h"

/** \brief Abstract interface definition for xTuple ERP Configure widgets.

  XAbstractConfigure is a pure virtual (abstract) class that defines
  a simple programming interface that all configuration windows
  must implement. This is used by the setup window to ensure that all data
  get saved properly.

  \see setup

*/

XAbstractConfigure::XAbstractConfigure(QWidget* parent, Qt::WindowFlags f)
  : XWidget(parent, f)
{
}

XAbstractConfigure::~XAbstractConfigure()
{
}

/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef OPPORTUNITYLIST_H
#define OPPORTUNITYLIST_H

#include "guiclient.h"
#include "xwidget.h"
#include <parameter.h>

#include "ui_opportunityList.h"

class opportunityList : public XWidget, public Ui::opportunityList
{
  Q_OBJECT

  public:
    opportunityList(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = Qt::Window);

  public slots:
    virtual void	languageChange();
    virtual SetResponse	set(const ParameterList&);
    virtual void	sClose();
    virtual void	sDelete();
    virtual void	sEdit();
    virtual void	sFillList();
    virtual void	sNew();
    virtual void	sPopulateMenu(QMenu*);
    virtual void	sPrint();
    virtual void	sView();
    virtual void 	setParams(ParameterList &);

  private:
    int		_mode;
    int		_myUsrId;
};

#endif // OPPORTUNITYLIST_H

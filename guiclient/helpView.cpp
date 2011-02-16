/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QWidget>
#include <QGridLayout>
#include <QUrl>
#include <QHelpContentWidget>

#include <QDebug>

#include "guiclient.h"
#include "xtHelp.h"
#include "helpViewBrowser.h"
#include "helpView.h"

static helpView *helpViewSingleton = 0;

helpView* helpView::getInstance(QWidget *parent)
{
  if(!helpViewSingleton)
    helpViewSingleton = new helpView(parent);
  return helpViewSingleton;
}

helpView::helpView(QWidget *parent)
  : QDockWidget(tr("xTuple Help Documentation"), parent)
{
  setObjectName("helpView");

  _layoutContainer = new QWidget(this);
  _layout = new QGridLayout;
  _helpBrowser = new helpViewBrowser(this);
  _help = xtHelp::getInstance(this);

  connect(_help->contentWidget(),       SIGNAL(linkActivated(const QUrl&)),                     this,   SLOT(sIndexChanged(const QUrl&)));
  connect(this,                         SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),        this,   SLOT(sLocationChanged(Qt::DockWidgetArea)));

  _layout->addWidget(_help->contentWidget(),0,0);
  if(_help->isOnline())
  {
    _help->contentWidget()->hide();
    sIndexChanged(QUrl("index.html"));
  }
  _layout->addWidget(_helpBrowser,1,0);
  _layoutContainer->setLayout(_layout);

  setWidget(_layoutContainer);

  omfgThis->addDockWidget(Qt::TopDockWidgetArea, this);
}

helpView::~helpView()
{
}

void helpView::sIndexChanged(const QUrl& index)
{
  _helpBrowser->setSource(index);
  _helpBrowser->show();
}

void helpView::sLocationChanged(Qt::DockWidgetArea area)
{
  bool redraw = false;
  if(
      area == Qt::RightDockWidgetArea ||
      area == Qt::LeftDockWidgetArea
    )
  {
    _layout->removeWidget(_helpBrowser);
    _layout->addWidget(_helpBrowser,1,0);
    redraw = true;
  }
  else if(
      area == Qt::TopDockWidgetArea ||
      area == Qt::BottomDockWidgetArea
    )
  {
    _layout->removeWidget(_helpBrowser);
    _layout->addWidget(_helpBrowser,0,1);
    redraw = true;
  }
  if(redraw)
  {
    show();
  }
}

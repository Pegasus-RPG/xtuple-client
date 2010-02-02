/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef xtupleplugin_h
#define xtupleplugin_h

#define cNew                  1
#define cEdit                 2
#define cView                 3

#include <metrics.h>

#include "parameter.h"

#include <QtDesigner/QtDesigner>
#include <QString>
#include <QIcon>

#ifdef Q_WS_WIN
  #ifdef MAKEDLL
    #define XTUPLEWIDGETS_EXPORT __declspec(dllexport)
  #else
    #define XTUPLEWIDGETS_EXPORT
  #endif
#else
  #define XTUPLEWIDGETS_EXPORT
#endif

class Preferences;
class Metrics;
class QWorkspace;
class Privileges;
class QWidget;

extern Preferences *_x_preferences;
extern Metrics     *_x_metrics;
extern QWorkspace  *_x_workspace;
extern Privileges  *_x_privileges;

class XTUPLEWIDGETS_EXPORT GuiClientInterface
{
  public:
    virtual ~GuiClientInterface() {}
    virtual QDialog* openDialog(const QString pname, ParameterList pparams, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0) = 0;
};

class xTuplePlugin : public QObject, public QDesignerCustomWidgetCollectionInterface
{
  Q_OBJECT
  Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

  public:
    xTuplePlugin(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

  private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

void XTUPLEWIDGETS_EXPORT initializePlugin(Preferences *, Metrics *, Privileges *, QWorkspace *);

#endif


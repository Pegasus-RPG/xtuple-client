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
 * The Original Code is xTuple ERP: PostBooks Edition 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2008 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2008 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by xTuple ERP: PostBooks Edition
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

#ifndef __SCRIPTTOOLBOX_H__
#define __SCRIPTTOOLBOX_H__

#include <QObject>
#include <QVariant>
#include <QtScript>

#include <parametergroup.h>

#include "parameter.h"
#include "guiclient.h"

#include "addresscluster.h"     // for AddressCluster::SaveFlags

class QWidget;
class QLayout;
class QGridLayout;
class QBoxLayout;
class QStackedLayout;
class QScriptEngine;

/* TODO: remove this enum and use AddressCluster::SaveFlags directly
   for some reason working with AddressCluster::SaveFlags failed but this works.
 */
enum SaveFlags
{ CHECK = AddressCluster::CHECK,
  CHANGEONE = AddressCluster::CHANGEONE,
  CHANGEALL = AddressCluster::CHANGEALL
};

Q_DECLARE_METATYPE(ParameterList)
Q_DECLARE_METATYPE(XSqlQuery);
Q_DECLARE_METATYPE(enum SetResponse)
Q_DECLARE_METATYPE(enum ParameterGroup::ParameterGroupStates);
Q_DECLARE_METATYPE(enum ParameterGroup::ParameterGroupTypes);
Q_DECLARE_METATYPE(enum Qt::WindowModality);
Q_DECLARE_METATYPE(enum SaveFlags);

QScriptValue ParameterListtoScriptValue(QScriptEngine *engine, const ParameterList &params);
void ParameterListfromScriptValue(const QScriptValue &obj, ParameterList &params);

QScriptValue XSqlQuerytoScriptValue(QScriptEngine *engine, const XSqlQuery &qry);
void XSqlQueryfromScriptValue(const QScriptValue &obj, XSqlQuery &qry);

QScriptValue SetResponsetoScriptValue(QScriptEngine *engine, const enum SetResponse &sr);
void SetResponsefromScriptValue(const QScriptValue &obj, enum SetResponse &sr);

QScriptValue ParameterGroupStatestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupStates &en);
void ParameterGroupStatesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupStates &en);

QScriptValue ParameterGroupTypestoScriptValue(QScriptEngine *engine, const enum ParameterGroup::ParameterGroupTypes &en);
void ParameterGroupTypesfromScriptValue(const QScriptValue &obj, enum ParameterGroup::ParameterGroupTypes &en);

QScriptValue QtWindowModalitytoScriptValue(QScriptEngine *engine, const enum Qt::WindowModality &en);
void QtWindowModalityfromScriptValue(const QScriptValue &obj, enum Qt::WindowModality &en);

QScriptValue SaveFlagstoScriptValue(QScriptEngine *engine, const enum SaveFlags &en);
void SaveFlagsfromScriptValue(const QScriptValue &obj, enum SaveFlags &en);

class ScriptToolbox : public QObject
{
  Q_OBJECT

  public:
    ScriptToolbox(QScriptEngine * engine);
    virtual ~ScriptToolbox();

    static QScriptValue variantToScriptValue(QScriptEngine *, QVariant);
    static void setLastWindow(QWidget * lw);

  public slots:

    QObject * executeQuery(const QString & query);
    QObject * executeQuery(const QString & query, const ParameterList & params);
    QObject * executeDbQuery(const QString & group, const QString & name);
    QObject * executeDbQuery(const QString & group, const QString & name, const ParameterList & params);
    QObject * executeBegin();
    QObject * executeCommit();
    QObject * executeRollback();
    
    QObject * qtyVal();
    QObject * TransQtyVal();
    QObject * qtyPerVal();
    QObject * percentVal();
    QObject * moneyVal();
    QObject * negMoneyVal();
    QObject * priceVal();
    QObject * costVal();
    QObject * ratioVal();
    QObject * weightVal();
    QObject * runTimeVal();
    QObject * orderVal();
    QObject * dayVal();

    QObject * widgetGetLayout(QWidget * w);

    QObject * createGridLayout();

    void layoutGridAddLayout(QObject *, QObject *, int row, int column, int alignment = 0);

    void layoutBoxInsertWidget(QObject *, int index, QWidget *, int stretch = 0, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int row, int column, int alignment = 0);
    void layoutGridAddWidget(QObject *, QWidget *, int fromRow, int fromColumn, int rowSpan, int columnSpan, int alignment = 0);
    void layoutStackedInsertWidget(QObject *, int index, QWidget *);

    QObject * menuAddAction(QObject * menu, const QString & text);
    QObject * menuAddMenu(QObject * menu, const QString & text, const QString & name = QString());

    QWidget * tabWidget(QWidget * tab, int idx);
    int       tabInsertTab(QWidget * tab, int idx, QWidget * page, const QString & text);
    void      tabRemoveTab(QWidget * tab, int idx);
    void      tabSetTabText(QWidget * tab, int idx, const QString & text);
    QString   tabtabText(QWidget * tab, int idx);

    QWidget * createWidget(const QString & className, QWidget * parent = 0, const QString & name = QString());
    QObject * createLayout(const QString & className, QWidget * parent, const QString & name = QString());
    QWidget * loadUi(const QString & screenName, QWidget * parent = 0);

    QWidget * lastWindow() const;
    QWidget * openWindow(const QString pname, QWidget *parent = 0, Qt::WindowModality modality = Qt::NonModal, Qt::WindowFlags flags = 0);

    void addColumnXTreeWidget(QWidget * tree, const QString &, int, int, bool = true, const QString = QString(), const QString = QString());
    void populateXTreeWidget(QWidget * tree, QObject * pSql, bool = FALSE);
    
    void loadQWebView(QWidget * webView, const QString & url);

    bool printReport(const QString & name, const ParameterList & params, const QString & pdfFilename = QString::null);

    bool coreDisconnect(QObject * sender, const QString & signal, QObject * receiver, const QString & method);

    QString fileDialog(QWidget * parent, const QString & caption, const QString & dir, const QString & filter, int fileModeSel, int acceptModeSel);
    void openUrl(const QString & fileUrl);
    bool copyFile(const QString & oldName, const QString & newName);
    QString getFileName(const QString & path);
    bool renameFile(const QString & oldName, const QString & newName);
    bool removeFile(const QString & name);
    bool fileExists(const QString & name);
    QString getCurrentDir();
    QString getHomeDir();
    void    listProperties(const QScriptValue &obj) const;
    bool    makePath(const QString & mkPath, const QString & rootPath);
    int     messageBox(const QString & type, QWidget * parent, const QString & title, const QString & text, int buttons = 0x00000400, int defaultButton = 0x00000000);
    bool    removePath(const QString & rmPath, const QString & rootPath);
    QString rootPath();
    QString textStreamRead(const QString & name);
    bool    textStreamWrite(const QString & name, const QString & WriteText);

    int     saveCreditCard(QWidget *parent,
                              int custId,
                              QString ccName, 
                              QString ccAddress1, 
                              QString ccAddress2,
                              QString ccCity, 
                              QString ccState, 
                              QString ccZip, 
                              QString ccCountry,
                              QString ccNumber,
                              QString ccType,
                              QString ccExpireMonth,
                              QString ccExpireYear,
                              int ccId = 0,
                              bool ccActive = true );
    QObject *getCreditCardProcessor();
    

  private:
    QScriptEngine * _engine;
    static QWidget * _lastWindow;
};

#endif // __SCRIPTTOOLBOX_H__

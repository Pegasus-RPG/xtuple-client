/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef GUICLIENT_H
#define GUICLIENT_H

#include <QDate>
#include <QDateTime>
#include <QMainWindow>
#include <QTimer>
#include <QAction>
#include <QCloseEvent>
#include <QList>
#include <QPixmap>
#include <QMenu>
#include <QMenuBar>

#include <xsqlquery.h>

#include "../common/format.h"

class QSplashScreen;
class QWorkspace;
class QPushButton;
class QIntValidator;
class QDoubleValidator;
class QCheckBox;
class QAssistantClient;
class QScriptEngine;

class menuProducts;
class menuInventory;
class menuSchedule;
class menuPurchase;
class menuManufacture;
class menuCRM;
class menuSales;
class menuAccounting;
class menuSystem;

class TimeoutHandler;
class InputManager;
class ReportHandler;

class XMainWindow;
class QMainWindow;
class XWidget;

#define q omfgThis->_q

#define cNew                  1
#define cEdit                 2
#define cView                 3
#define cCopy                 4
#define cRelease              5
#define cPost                 6
#define cReplace              7

#define cPostedCounts         0x01
#define cUnpostedCounts       0x02
#define cAllCounts            0x04

//  Possible Location/Itemloc Sources
#define cLocation             0x01
#define cItemloc              0x02

//  Possible Transactions Groups
#define cTransAll             0xFF
#define cTransReceipts        0x01
#define cTransIssues          0x02
#define cTransShipments       0x04
#define cTransAdjCounts       0x08
#define cTransTransfers       0x10
#define cTransScraps          0x20

//  Possible Journal Types
#define SalesJournal          0x01
#define CreditMemoJournal     0x02
#define CashReceiptsJournal   0x03
#define PayablesJournal       0x04
#define CheckJournal          0x05

// Possible return values from submitReport
#define cNoReportDefinition   -2

int  systemError(QWidget *, const QString &);
int  systemError(QWidget *, const QString &, const QString &, const int);
void message(const QString &, int = 0);
void resetMessage();
void audioAccept();
void audioReject();

extern bool _evaluation;

extern QSplashScreen *_splash;

#include "../common/metrics.h"
extern Metrics     *_metrics;
extern Preferences *_preferences;
extern Privileges  *_privileges;
#include "../common/metricsenc.h"
extern Metricsenc  *_metricsenc;


enum SetResponse
{
  NoError, NoError_Cancel, NoError_Run, NoError_Print, NoError_Submit,
  Error_NoSetup, UndefinedError
};


class ActionSet;

class Action : public QAction
{
  friend class ActionSet;

  public:
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *);  
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, bool,
            const QPixmap &, QWidget *,
            const QString &); 

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString & );

    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap &, QWidget *);  
            
    Action( QWidget *, const char *, const QString &,
            QObject *, const char *,
            QWidget *, const QString &,
            const QPixmap &, QWidget *,
            const QString &); 

    inline const QString name()        { return _name;        };
    inline const QString displayName() { return _displayName; };
    inline const QString toolTip() { return _toolTip; };

  protected:
    QString _name;
    QString _displayName;
    QString _toolTip;

  private:
    void init( QWidget *, const char *, const QString &,
               QObject *, const char *,
               QWidget *, const QString & );
};

class ActionSet : public QList<Action *>
{
};

class GUIClient : public QMainWindow
{
  friend class XWidget;
  friend class XMainWindow;
  friend class XDialog;

  Q_OBJECT

  Q_PROPERTY(QString key READ key)
  
  public:
    GUIClient(const QString &, const QString &);

    void setCaption();
    void initMenuBar();
    void saveToolbarPositions();

    inline QWorkspace *workspace()         { return _workspace;    }
    inline InputManager *inputManager()    { return _inputManager; }
    inline QString databaseURL()           { return _databaseURL;  }
    inline QString username()              { return _username;     }

    inline const QDate &startOfTime()      { return _startOfTime;  }
    inline const QDate &endOfTime()        { return _endOfTime;    }
    inline const QDate &dbDate()           { return _dbDate;       }

    inline QDoubleValidator *qtyVal()      { return _qtyVal;       }
    inline QDoubleValidator *transQtyVal() { return _transQtyVal;  }
    inline QDoubleValidator *qtyPerVal()   { return _qtyPerVal;    }
    inline QDoubleValidator *scrapVal()    { return _scrapVal;     } 
    inline QDoubleValidator *percentVal()  { return _percentVal;   }
    inline QDoubleValidator *negPercentVal()  { return _negPercentVal;   }
    inline QDoubleValidator *moneyVal()    { return _moneyVal;     }
    inline QDoubleValidator *negMoneyVal() { return _negMoneyVal;  }
    inline QDoubleValidator *priceVal()    { return _priceVal;     }
    inline QDoubleValidator *costVal()     { return _costVal;      }
    inline QDoubleValidator *ratioVal()    { return _ratioVal;     }
    inline QDoubleValidator *weightVal()   { return _weightVal;    }
    inline QDoubleValidator *runTimeVal()  { return _runTimeVal;   }
    inline QIntValidator *orderVal()       { return _orderVal;     }
    inline QIntValidator *dayVal()         { return _dayVal;       }

    inline QFont systemFont()              { return *_systemFont;  }
    inline QFont fixedFont()               { return *_fixedFont;   }

    bool singleCurrency();
    bool showTopLevel() const { return _showTopLevel; }
    QWidgetList windowList();
    void populateCustomMenu(QMenu*, const QString &);

    void handleNewWindow(QWidget *, Qt::WindowModality = Qt::NonModal);
    QMenuBar *menuBar();

    XSqlQuery        _q;
    ActionSet        actions;
    XSqlQuery        __item;
    int              __itemListSerial;
    XSqlQuery        __cust;
    int              __custListSerial;
    TimeoutHandler   *_timeoutHandler;
    ReportHandler    *_reportHandler;
    QAssistantClient *_assClient;

    QMap<const QObject*,int> _customCommands;

    QString _key;
    QString key() { return _key; }

    QString _singleWindow;

    void launchBrowser(QWidget*, const QString &);
    QWidget * myActiveWindow();

    void loadScriptGlobals(QScriptEngine * engine);

  public slots:
    void sReportError(const QString &);
    void sTick();
    void sChecksUpdated(int, int, bool);
    void sAssortmentsUpdated(int, bool);
    void sBBOMsUpdated(int, bool);
    void sBOMsUpdated(int, bool);
    void sBOOsUpdated(int, bool);
    void sBudgetsUpdated(int, bool);
    void sBankAdjustmentsUpdated(int, bool);
    void sBillingSelectionUpdated(int, int);
    void sCashReceiptsUpdated(int, bool);
    void sConfigureGLUpdated();
    void sCreditMemosUpdated();
    void sCrmAccountsUpdated(int);
    void sCustomCommand();
    void sCustomersUpdated(int, bool);
    void sGlSeriesUpdated();
    void sInvoicesUpdated(int, bool);
    void sItemGroupsUpdated(int, bool);
    void sItemsUpdated(int, bool);
    void sItemsitesUpdated();
    void sPaymentsUpdated(int, int, bool);
    void sProjectsUpdated(int);
    void sProspectsUpdated();
    void sPurchaseOrderReceiptsUpdated();
    void sPurchaseOrdersUpdated(int, bool);
    void sPurchaseRequestsUpdated();
    void sQOHChanged(int, bool);
    void sQuotesUpdated(int);
    void sReportsChanged(int, bool);
    void sReturnAuthorizationsUpdated();
    void sSalesOrdersUpdated(int);
    void sStandardPeriodsUpdated();
    void sTaxAuthsUpdated(int);
    void sTransferOrdersUpdated(int);
    void sVendorsUpdated();
    void sVouchersUpdated();
    void sWarehousesUpdated();
    void sWorkCentersUpdated();
    void sWorkOrderMaterialsUpdated(int, int, bool);
    void sWorkOrderOperationsUpdated(int, int, bool);
    void sWorkOrdersUpdated(int, bool);

    void sIdleTimeout();

    void sFocusChanged(QWidget* old, QWidget* now);

    void sNewErrorMessage();
    void setWindowTitle();

  signals:
    void tick();

    void checksUpdated(int, int, bool);
    void assortmentsUpdated(int, bool);
    void bankAdjustmentsUpdated(int, bool);
    void bbomsUpdated(int, bool);
    void billingSelectionUpdated(int, int);
    void bomsUpdated(int, bool);
    void boosUpdated(int, bool);
    void budgetsUpdated(int, bool);
    void cashReceiptsUpdated(int, bool);
    void configureGLUpdated();
    void creditMemosUpdated();
    void crmAccountsUpdated(int);
    void customersUpdated(int, bool);
    void glSeriesUpdated();
    void invoicesUpdated(int, bool);
    void itemGroupsUpdated(int, bool);
    void itemsUpdated(int, bool);
    void itemsitesUpdated();
    void paymentsUpdated(int, int, bool);
    void projectsUpdated(int);
    void prospectsUpdated();
    void purchaseOrderReceiptsUpdated();
    void purchaseOrdersUpdated(int, bool);
    void purchaseRequestsUpdated();
    void qohChanged(int, bool);
    void quotesUpdated(int, bool);
    void reportsChanged(int, bool);
    void returnAuthorizationsUpdated();
    void salesOrdersUpdated(int, bool);
    void standardPeriodsUpdated();
    void taxAuthsUpdated(int);
    void transferOrdersUpdated(int);
    void vendorsUpdated();
    void vouchersUpdated();
    void warehousesUpdated();
    void workCentersUpdated();
    void workOrderMaterialsUpdated(int, int, bool);
    void workOrderOperationsUpdated(int, int, bool);
    void workOrdersUpdated(int, bool);

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);

  protected slots:
    void windowDestroyed(QObject*);

  private:
    QWorkspace   *_workspace;
    QTimer       _tick;
    QPushButton  *_eventButton;
    QPushButton  *_registerButton;
    QPushButton  *_errorButton;
    QString      _databaseURL;
    QString      _username;
    bool         _showTopLevel;
    QWidgetList  _windowList;
    QMenuBar	*_menuBar;
    QWidget     *_activeWindow;

    InputManager   *_inputManager;

    menuProducts    *productsMenu;
    menuInventory   *inventoryMenu;
    menuSchedule    *scheduleMenu;
    menuPurchase    *purchaseMenu;
    menuManufacture *manufactureMenu;
    menuCRM         *crmMenu;
    menuSales       *salesMenu;
    menuAccounting  *accountingMenu;
    menuSystem      *systemMenu;

    QDate _startOfTime;
    QDate _endOfTime;
    QDate _dbDate;

    QDoubleValidator *_qtyVal;
    QDoubleValidator *_transQtyVal;
    QDoubleValidator *_qtyPerVal;
    QDoubleValidator *_scrapVal;
    QDoubleValidator *_percentVal;
    QDoubleValidator *_negPercentVal;
    QDoubleValidator *_moneyVal;
    QDoubleValidator *_negMoneyVal;
    QDoubleValidator *_priceVal;
    QDoubleValidator *_costVal;
    QDoubleValidator *_ratioVal;
    QDoubleValidator *_weightVal;
    QDoubleValidator *_runTimeVal;
    QIntValidator    *_orderVal;
    QIntValidator    *_dayVal;

    QFont *_systemFont;
    QFont *_fixedFont;

    bool _shown;
};
extern GUIClient *omfgThis;

#endif


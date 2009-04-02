/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef virtCluster_h
#define virtCluster_h

#include "widgets.h"
#include "parameter.h"
#include "xlineedit.h"
#include "xtreewidget.h"
#include "xcheckbox.h"
#include "xdatawidgetmapper.h"

#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class QGridLayout;
class VirtualClusterLineEdit;

class XTUPLEWIDGETS_EXPORT VirtualList : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualList();
        VirtualList(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSearch(const QString&);
        virtual void sSelect();

    protected:
        virtual void init();

        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        QLabel*      _titleLit;
        QPushButton* _close;
        QPushButton* _select;
        XTreeWidget* _listTab;
        int          _id;
};

class XTUPLEWIDGETS_EXPORT VirtualSearch : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sClose();
        virtual void sFillList();
        virtual void sSelect();

    protected:
        VirtualClusterLineEdit* _parent;
        QVBoxLayout* _dialogLyt;
        int  _id;
        QLabel*      _searchLit;
        QLineEdit*   _search;
        XCheckBox*   _searchNumber;
        XCheckBox*   _searchName;
        XCheckBox*   _searchDescrip;
        QLabel*      _titleLit;
        QPushButton* _close;
        QPushButton* _select;
        XTreeWidget* _listTab;
        QLayout*     dialogLyt;
        QLayout*     searchLyt;
        QLayout*     searchStrLyt;
        QGridLayout* selectorsLyt;
        QLayout*     tableLyt;
        QLayout*     buttonsLyt;
};

class XTUPLEWIDGETS_EXPORT VirtualInfo : public QDialog
{
    Q_OBJECT

    friend class VirtualClusterLineEdit;

    public:
        VirtualInfo(QWidget*, Qt::WindowFlags = 0);

    public slots:
        virtual void sPopulate();

    protected:
        VirtualClusterLineEdit* _parent;
        QLabel*         _titleLit;
        QLabel*         _numberLit;
        QLabel*         _number;
        QLabel*         _nameLit;
        QLabel*         _name;
        QLabel*         _descripLit;
        QLabel*         _descrip;
        QPushButton*    _close;

    private:
        int _id;
};

/*
    VirtualClusterLineEdit is an abstract class that encapsulates
    the basics of retrieving an ID given a NUMBER or a NUMBER given
    an ID.  This class and its subclasses are intended to be used
    by VirtualCluster objects, not directly by an application.

    Users of this class need to supply the names of the table, its
    id column, and its number column (numbers are user-readable
    strings such as customer abbreviations or purchase order numbers).

    Subclasses must supply implementations for the sInfo(), sList(),
    and sSearch() slots.  sInfo() is a distinct dialog that shows
    extended data from the record with table_id = id(). sList() and
    sSearch() are dialogs that let the user select a number/id pair
    value for the widget.
*/

class XTUPLEWIDGETS_EXPORT VirtualClusterLineEdit : public XLineEdit
{
    Q_OBJECT
    
    friend class VirtualCluster;
    friend class VirtualInfo;
    friend class VirtualList;
    friend class VirtualSearch;

    public:
        VirtualClusterLineEdit(QWidget*, const char*, const char*, const char*,
                               const char*, const char*, const char*,
                               const char* = 0);
        virtual int  id() const { return _id; }

    public slots:
        virtual void setId(const int);
        virtual void setNumber(const QString&);
        inline virtual QString extraClause() const { return _extraClause; };

    protected slots:
        virtual void clear();
        inline virtual void clearExtraClause() { _extraClause = ""; }

        virtual VirtualList*    listFactory();
        virtual VirtualSearch*  searchFactory();
        virtual VirtualInfo*    infoFactory();

        virtual void sList();
        virtual void sInfo();
        virtual void sSearch();
        virtual void sEllipses();
        virtual void sParse();

        virtual void setTitles(const QString&, const QString& = 0);
        inline virtual void setExtraClause(const QString& pExt) { _extraClause = pExt; };
        virtual void setTableAndColumnNames(const char* pTabName,
                                            const char* pIdColumn,
                                            const char* pNumberColumn,
                                            const char* pNameColumn,
                                            const char* pDescripColumn);

        void setStrict(bool);
        bool isStrict() const { return _strict; }

    signals:
        void newId(int);
        void parsed();
        void valid(bool);

    protected:
        QString _titleSingular;
        QString _titlePlural;
        QString _query;
        QString _idClause;
        QString _numClause;
        QString _extraClause;
        QString _name;
        QString _description;
        QString _idColName;
        QString _numColName;
        QString _nameColName;
        QString _descripColName;
        bool    _hasDescription;
        bool    _hasName;
        bool    _strict;
        virtual void silentSetId(const int);
        virtual void keyPressEvent(QKeyEvent *);

};

/*

    VirtualCluster is a widget that contains a VirtualClusterLineEdit
    and surrounds it with a label and two buttons.  One button lets
    the user invoke VirtualClusterLineEdit's sInfo slot while another
    button invokes either sSearch() or sList(), depending on the
    user's preferences.
    
    VirtualCluster provides a copy of much of the VirtualClusterLineEdit
    API to hide from the caller the fact that it contains one of these
    objects.  It also lets the calling code set it as read-only.

*/

class XTUPLEWIDGETS_EXPORT VirtualCluster : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString label          READ label          WRITE setLabel)
    Q_PROPERTY(bool    infoVisible    READ infoVisible    WRITE setInfoVisible)
    Q_PROPERTY(bool    listVisible    READ listVisible    WRITE setListVisible)
    Q_PROPERTY(bool    nameVisible    READ nameVisible    WRITE setNameVisible)
    Q_PROPERTY(bool    readOnly       READ readOnly       WRITE setReadOnly)
    Q_PROPERTY(QString fieldName      READ fieldName      WRITE setFieldName)
    Q_PROPERTY(QString number         READ number         WRITE setNumber         DESIGNABLE false)
    Q_PROPERTY(QString defaultNumber  READ defaultNumber  WRITE setDefaultNumber  DESIGNABLE false)

    friend class VirtualClusterLineEdit;

    public:
        VirtualCluster(QWidget*, const char* = 0);
        VirtualCluster(QWidget*, VirtualClusterLineEdit* = 0, const char* = 0);

        Q_INVOKABLE inline virtual int     id()             const { return _number->id(); };
        inline virtual bool    infoVisible()    const { return _info->isVisible(); };
        inline virtual bool    listVisible()    const { return _list->isVisible(); };
        inline virtual QString label()          const { return _label->text(); };
        inline virtual bool    nameVisible()    const { return _name->isVisible(); };
        inline virtual QString number()         const { return _number->text(); };
        inline virtual QString description()    const { return _description->text(); };
        inline virtual bool    isValid()        const { return _number->isValid(); };
        Q_INVOKABLE virtual QString name()           const { return _name->text(); };
        inline virtual bool    isStrict()       const { return _number->isStrict(); };
        inline virtual bool    readOnly()       const { return _readOnly; };
               virtual QString defaultNumber()  const { return _default; };
        inline virtual QString fieldName()      const { return _fieldName; };
        inline virtual QString extraClause()    const { return _number->extraClause(); };

    public slots:
        // most of the heavy lifting is done by VirtualClusterLineEdit _number
        inline virtual void clearExtraClause()                { _number->clearExtraClause(); };
        inline virtual void setDefaultNumber(const QString& p){ _default=p;};
        inline virtual void setDescription(const QString& p)  { _description->setText(p); };
        inline virtual void setExtraClause(const QString& p)  { _number->setExtraClause(p); };
        inline virtual void setFieldName(QString p)           { _fieldName = p; };
        inline virtual void setId(const int p)                { _number->setId(p); };
        inline virtual void setInfoVisible(const bool p)      { _info->setHidden(!p); };
        inline virtual void setListVisible(const bool p)      { _list->setHidden(!p); };
        inline virtual void setName(const QString& p)         { _name->setText(p); };
        inline virtual void setNameVisible(const bool p)      { _name->setHidden(!p); };
        inline virtual void setNumber(const int p)            { _number->setNumber(QString::number(p)); };
        inline virtual void setNumber(const QString& p)       { _number->setNumber(p); };

        virtual void clear();
        virtual void setDataWidgetMap(XDataWidgetMapper* m);
        virtual void setEnabled(const bool p);
        virtual void setLabel(const QString& p);
        virtual void setStrict(const bool b);
        virtual void setReadOnly(const bool b);
        virtual void sRefresh();
        virtual void updateMapperData();

    signals:
        void newId(int);
        void valid(bool);

    protected:
        virtual void addNumberWidget(VirtualClusterLineEdit* pNumberWidget);

        QGridLayout*            _grid;
        QLabel*                 _label;
        VirtualClusterLineEdit* _number;
        QPushButton*            _list;
        QPushButton*            _info;
        QLabel*                 _description;
        QLabel*                 _name;
        bool                    _readOnly;
	QString                 _fieldName;
        QString                 _default;
        XDataWidgetMapper       *_mapper;

    private:
        virtual void init();
};

#endif

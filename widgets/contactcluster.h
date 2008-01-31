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
 * The Original Code is PostBooks Accounting, ERP, and CRM Suite. 
 * 
 * The Original Developer is not the Initial Developer and is __________. 
 * If left blank, the Original Developer is the Initial Developer. 
 * The Initial Developer of the Original Code is OpenMFG, LLC, 
 * d/b/a xTuple. All portions of the code written by xTuple are Copyright 
 * (c) 1999-2007 OpenMFG, LLC, d/b/a xTuple. All Rights Reserved. 
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
 * Copyright (c) 1999-2007 by OpenMFG, LLC, d/b/a xTuple
 * 
 * Attribution Phrase: 
 * Powered by PostBooks, an open source solution from xTuple
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

#ifndef contactCluster_h
#define contactCluster_h

#include "OpenMFGWidgets.h"
#include "virtualCluster.h"
#include "xcombobox.h"
#include "addresscluster.h"
#include "crmacctcluster.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

class QGridLayout;
class ContactCluster;

class OPENMFGWIDGETS_EXPORT ContactList : public VirtualList
{
    Q_OBJECT

    friend class ContactCluster;
    friend class ContactInfo;
    friend class ContactSearch;

    public:
      ContactList(QWidget*, const char* = 0, bool = false, Qt::WFlags = 0);

    public slots:
      virtual void sFillList();
      virtual void sSearch(const QString&);

    protected:
      ContactCluster* _parent;
};

class OPENMFGWIDGETS_EXPORT ContactSearch : public VirtualSearch
{
    Q_OBJECT

    friend class ContactCluster;

    public:
	ContactSearch(QWidget*, Qt::WindowFlags = 0);

    public slots:
	virtual void sFillList();

    protected:
	ContactCluster*	_parent;
	QCheckBox*	_searchFirst;
	QCheckBox*	_searchLast;
	QCheckBox*	_searchTitle;
	QCheckBox*	_searchCRMAcct;
	QCheckBox*	_searchPhones;
	QCheckBox*	_searchEmail;
	QCheckBox*	_searchWebAddr;
	QCheckBox*	_searchInactive;

    private:
	int _id;
};

class OPENMFGWIDGETS_EXPORT ContactCluster : public VirtualCluster
{
    Q_OBJECT

    Q_PROPERTY(bool    accountVisible READ accountVisible WRITE setAccountVisible);
    Q_PROPERTY(bool    activeVisible READ activeVisible WRITE setActiveVisible);
    Q_PROPERTY(bool    addressVisible READ addressVisible WRITE setAddressVisible);
    Q_PROPERTY(bool    emailVisible  READ emailVisible  WRITE setEmailVisible);
    Q_PROPERTY(bool    initialsVisible  READ initialsVisible  WRITE setInitialsVisible);
    Q_PROPERTY(bool    minimalLayout READ minimalLayout WRITE setMinimalLayout);
    Q_PROPERTY(bool    phonesVisible READ phonesVisible WRITE setPhonesVisible);
    Q_PROPERTY(bool    webaddrVisible READ webaddrVisible WRITE setWebaddrVisible);

    friend class ContactInfo;
    friend class ContactList;
    friend class ContactSearch;

    public:
	// AccountLimits may be ORed together
	enum AccountLimits { Employee = 1,	Customer =  2,	Vendor     = 4,
			     Partner  = 8,	Prospect = 16,	Competitor = 32};

	ContactCluster(QWidget*, const char* = 0);
	virtual QString name() const;

	inline virtual bool    active()		const { return _active->isChecked(); };
	inline virtual bool    activeVisible()  const { return _active->isVisible(); };
	inline virtual bool   accountVisible()	const { return _crmAcct->isVisible(); };
	inline virtual bool    addressVisible() const { return _address->isVisible(); };
	inline virtual AddressCluster* addressWidget() const { return _address; };
	inline virtual int     addressId()	const { return _address->id(); };
	inline virtual int     crmAcctId()	const { return _crmAcct->id(); };
	inline virtual QString description()    const { return ""; };
	inline virtual QString emailAddress()	const { return _email->text(); };
	inline virtual bool    emailVisible()   const { return _email->isVisible(); };
	inline virtual QString fax()		const { return _fax->text(); };
	inline virtual QString first()		const { return _first->text(); };
	inline virtual QString honorific()	const { return _honorific->currentText(); };
	inline virtual int     id()	        const { return _id; };
	inline virtual QString initials()	const { return _initials->text(); };
	inline virtual bool   initialsVisible()	const { return _initials->isVisible(); };
	inline virtual bool    isValid()        const { return _valid; };
	inline virtual QString label()	        const { return _label->text(); };
	inline virtual QString last()		const { return _last->text(); };
	inline virtual bool    minimalLayout()  const { return _minimalLayout; };
	inline virtual QString notes()	        const { return _notes; };
	inline virtual QString phone()		const { return _phone->text(); };
	inline virtual QString phone2()		const { return _phone2->text(); };
	inline virtual bool    phonesVisible()  const { return _phone->isVisible(); };
	inline virtual int     searchAcct()	const { return _searchAcctId; };
	inline virtual QString webAddress()	const { return _webaddr->text(); };
	inline virtual bool    webaddrVisible() const { return _webaddr->isVisible(); };

    public slots:
	inline virtual void clearExtraClause()	{ };
	inline virtual void setExtraClause(const QString&) { };
	inline virtual void setAddress(const int p)     { _address->setId(p); };
	inline virtual void setDescription(const QString&) { };
	inline virtual void setFirst(const QString& p)	{ _first->setText(p); };
	inline virtual void setHonorific(const QString& p) { _honorific->setEditText(p); };
	inline virtual void setLabel(const QString& p)  { _label->setText(p); _label->setHidden(_label->text().isEmpty()); };
	inline virtual void setLast(const QString& p)	{ _last->setText(p); };
	inline virtual void setNotes(const QString& p)  { _notes = p; };
	inline virtual void setPhone(const QString& p)	{ _phone->setText(p); };
	inline virtual void setNumber(const QString&)	{ };
	inline virtual void setNumber(const int)	{ };
	inline virtual void setTitle(const QString& p)	{ _title->setText(p); };

	virtual void	clear();
	virtual void	sCheck();
	virtual void	sEllipses();
	virtual void	sInfo();
	virtual void	sList();
	virtual void	sSearch();
	virtual int	save(AddressCluster::SaveFlags = AddressCluster::CHECK);
	virtual void	setAccount(const int);
	virtual void	setAccountVisible(const bool);
	virtual void	setActiveVisible(const bool);
	virtual void	setAddressVisible(const bool);
	virtual void	setEmailVisible(const bool);
	virtual void	setId(const int);
	virtual void	setInitialsVisible(const bool);
	virtual void	setMinimalLayout(const bool);
	virtual void	setName(const QString& p);
	virtual void	setPhonesVisible(const bool);
	virtual void	setSearchAcct(const int);
	virtual void	setWebaddrVisible(const bool);

    signals:
	void changed();
	void newId(int);

    protected:
	QHBoxLayout*	_nameBox;
	QHBoxLayout*	_titleBox;
	QHBoxLayout*	_buttonBox;
	XComboBox*	_honorific;
	QLabel*		_firstLit;
	QLineEdit*	_first;
	QLabel*		_lastLit;
	QLineEdit*	_last;
	QLabel*		_initialsLit;
	QLineEdit*	_initials;
	QLabel*		_crmAcctLit;
	CRMAcctCluster*	_crmAcct;
	QLabel*		_titleLit;
	QLineEdit*	_title;
	QLabel*		_phoneLit;
	QLineEdit*	_phone;
	QLabel*		_phone2Lit;
	QLineEdit*	_phone2;
	QLabel*		_faxLit;
	QLineEdit*	_fax;
	QLabel*		_emailLit;
	QLineEdit*	_email;
	QLabel*		_webaddrLit;
	QLineEdit*	_webaddr;
	QCheckBox*	_active;
	AddressCluster*	_address;

	QString	_extraClause;
	QString	_query;	
	int	_searchAcctId;
	QString _titleSingular;
	QString _titlePlural;

        bool    _ignoreSignals;

    private:
	virtual void	init();
	virtual void	layout();
	virtual void	silentSetId(const int);

	int		_id;
	bool		_layoutDone;
	int		_limits;
	bool		_minimalLayout;
	QString		_notes;
	bool		_valid;

	// cached values
	QString		c_honorific;
	QString		c_first;
	QString		c_last;
	QString		c_initials;
	int		c_crmAcct;
	QString		c_title;
	QString		c_phone;
	QString		c_phone2;
	QString		c_fax;
	QString		c_email;
	QString		c_webaddr;
	bool		c_active;
	int		c_address;
        QString         c_notes;
};

#endif

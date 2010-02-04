/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2010 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#ifndef _contactCluster_h

#define _contactCluster_h

#include "virtualCluster.h"
#include "addresscluster.h"
#include "xurllabel.h"


class XTUPLEWIDGETS_EXPORT ContactClusterLineEdit : public VirtualClusterLineEdit
{
    Q_OBJECT

    friend class ContactCluster;
    public:
      ContactClusterLineEdit(QWidget*, const char* = 0);
};

class XTUPLEWIDGETS_EXPORT ContactCluster : public VirtualCluster
{
    Q_OBJECT
    Q_PROPERTY(bool minimalLayout READ minimalLayout WRITE setMinimalLayout);

    public:
      ContactCluster(QWidget*, const char* = 0);
      Q_INVOKABLE int searchAcctId() { return _searchAcctId; }

      bool minimalLayout() { return _minLayout; }

      void setHonorific(const QString honorifc);
      void setFirst(const QString first);
      void setMiddle(const QString middle);
      void setLast(const QString last);
      void setSuffix(const QString suffix);
      void setPhone(const QString phone);
      void setTitle(const QString title);
      void setFax(const QString fax);
      void setEmailAddress(const QString email);

      QString honorific() const { return _fname->at(0); }
      QString first() const { return _fname->at(1); }
      QString middle() const { return _fname->at(2); }
      QString last() const { return _fname->at(3); }
      QString suffix() const { return _fname->at(4); }
      QString title() const { return _description->text(); }
      QString phone() const { return _phone->text(); }
      QString fax() const { return _fax->text(); }
      QString emailAddress() const { return ""; }

      // TODO: Remove these
      void setAccount(int) { return; }
      int crmAcctId() { return -1; }
      void setInitialsVisible(bool) { return; }
      void setActiveVisible(bool) { return; }
      void setAccountVisible(bool) { return; }
      void setOwnerEnabled(bool) { return; }
      int save(AddressCluster::SaveFlags = AddressCluster::CHECK) { return 2; }
      int check(AddressCluster::SaveFlags = AddressCluster::CHECK) { return 2; }
      AddressCluster* addressWidget() const { return _address; }
      void setOwnerUsername(QString) { return; }
      bool sChanged() { return false; }
      QString change() { return QString(); }

    public slots:
      void openUrl(QString url);
      void setMinimalLayout(bool);
      void setSearchAcct(int crmAcctId);

      void sList();
      void sSearch();

    private slots:
      void populate();

    protected:
      void addNumberWidget(ContactClusterLineEdit* pNumberWidget);
      void setName(int segment, const QString name);

      int _searchAcctId;
      AddressCluster* _address;
      QSpacerItem* _cntctSpacer;
      QVBoxLayout* _addrLayout;
      QSpacerItem* _addrSpacer;
      QLabel* _titleLit;
      QLabel* _phoneLit;
      QLabel* _phone;
      QLabel* _phone2Lit;
      QLabel* _phone2;
      QLabel* _faxLit;
      QLabel* _fax;
      QLabel* _emailLit;
      QLabel* _webaddrLit;
      QLabel* _addr;
      QStringList* _fname;
      XURLLabel* _email;
      XURLLabel* _webaddr;

    private:
      bool _minLayout;
};

#endif

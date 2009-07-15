/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2009 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include "addressclustersetup.h"
#include "alarmssetup.h"
#include "clineeditsetup.h"
#include "commentssetup.h"
#include "contactclustersetup.h"
#include "crmacctlineeditsetup.h"
#include "currdisplaysetup.h"
#include "documentssetup.h"
#include "glclustersetup.h"
#include "itemlineeditsetup.h"
#include "metasqlhighlighterproto.h"
#include "orderlineeditsetup.h"
#include "parametergroupsetup.h"
#include "polineeditsetup.h"
#include "projectlineeditsetup.h"
#include "qactionproto.h"
#include "qdomattrproto.h"
#include "qdomcdatasectionproto.h"
#include "qdomcharacterdataproto.h"
#include "qdomcommentproto.h"
#include "qdomdocumentfragmentproto.h"
#include "qdomdocumentproto.h"
#include "qdomdocumenttypeproto.h"
#include "qdomelementproto.h"
#include "qdomentityproto.h"
#include "qdomentityreferenceproto.h"
#include "qdomimplementationproto.h"
#include "qdomnamednodemapproto.h"
#include "qdomnodelistproto.h"
#include "qdomnodeproto.h"
#include "qdomnotationproto.h"
#include "qdomprocessinginstructionproto.h"
#include "qdomtextproto.h"
#include "qfontproto.h"
#include "qmenuproto.h"
#include "qmessageboxsetup.h"
#include "qnetworkreplyproto.h"
#include "qnetworkrequestproto.h"
#include "qsqldatabaseproto.h"
#include "qsqlrecordproto.h"
#include "qstackedwidgetproto.h"
#include "qtabwidgetproto.h"
#include "qtextdocumentproto.h"
#include "qtexteditproto.h"
#include "qtreewidgetitemproto.h"
#include "qtsetup.h"
#include "qurlproto.h"
#include "qwidgetproto.h"
#include "ralineeditsetup.h"
#include "revisionlineeditsetup.h"
#include "screensetup.h"
#include "setupscriptapi.h"
#include "shipmentclusterlineeditsetup.h"
#include "solineeditsetup.h"
#include "tolineeditsetup.h"
#include "usernamelineeditsetup.h"
#include "vendorgroupsetup.h"
#include "wcomboboxsetup.h"
#include "wolineeditsetup.h"
#include "womatlclustersetup.h"
#include "xcomboboxsetup.h"
#include "xdatawidgetmapperproto.h"
#include "xdateeditsetup.h"
#include "xnetworkaccessmanager.h"
#include "xsqltablemodelproto.h"
#include "xtreewidgetitemproto.h"

void setupScriptApi(QScriptEngine *engine)
{
  setupAddressCluster(engine);
  setupAlarms(engine);
  setupCLineEdit(engine);
  setupCRMAcctLineEdit(engine);
  setupComments(engine);
  setupContactCluster(engine);
  setupCurrDisplay(engine);
  setupDocuments(engine);
  setupGLCluster(engine);
  setupItemLineEdit(engine);
  setupMetaSQLHighlighterProto(engine);
  setupOrderLineEdit(engine);
  setupParameterGroup(engine);
  setupPoLineEdit(engine);
  setupProjectLineEdit(engine);
  setupQActionProto(engine);
  setupQDomAttrProto(engine);
  setupQDomCDATASectionProto(engine);
  setupQDomCharacterDataProto(engine);
  setupQDomCommentProto(engine);
  setupQDomDocumentFragmentProto(engine);
  setupQDomDocumentProto(engine);
  setupQDomDocumentTypeProto(engine);
  setupQDomElementProto(engine);
  setupQDomEntityProto(engine);
  setupQDomEntityReferenceProto(engine);
  setupQDomImplementationProto(engine);
  setupQDomNamedNodeMapProto(engine);
  setupQDomNodeListProto(engine);
  setupQDomNodeProto(engine);
  setupQDomNotationProto(engine);
  setupQDomProcessingInstructionProto(engine);
  setupQDomTextProto(engine);
  setupQFontProto(engine);
  setupQMenuProto(engine);
  setupQMessageBox(engine);
  setupQNetworkAccessManagerProto(engine);
  setupQNetworkReplyProto(engine);
  setupQNetworkRequestProto(engine);
  setupQSqlDatabaseProto(engine);
  setupQSqlRecordProto(engine);
  setupQStackedWidgetProto(engine);
  setupQTabWidgetProto(engine);
  setupQTextDocumentProto(engine);
  setupQTextEditProto(engine);
  setupQTreeWidgetItemProto(engine);
  setupQt(engine);
  setupQUrlProto(engine);
  setupQWidgetProto(engine);
  setupRaLineEdit(engine);
  setupRevisionLineEdit(engine);
  setupScreen(engine);
  setupShipmentClusterLineEdit(engine);
  setupSoLineEdit(engine);
  setupToLineEdit(engine);
  setupUsernameLineEdit(engine);
  setupVendorGroup(engine);
  setupWComboBox(engine);
  setupWoLineEdit(engine);
  setupWomatlCluster(engine);
  setupXComboBox(engine);
  setupXDataWidgetMapperProto(engine);
  setupXDateEdit(engine);
  setupXSqlTableModelProto(engine);
  setupXTreeWidgetItemProto(engine);

}

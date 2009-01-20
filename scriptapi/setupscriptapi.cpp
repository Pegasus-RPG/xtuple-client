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
#include "orderlineeditsetup.h"
#include "parametergroupsetup.h"
#include "polineeditsetup.h"
#include "projectlineeditsetup.h"
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
#include "qnetworkreplyproto.h"
#include "qnetworkrequestproto.h"
#include "qsqldatabaseproto.h"
#include "qurlproto.h"
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
  setupOrderLineEdit(engine);
  setupParameterGroup(engine);
  setupPoLineEdit(engine);
  setupProjectLineEdit(engine);
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
  setupQNetworkAccessManagerProto(engine);
  setupQNetworkReplyProto(engine);
  setupQNetworkRequestProto(engine);
  setupQSqlDatabaseProto(engine);
  setupQUrlProto(engine);
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

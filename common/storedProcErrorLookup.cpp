/*
 * This file is part of the xTuple ERP: PostBooks Edition, a free and
 * open source Enterprise Resource Planning software suite,
 * Copyright (c) 1999-2014 by OpenMFG LLC, d/b/a xTuple.
 * It is licensed to you under the Common Public Attribution License
 * version 1.0, the full text of which (including xTuple-specific Exhibits)
 * is available at www.xtuple.com/CPAL.  By using this software, you agree
 * to be bound by its terms.
 */

#include <QMessageBox>
#include <QMultiHash>
#include <QObject>
#include <QPair>
#include <QString>
#include <QCoreApplication>

/*	try to address bug 4218
  This code assumes that stored procedures
  return zero or positive integers on success
  return negative integers on failure
 */

QMultiHash< QString, QPair<int, QString> >	ErrorLookupHash;

/*
  developers add error messages to an array for ease of adding new ones.
  initErrorLookupHash then inserts these into a QMultiHash(key, value).
  The key is the stored procedure name.
  The value is a QPair* of (return value, error message).
*/

const struct {
  QString	procName;	// name of the stored procedure
  int		retVal;		// return value from the stored procedure
  const char*	msg;		// msg to display, but see msgPtr and proxyName
  int		msgPtr;		// if <> 0 then look up (procName, msgPtr)
  QString	proxyName;	// look up (proxyName, retVal)
} errors[] = {

  { "attachQuoteToOpportunity", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Quote cannot be attached because "
                            "the Quote cannot be found."),                     0, "" },
  { "attachQuoteToOpportunity", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Quote cannot be attached because "
                            "the Opportunity cannot be found."),               0, "" },
  { "attachQuoteToOpportunity", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Quote cannot be attached because "
                            "it is already associated with an Opportunity.  You must "
                            "detach this Quote before you may attach it."),    0, "" },

  { "attachSalesOrderToOpportunity", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order cannot be attached because "
                            "the Sales Order cannot be found."),                     0, "" },
  { "attachSalesOrderToOpportunity", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order cannot be attached because "
                            "the Opportunity cannot be found."),               0, "" },
  { "attachSalesOrderToOpportunity", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order cannot be attached because "
                            "it is already associated with an Opportunity.  You must "
                            "detach this Sales Order before you may attach it."),    0, "" },

  { "calculateFreightDistribution", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "No valid Items have been distributed to the Voucher."
                                    " Items must be Purchased goods and be costed items.  Please check and distribute items before"
                                    " attempting to distribute freight."),     0, "" },
  { "calculateFreightDistribution", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Voucher you are attempting to distribute freight "
                                    "to has incorrect item totals for the desired distribution method. Please check that Voucher"),     0, "" },

  { "calcIssueToShippingLineBalance", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
                            "The order type is not valid for issue to shipping."),     0, "" },


  { "changeCMHeadTaxAuth", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Return was not found."),	0, "" },
  { "changeCMHeadTaxAuth", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority was not found."),	0, "" },

  { "changeInvoiceTaxZone", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Invoice was not found."),	0, "" },
  { "changeInvoiceTaxZone", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Freight Tax Type was not found."),0, "" },

  { "changeCobTaxZone", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Bill was not found."),	0, "" },
  { "changeCobTaxZone", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Freight Tax Type was not found."),0, "" },

  { "changeQuoteTaxZone", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Quote was not found."),		0, "" },
 // { "changeQuoteTaxAuth", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority was not found."),	0, "" },

  { "changeSOTaxZone", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order was not found."),	0, "" },
 // { "changeSOTaxAuth", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority was not found."),	0, "" },
  { "changeTOTax", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order was not found."),	0, "" },
  { "changeTOTaxAuth", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority was not found."),	0, "" },
  { "changeWoQty", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Work Order is closed and cannot be changed."),	0, "" },
  { "charassUniqueTrigger", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This characteristic has been defined as unique.  "
                            "You cannot use this characteristic more than once in this context."),	0, "" },
  { "closeAccountingPeriod", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because it is already closed."),
									0, "" },
  { "closeAccountingPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because there is a gap between the "
				    "end of the previous Period and the start "
				    "of this Period. You must edit either the "
				    "previous Perod or this Period to "
				    "eliminate the gap."),		0, "" },
  { "closeAccountingPeriod", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because the previous Period is not "
				    "closed. You must close the previous Period"
				    " before you may close this Period."),
									0, "" },
  { "closeAccountingPeriod", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because there is a gap between the "
				    "end of this Period and the start of the "
				    "next Period. You must edit either this "
				    "Period or the next Period to eliminate "
				    "the gap."),			0, "" },
  { "closeAccountingPeriod", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because it ends in the future."),
									0, "" },
  { "closeAccountingPeriod", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period cannot be "
				    "closed because it is the last period in "
				    "the Fiscal Year and the next Fiscal Year "
				    "has not been defined yet. Create the "
				    "next Fiscal Year before closing this "
				    "Accounting Period."),		0, "" },

  { "closeAccountingYearPeriod", -1, "", -1, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -2, "", -2, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -3, "", -3, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -4, "", -4, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -5, "", -5, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -6, "", -6, "closeAccountingPeriod" },
  { "closeAccountingYearPeriod", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be closed "
                                        "because you have not specified a Year End Equity Account "
                                        "in the accounting configuration."), 0, "" },
  { "closeAccountingYearPeriod", -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be "
                                        "closed because there does not seem to "
                                        "be an Accounting Period defined for "
                                        "the beginning of the next Fiscal "
                                        "Year."),                       0, "" },
  { "closeAccountingYearPeriod", -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be closed "
                                        "because there is no Trial Balance record for "
                                        "the account in the required Period. Or you have "
                                        "not specified a Year End Equity Account in the "
                                        "accounting configuration."), 0, "" },
  { "closeAccountingYearPeriod", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be closed "
                                      "because there are periods within the year that are still open."), 0, "" },
  { "closeAccountingYearPeriod", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be closed "
                                    "because there are prior years that are still open."), 0, "" },
  { "closeToItem",	 -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The item cannot be Closed at this time "
				"as there is inventory at shipping."),	0, "" },

  { "convertCustomerToProspect",	-1, "", -1, "deleteCustomer" },
  { "convertCustomerToProspect",	-2, "", -2, "deleteCustomer" },
  { "convertCustomerToProspect",	-3, "", -3, "deleteCustomer" },
  { "convertCustomerToProspect",	-4, "", -4, "deleteCustomer" },
  { "convertCustomerToProspect",	-5, "", -5, "deleteCustomer" },
  { "convertCustomerToProspect",
		       -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not convert Customer to Prospect to "
			       "because there is already a Prospect with "
			       "this internal ID."),		   0, "" },

  { "convertProspectToCustomer",	-1, "", -1, "deleteProspect" },
  { "convertProspectToCustomer",
		       -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not convert Prospect to Customer "
			       "because there is already a Customer with "
			       "this internal ID."),		   0, "" },

  { "convertQuote", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Quote #%1 has one or more line items without a "
			   "warehouse specified. These line items must be "
			   "fixed before you may convert this quote." ),
								   0, "" },
  { "convertQuote", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot find the Customer data for Quote #%1."),
								   0, "" },
  { "convertQuote", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Quote #%1 is associated with a Prospect, "
			       "not a Customer. Convert the Prospect to a "
			       "Customer first."),		   0, "" },
  { "convertQuote", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Quote #%1 is for a Customer that has been "
			   "placed on a Credit Hold and you do not have "
			   "privilege to create Sales Orders for Customers on "
			   "Credit Hold.  The selected Customer must be taken "
			   "off of Credit Hold before you may create convert "
			   "this Quote."),			   0, "" },

  { "convertQuote", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Quote #%1 is for a Customer that has been "
			   "placed on a Credit Warning and you do not have "
			   "privilege to create Sales Orders for Customers on "
			   "Credit Warning.  The selected Customer must be "
			   "taken off of Credit Warning before you may create "
			   "convert this Quote."),		   0, "" },
  { "convertQuote", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Quote #%1 has expired and can not be converted."),
								   0, "" },
  { "convertQuote", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Customer PO required."),
    0, "" },
  { "convertQuote", -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Duplicate Customer PO."),
    0, "" },

  { "copyItemSite",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not copy the Item Site because it "
			    "does not appear to exist."),		0, "" },
  { "copyItemSite",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not copy the Item Site because the "
			    "warehouse for the new Item Site record "
			    "does not appear to exist."),		0, "" },
  { "copyItemSite",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You do not have sufficient privilege to "
			    "create an Item Site."),			0, "" },

  { "copyBOM", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not find the Source BOM to copy."), 0, "" },
  { "copyBOM", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected source Item does not "
                                     "have any Bill of Material Component Items associated with it."), 0, "" },
  { "copyBOM", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected target Item already has a "
                                     "Bill of Materials associated with it.\n"
                                     "You must first delete the Bill of Materials for the selected target item before "
                                     "attempting to copy an existing Bill of Materials."), 0, "" },
  { "copyBOM", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Item you are trying to copy "
                                     "this Bill of Material to is a "
                                     "component item which would cause a recursive Bill of Material."),	0, "" },

  { "copyPO", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not find the P/O to copy."),		0, "" },
  { "copyPO", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Vendor of the original P/O does not match "
		     "the Vendor for the copy. Changing the Vendor "
		     "is not yet supported when copying a P/O."),	0, "" },
  { "copyPO", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The system does not allow purchases of Items for this "
		     "Vendor without Item Sources and at least one line item "
		     "item in the original P/O does not have an active "
		     "Item Source."),					0, "" },
  { "copyPO", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "At least one line item in the original P/O does not have "
		     "an active Item Source Price for this Vendor."),	0, "" },

  { "copyPrj",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Copying an existing project failed, possibly because "
                       "the source project does not exist."),           0, "" },
  { "copyPrj", -10, "",                                      -10, "saveAlarm" },

  { "correctOperationPosting",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You may not correct a quantity greater than the amount "
                                       "originally posted."),	0, "" },
  
  { "correctReceipt",  -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Can not correct receipt for order type other than PO, RA or TO"),  0, "" },
  { "correctReceipt",  -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The receipt has been split and may not be corrected. "
				      "Correct Receipt."),	0, "" },
  { "correctReceipt",  -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "correctReceipt",  -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "correctReceipt",  -15, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },
  { "correctReceipt",  -16, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not find a recv record to correct."),  0, "" },
  { "correctReceipt",  -17, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Order item information not found for recv_id."),  0, "" },
  { "correctReceipt",  -18, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Could not insert G/L transaction: no cost category found for itemsite_id."),  0, "" },
  { "correctReceipt",  -19, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Failed to create a GL transaction for the purchase price variance."),  0, "" },

  { "correctProduction",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Can not correct production, WO must be status In-Process."),  0, "" },
  { "correctProduction",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Correct production does not support Job costed items."),  0, "" },
  { "correctProduction",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "ItemlocSeries is required when pPreDistributed."),  0, "" },
  { "correctProduction",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Missing cost category."),  0, "" },
  { "correctProduction",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Posting Distribution Detail Returned 0 Results."),  0, "" },

  { "createAccountingPeriod",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Start Date falls within another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The End Date falls within another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Start and End Dates enclose another "
				      "Accounting Period."),	0, "" },
  { "createAccountingPeriod",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Period dates are outside the "
				      "selected Fiscal Year."),	0, "" },
  { "createAccountingPeriod",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Start Date must be prior "
				      "to the End Date."),	0, "" },

  { "createAccountingYearPeriod",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Year is closed." ),	0, "" },
  { "createAccountingYearPeriod",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Year dates may not overlap another year." ),	0, "" },
  { "createAccountingYearPeriod",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Year dates may not overlap another year." ),	0, "" },
  { "createAccountingYearPeriod",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Periods exist for this "
                                                       "year outside the proposed dates." ),	0, "" },
  { "createAccountingYearPeriod",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Start Date must be prior to the End Date" ),	0, "" },

  { "createAPCreditMemoApplication",
			-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You may not apply more than the balance due "
			       "to this document."),		0, "" },
  { "createAPCreditMemoApplication",
			-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You may not apply more than the amount "
			       "available to apply for this Credit Memo."),
								0, "" },

  { "createARCreditMemo", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Either the Prepaid Account or the A/R Account"
				 " for this Customer could not be found."),
								0, "" },

  { "createARDebitMemo", -1, "",		-1, "createARCreditMemo" },

  { "createBOMItem",	 -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You may not create a BOM Item that defines a "
				"Parent that is composed of itself."),	0, "" },
  { "createBOMItem",	 -2, QT_TRANSLATE_NOOP("storedProcErrorLookup",  "The Component that you have selected for this"
				 "BOM Item is a manufactured or phantom Item "
				 "that uses the Parent Item as a Component "
				 "Item in its own BOM. You may not create a "
				 "recursive BOM."),			0, "" },

  { "createCrmAcct",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Account Number is already in use by "
			       "an existing Account. Please choose a "
			       "different number and save again."),	0, "" },
  { "createCrmAcct",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Account Number is already in use by "
			       "an existing Customer. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Account Number is already in use by "
			       "an existing Prospect. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Account Number is already in use by "
			       "an existing Vendor. Please choose a different"
			       " number and save again."),		0, "" },
  { "createCrmAcct",	-7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Account Number is already in use by "
			       "an existing Tax Authority. Please choose a "
			       "different number and save again."),	0, "" },

  { "createProspect",	 -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot create a Prospect because there is no "
				"Account to tie it to."),
								0, "" },
  { "createProspect",	 -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot create a Prospect for this Account "
				"because it is already a Customer."),
								0, "" },
  { "createProspect",    -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot create a Prospect for this Account "
				"because it is already a Prospect."),
								0, "" },
  { "createPurchaseToSale", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "SO Header Information related to this SO Item not found!"),	0, "" },
  { "createPurchaseToSale", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Item Source Information not found!"),	0, "" },

  { "createRecurringItems", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot create recurring items with an "
                                    "unrecognized object type."),       0, "" },

  { "CreateRevision", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Revision control not enabled."),	0, "" },
  { "createTodoItem",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be created as "
			       "there is no assigned User."),	0, "" },
  { "createTodoItem",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be created as "
			       "the Task Name is blank."),	0, "" },
  { "createTodoItem",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be created as "
			       "there is no Due Date."),	0, "" },

  { "createWo",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order can not be created because Site "
			       "not allowed to Manufacture this Item."),	0, "" }, 
  { "createWo",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order can not be exploded because items on "
			       "the BOM exist without itemsites."),	0, "" }, 

  { "CRMAccount",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The CRM Account is associated with a system user "
                               " so the number cannot be changed."),		 0, "" },
  { "CRMAccount",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "User does not exist so this CRM Account Number is invalid."
                                                                  ),		 0, "" },
  { "CRMAccount",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot change the Username"), 0, "" },

  { "deleteAccount",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Cost Categories."
                             "  You must reassign these Cost Category "
                             "assignments before you may delete the selected "
                             "Ledger Account."),                           0, "" },
  { "deleteAccount",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Sales Account "
                             "Assignment. You must reassign these Sales "
                             "Account Assignments before you may delete "
                             "the selected Ledger Account."),              0, "" },
  { "deleteAccount",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Customer A/R "
                             "Account assignments. You must reassign these "
                             "Customer A/R Account assignments before you may "
                             "delete the selected Ledger Account."),       0, "" },
  { "deleteAccount",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used as the default Account one or "
                             "more Sites. You must reassign the default "
                             "Account for these Sites before you may delete "
                             "the selected Ledger Account."),              0, "" },
  { "deleteAccount",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Bank Accounts. "
                             "You must reassign these Bank Accounts before you "
                             "may delete the selected Ledger Account."),   0, "" },
  { "deleteAccount",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Expense "
                             "Categories. You must reassign these Expense "
                             "Categories before you may delete the selected "
                             "Ledger Account."),                           0, "" },
  { "deleteAccount",  -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Tax Codes. "
                             "You must reassign these Tax Codes before you "
                             "may delete the selected Ledger Account."),   0, "" },
  { "deleteAccount",  -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Standard "
                             "Journals. You must reassign these Standard "
                             "Journal Items before you may delete the selected "
                             "Ledger Account."),                           0, "" },
  { "deleteAccount",  -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Customer A/P "
                             "Account assignments. You must reassign these "
                             "Customer A/P Account assignments before you may "
                             "delete the selected Ledger Account."),       0, "" },
  { "deleteAccount", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more Currency "
                             "definition. You must reassign these Currency "
                             "definitions before you may delete the selected "
                             "Ledger Account."),                           0, "" },
  { "deleteAccount", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as it "
                             "is currently used in one or more A/R Open Items. "
                             "You must reassign these Currency definitions "
                             "before you may delete the selected Ledger Account."),
                                                                        0, "" },
  { "deleteAccount", -99, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Ledger Account cannot be deleted as "
                             "there have been Ledger Transactions posted "
                             "against it."),                            0, "" },

  { "deleteAccountingPeriod", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period has G/L "
				     "Transactions posted against it and, thus, "
				     "cannot be deleted."), 0, "" },

  { "deleteAccountingPeriod", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Accounting Period is not "
				     "the last accounting period and "
				     "cannot be deleted."), 0, "" },

  { "deleteAccountingYearPeriod", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be "
					 "deleted because it is closed."),
									0, "" },
  { "deleteAccountingYearPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Fiscal Year cannot be "
					 "deleted because there are Accounting "
					 "Periods defined for it."),	0, "" },

  { "deleteAddress",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Address cannot be deleted as "
			       "it is used by an active Contact."),
								 0, "" },
  { "deleteAddress",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Address cannot be deleted as "
			       "it is used by an active Vendor."),
								 0, "" },
  { "deleteAddress",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Address cannot be deleted as "
			       "it is used by an active Ship-To Address."),
								 0, "" },
  { "deleteAddress",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Address cannot be deleted as "
			       "it is used by an active Vendor Address."),
								 0, "" },
  { "deleteAddress",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Address cannot be deleted as "
			       "it is used by an active Site."),
			       					 0, "" },

  { "deleteBankAdjustmentType",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Bank Adjustment Type "
                                        "cannot be deleted because it is "
                                        "currently used by a Bank Adjustment."),
                                                                 0, "" },

  { "deleteCashrcpt",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be deleted "
                              "because it is a Customer Deposit made with "
                              "a Credit Card and the card has already been "
                              "charged."),                              0, "" },
  { "deleteCharacteristic",
			-99, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Characteristic cannot be deleted "
                               "because it has been assigned to other documents. "
			       "You must remove these assignments before "
                               "you may delete the selected Characteristic."),
			       					 0, "" },

  { "deleteCheck", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot delete this Payment because either it has not "
                          "been voided, it has already been posted or replaced,"
                          " or it has been transmitted electronically."),
			           					0, "" },

  { "deleteClassCode", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",  "The selected Class Code cannot be deleted "
                              "because there are Items that are assigned to "
                              "it. You must reassign these Items before you "
                              "may delete the selected Class Code." ),  0, ""},

  { "deleteCompany", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Company cannot be deleted as it "
                            "is in use by existing Account. You must reclass "
                            "these Accounts before you may delete the selected "
                            "Company." ),                        0, "" },

  { "deleteContact",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the primary or secondary Contact "
			       " for a Account."),		 0, "" },
  { "deleteContact",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the Correspondence or Billing "
			       "Contact for a Customer."),	 0, "" },
  { "deleteContact",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the primary or secondary "
			       "Contact for a Vendor."),	 0, "" },
  { "deleteContact",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Ship-To Address."),
								 0, "" },
  { "deleteContact",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Vendor Address."),
			       					 0, "" },
  { "deleteContact",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Contact cannot be deleted as "
			       "s/he is the Contact for a Site."),
								 0, "" },

  { "deleteCRMAccount",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Customer."),		 0, "" },
  { "deleteCRMAccount",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Vendor."),		 0, "" },
  { "deleteCRMAccount",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Prospect."),		 0, "" },
  { "deleteCRMAccount",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it has Contacts. You may Detach the Contacts "
			       "from this Account and try deleting it "
			       "again or set its status to inactive"),
								 0, "" },
  { "deleteCRMAccount",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Tax Authority."),	 0, "" },
  { "deleteCRMAccount",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Sales Rep."),	        0, "" },
  { "deleteCRMAccount",	-7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a Employee."),	        0, "" },
  { "deleteCRMAccount",	-8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Account cannot be deleted as "
			       "it is a User."),	        0, "" },

  { "deleteCustomer",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer cannot be deleted as there "
			       "are still Ship-Tos assigned to it. You must "
			       "delete all of the selected Customer's Ship-Tos "
			       "before you may delete it."),	 0, "" },
  { "deleteCustomer",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Customer. You may Edit the "
			       "selected Customer and set its status to "
			       "inactive."),			 0, "" }, //Purchase Orders
  { "deleteCustomer",	-3, "Returns",		-2, "" },
  { "deleteCustomer",	-4, "custhist",		-2, "" },
  { "deleteCustomer",	-5, "A/R Open",		-2, "" },
  { "deleteCustomer",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer cannot be deleted as "
			       "Payments have been written to it."),	0, "" },
  { "deleteCustomer",   -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer cannot be deleted as there "
                               "are still Invoices assigned to it. You must "
                               "delete all of the selected Customer's Invoices "
                               "before you may delete it"),      0, "" },
  { "deleteCustomer",   -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer cannot be deleted as there "
                               "are still Quotes assigned to it. You must "
                               "delete all of the selected Customer's Quotes "
                               "before you may delete it"),      0, "" },

  { "deleteCustomerType", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Customer Type cannot be deleted "
                                 "as there are one or more Customers assigned "
                                 "to it. You must reassign these Customers "
                                 "before you may delete the selected Customer "
                                 "Type."),                              0, "" },

{ "deleteEmpgrp",       -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Employee Group cannot be deleted "
                               "as there are one or more Employees assigned "
                               "to it. You must reassign these Employees "
                               "before you may delete the selected Employee "
                               "Group."),                              0, "" },

  { "deleteForm", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Check Format cannot be deleted as it is "
                         "used by one or more Bank Accounts. You must reassign "
                         "these Bank Accounts before you may delete the "
                         "selected Check Form." ),                      0, "" },

  { "deleteFreightClass", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",  "The selected Freight Class cannot be deleted "
                                  "because there are Items that are assigned to "
                                  "it. You must reassign these Items before you "
                                  "may delete the selected Freight Class." ),  0, ""},

  { "deleteIncident",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Incident cannot be deleted as there are "
			       "To-Do List Items associated with it."),
								 	0, "" },
  { "deleteIncident",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Incident cannot be deleted as there are "
			       "Comments associated with it."), 	0, "" },

  { "deleteItem",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as it is used "
			       "in one or more bills of materials."), 	0, "" },
  { "deleteItem",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as there are "
			       "Item Site records associated with it."), 	0, "" },
  { "deleteItem",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as there are "
			       "Substitute records associated with it."), 	0, "" },
  { "deleteItem",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as there are "
			       "Breeder BOM records associated with it."), 	0, "" },
  { "deleteItem",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as there are "
                               "assignment records associated with it."), 	0, "" },
  { "deleteItem",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as there are "
			       "Revision Control records associated with it."), 	0, "" },
  { "deleteItem",	-7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item cannot be deleted as it has "
                               "been invoiced."), 	0, "" },
  { "deleteItemSite",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is Inventory History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is Work Order History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is Sales History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is Purchasing History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is Planning History posted against it. "
			      "You may edit the Item Site and deactivate it."),	0, "" },
  { "deleteItemSite",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there are Production Plans associated with it."),	0, "" },
  { "deleteItemSite",  -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "it is used as a Supplied from Site."),	0, "" },
  { "deleteItemSite",  -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Item Site cannot be deleted as "
			      "there is a non-zero Inventory Quantity posted "
			      "against it."),	0, "" },

  { "deleteItemUOMConv", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This UOM Conversion cannot be deleted as "
			       "there are records for this Item which use this "
                               "UOM."), 	                        0, "" },
  { "deleteLocation", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There are one or more Item Sites that use the "
                              "selected Location as their default Location. "
                              "You must reassign the default Location for all Item Sites that use the "
                              "selected Location before you may delete it or deactivate it."),       0, "" },
  { "deleteLocation", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is Inventory contained in the selected Location. "
                              "You must move all Inventory out of the selected Location "
                              "and may then set its status to inactive." ),       0, "" },
  { "deleteLocation", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There are one or more undistributed Location records "
                              "that have been posted against the selected Locations. "
                              "This probably indicates a system error."
                              "Please contact your Systems Adminstrator to have this resolved."),       0, "" },
  { "deleteLocation", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Location cannot be deleted as there has "
                              "been Inventory Transaction History posted against it."),       0, "" },

  { "deleteOpenRecurringItems",  -1, "",                 -1, "deleteIncident" },
  { "deleteOpenRecurringItems",  -2, "",                 -2, "deleteIncident" },
  { "deleteOpenRecurringItems", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot delete open recurring items "
                                        "with an invalid type."),       0, "" },
  { "deleteOpenRecurringItems", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot delete open recurring items "
                                        "without a valid parent item."),0, "" },

  { "deleteOpportunity", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Opportunity cannot be deleted because "
                            "there are ToDo Items assigned to it.  You must "
                            "delete or reassign these ToDo Items "
                            "before you may delete it."),                      0, "" },
  { "deleteOpportunity", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Opportunity cannot be deleted because "
                            "there are Quotes assigned to it.  You must "
                            "delete or reassign these Quotes "
                            "before you may delete it."),                      0, "" },
  { "deleteOpportunity", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Opportunity cannot be deleted because "
                            "there are Sales Orders assigned to it.  You must "
                            "delete or reassign these Sales Orders "
                            "before you may delete it."),                      0, "" },

  { "deletePackage", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Package cannot be deleted because "
                            "there are other packages that depend on it to "
                            "function properly."),                      0, "" },

  { "deleteProfitCenter", -1,
                        QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Profit Center cannot be deleted as it "
                           "is in use by existing Account. You must reclass "
                           "these Accounts before you may delete the selected "
                           "Profit Center." ),                          0, "" },

  { "deleteProject",      -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Quote's refer to this project."),                   0, "" },
  { "deleteProject",      -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Sales Orders refer to this project."),              0, "" },
  { "deleteProject",      -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Work Orders refer to this project."),               0, "" },
  { "deleteProject",      -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Purchase Requests refer to this project."),         0, "" },
  { "deleteProject",      -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Purchase order Items refer to this project."),      0, "" },
  { "deleteProject",      -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One or more "
                           "Invoices refer to this project."),                  0, "" },

  { "deleteProspect",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Prospect cannot be deleted as "
			       "there are still Quotes for it. You must delete "
			       "all of this Prospect's Quotes before you may "
			       "delete the Prospect."),		 0, "" },
  { "deleteSalesRep",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Rep. cannot be deleted as "
                              "he/she is still assigned to one or more "
                              "Customers.  You must reassign different Sales "
                              "Reps. to all Customers to which the selected "
                              "Sales Rep. is assigned before you may delete "
                              "the selected Sales Rep." ),      0, "" },
  { "deleteSalesRep",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Rep. cannot be deleted as "
                              "he/she is still assigned to one or more "
                              "Ship-tos.  You must reassign different Sales "
                              "Reps. to all Ship-tos to which the selected "
                              "Sales Rep. is assigned before you may delete "
                              "the selected Sales Rep." ),      0, "" },
  { "deleteSalesRep",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Rep. cannot be deleted as "
                              "there has been sales history recorded against "
                              "him/her.  You may edit and set the selected "
                              "Sales Rep's active status to inactive." ),
                                                                0, "" },

  { "deleteShipto",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Shipto cannot be deleted as there "
			       "is still Archived Sales History assigned to it. You must "
			       "delete all of the selected Customer's Ship-Tos "
			       "before you may delete it."),	 0, "" },
  { "deleteShipto",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Shipto cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive."),			 0, "" },
  { "deleteShipto",	-3, "The selected Shipto cannot be deleted "
			       "as there has been Returns recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive.",		-2, "" },
  { "deleteShipto",	-4, "The selected Shipto cannot be deleted "
			       "as there has been Sales History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive.",		-2, "" },
  { "deleteShipto",	-5, "The selected Shipto cannot be deleted "
			       "as there has been Quote History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive.",		-2, "" },
  { "deleteShipto",	-6, "The selected Shipto cannot be deleted "
			       "as there has been Invoice History recorded "
			       "for this Shipto. You may Edit the "
			       "selected Shipto and set its status to "
			       "inactive.",			 0, "" },

  { "deleteSO",	 -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted because a Credit "
			"Card has been charged for it."),		0, "" },
  { "deleteSO",	 -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted because there is "
			"Credit Card transaction history for it."),	0, "" },
  /* uncomment this when releaseSoNumber returns INTEGER instead of BOOLEAN
  { "deleteSO",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted because there was "
			"an internal error releasing the Sales Order Number."),
									0, "" },
  */
  { "deleteSO",  -101, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as some of its "
			"line items have already been shipped."),	0, "" },
  { "deleteSO",  -102, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as some of its "
			"line items have already been issued to shipping.  "),		0, "" },
  { "deleteSO",  -103, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as some of its "
			"line items are linked to a Return Authorization.  "
			"You must resolve this conflict before "
		        "you may delete this Sales Order."),		0, "" },
  { "deleteSO",  -104, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as some of its "
			"line items are linked to an In Process Work Order.  "
			"You must resolve this conflict before "
		        "you may delete this Sales Order."),		0, "" },
  { "deleteSO",  -105, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as some of its "
			"line items have transaction history.  "),		0, "" },
  { "deleteSO",  -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order cannot be deleted as one or more "
                        "of its Line items have associated Purchase Order Line Items which are either closed or "
                        "have receipts associated with them. You may want to consider cancelling this Sales "
                        "Order instead."), 0, "" },
  { "deleteSO",  -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Sales Order was deleted successfully. However, "
                        "the Released Purchase Orders associated with one or more line items of this Sales Order "
                        "could not be deleted. You must delete these Purchase Orders seperately if desired."), 0, "" },

  { "deleteSOItem",  -101, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it has already been shipped."),	0, "" },
  { "deleteSOItem",  -102, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it has already been issued to shipping."),	0, "" },
  { "deleteSOItem",  -103, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it is linked to a Return Authorization.  "
			"You must resolve this conflict before "
		        "you may delete this Sales Order Item."),		0, "" },
  { "deleteSOItem",  -104, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it is linked to an In Process Work Order.  "
			"You must resolve this conflict before "
		        "you may delete this Sales Order Item."),		0, "" },
  { "deleteSOItem",  -105, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it has generated Inventory History.  "
		        "You may want to consider cancelling this Sales Order Item."),		0, "" },
  { "deleteSOItem",  -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order Item cannot be deleted as it has associated Purchase Order Line Item "
                        "which is either closed or has receipts associated with it. You may want to consider cancelling this Sales Order Item instead."), 0, "" },
  { "deleteSOItem",  -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Sales Order Item was deleted successfully. However, the Purchase Order Line Item "
                        "associated with this Sales Line could not be deleted. You must delete this Purchase Line Item seperately if desired."), 0, "" },

  { "deleteSubaccount", -1,
                        QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Subaccount cannot be deleted as it "
                           "is in use by existing Account. You must reclass "
                           "these Accounts before you may delete the selected "
                           "Subaccount."),                              0, "" },

  { "deleteTO",		-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order cannot be deleted as some of "
			       "its line items have already been shipped."),
								 0, "" },
  { "deleteTO",		-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order cannot be deleted as some of "
			       "its line items have already been issued to "
			       "shipping. You must return this stock before "
			       "you may delete this Transfer Order."),
			       					 0, "" },

  { "deleteTax", -10,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Code cannot be deleted as there are "
			   "Tax Assignments that refer to it. Change those "
			   "Tax Assignments before trying to delete this "
			   "Tax Code."),				0, "" },
    
  { "deleteTaxAuthority", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority cannot be deleted as "
				 "there are Tax Selections for it. Change or "
				 "delete those Tax Selections before deleting "
				 "this Tax Authority."),		0, "" },
  { "deleteTaxAuthority", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Authority cannot be deleted as "
				 "Payments have been written to it."),	0, "" },

  { "deleteTaxClass", -1,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Class cannot be deleted as there are "
			   "Tax Codes that refer to it."),	0, "" },
  
  { "deleteTaxZone", -1,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Zone cannot be deleted as there are "
			   "Tax Assignments that refer to it."),	0, "" },
  { "deleteTaxZone", -2,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Tax Zone cannot be deleted as there are "
			   "Tax Registrations that refer to it."),	0, "" },
  
  { "deleteTo",	 -1,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order cannot be deleted as line "
			   "items for it have already been shipped."),	0, "" },
  { "deleteTo",	 -2,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order cannot be deleted as line "
			   "items for it have been issued to shipping."),0, "" },
  { "deleteTo",	 -3,	QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order cannot be deleted as the "
			   "order number cannot be released."),		0, "" },

  { "deleteVendor",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "have been P/Os created against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "has been P/O Material Receipt History posted "
			       "against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "has been P/O Material Return History posted "
			       "against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "have been Vouchers posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "have been A/P Open Items posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there"
			       "have been A/P Applications posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },
  { "deleteVendor",	-7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Vendor cannot be deleted as there "
			       "have been Payments posted against it. "
			       "You may deactivate this Vendor instead."),
								 0, "" },

  { "deleteWo",		-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Work Order cannot be deleted because time "
			       "clock entries exist for it. Please Close it "
			       "instead of trying to Delete it."),
								 0, "" },
  { "deleteWo",		-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Work Order cannot be deleted for Job Item Types. "
             "Please close the associated Sales Order "
			       "instead of trying to Delete it."),
								 0, "" },
  { "deleteWo",		-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Work Order cannot be deleted in the current status. "
             "Please close the associated Sales Order "
             "instead of trying to Delete it."),
                 0, "" },
  { "deleteWo",		-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Work Order cannot be deleted as it still has "
             "WIP value against it.  Please reverse the material issue or close the W/O "
             "instead of trying to Delete it."),
                 0, "" },

  { "disablePackage", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This version of the PostgreSQL database server "
                             "does not support package enabling or disabling. "
                             "Upgrade to PostgreSQL 8.2 or later."),    0, "" },
  { "disablePackage", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not find a package with the internal id % "
                             "to enable or disable."),                  0, "" },

  { "distributeVoucherLine", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Distribution would result in zero "
                                    "quantity and amount."
                                    "Please distribute manually."),     0, "" },
  { "distributeVoucherLine", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The purchase order and voucher have "
                                    "different currencies. "
                                    "Please distribute manually."),     0, "" },
  { "distributeVoucherLine", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Distribution would result in a negative "
                                    "amount. "
                                    "Please distribute manually."),     0, "" },
  { "distributeVoucherLine", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Item has multiple cost elements. "
                                    "Please distribute manually."),     0, "" },

  { "distributetodefault", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There was an error distributing to default location. "
                                                 "There is no default location defined for this Item Site."),     0, "" },
  { "distributetodefault", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There was an error distributing to default location. "
                                                 "There is no quantity to distribute for this Item Site."),     0, "" },


  { "editccnumber",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You must select Master Card, Visa, American "
			       "Express or Discover as the credit card type."),
								 0, "" },
  { "editccnumber",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The length of a Master Card credit card number "
			       "has to be 16 digits."),		 0, "" },
  { "editccnumber",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The length of a Visa credit card number "
			       "has to be either 13 or 16 digits."), 0, "" },
  { "editccnumber",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The length of an American Express credit card "
			       "number has to be 15 digits."),	 0, "" },
  { "editccnumber",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The length of a Discover credit card number "
			       "has to be 16 digits."),		 0, "" },
  { "editccnumber",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The first six digits for a valid Master Card "
			       "number must be between 510000 and 559999 or between 222100 and 272099"),
								 0, "" },
  { "editccnumber",	-7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The first digit for a valid Visa number must "
			       "be 4"),				 0, "" },
  { "editccnumber",	-8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The first two digits for a valid American "
			       "Express number must be 34 or 37."),
								 0, "" },
  { "editccnumber",	-9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The first four digits for a valid Discover "
			       "Express number must be 6011."),	 0, "" },
  { "editccnumber",    -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The credit card number that you have provided "
			       "is not valid."),		 0, "" },

  { "enablePackage",  -1, "", -1, "disablePackage" },
  { "enablePackage",  -2, "", -2, "disablePackage" },

  { "enterReceipt",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Information for this order line item could not "
			       "be found. If it is a Purchase Order Item then "
			       "it does not appear to exist. If it is a "
			       "Transfer Order Item then either the Transfer "
			       "Order does not exist or there is no Item Site "
			       "for this line item."),		0, "" },

  { "explodeWo",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded as there is no "
			 "valid Bill of Materials on file for the Work Order "
			 "Item. You must create a valid Bill of Materials "
			 "for the Work Order Item before you may explode the "
			 "Work Order."),				0, "" },
  { "explodeWo",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded as there are one "
			 "or more Component Items on the Bill of Materials for "
                         "the Work Order Item that are not valid in the Work "
			 "Order Site. You must create a valid Item Site "
			 "for all of the Component Items before you may "
			 "explode this Work Order."),			0, "" },
  { "explodeWo",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded as there are one "
			 "or more Co-Product/By-Product Items on the Breeder "
			 "Bill of Materials for the Work Order Item that do "
			 "not exist in the Work Order Site. You must "
			 "create a valid Item Site for all of the Co-Product/"
			 "By-Product Items before you may explode this "
			 "Work Order."),				0, "" },
  { "explodeWo",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded because it is not "
			 "Open."),					0, "" },
  { "explodeWo",  -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded because the quantity"
			 "ordered is not valid."),			0, "" },

  { "fkeycheck",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                                        "Cannot check dependencies when the "
                                        "contact is one of multiple foreign "
                                        "key columns."),                0, "" },

  { "freezeAccountingPeriod", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot freeze this Accounting Period "
				     "because it is still open."),     0, "" },
  { "freezeAccountingPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot freeze this Accounting Period "
				     "because it is already frozen."), 0, "" },

  { "getAssocLotSerialIds", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "pItemlocdistId is required."), 0, "" },

  { "insertGLTransaction", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "G/L Transaction can not be posted because "
             "the debit and credit accounts reference two different companies."), 0, "" },
  { "insertGLTransaction", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post a G/L transaction to a "
             "nonexistent period."), 0, "" },
  { "insertGLTransaction", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Nothing to do as the value to post to the "
			       "G/L is 0."), 0, "" },
  { "insertGLTransaction", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post to closed period."), 0, "" },
  { "insertGLTransaction", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
             "User does not have privilege to post to frozen period."), 0, "" },

  { "insertIntoGLSeries", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot add to a G/L Series because the "
				 "Account is NULL or -1."),	0, "" },
  { "insertIntoGLSeries", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot add to a G/L Series because the "
				 "Accounting Period is closed."), 0, "" },

  { "invAdjustment",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "invAdjustment",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Item is not eligible "
         "for Inventory Adjustments based on item type or itemsite cost method. "),  0, "" },
  { "invAdjustment",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "invAdjustment",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },


  { "invExpense",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "invExpense",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },
  { "invExpense",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."), 0, "" },

  { "invReceipt",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "invReceipt",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },
  { "invReceipt",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."), 0, "" },

  { "invScrap",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "invScrap",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),  0, "" },
  { "invScrap",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "invScrap",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },

  { "isControlledItemsite",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "No Itemsite Found."),  0, "" },

  { "isInventoryItemsite",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "No itemsite found."),  0, "" },

  { "issueAllBalanceToShipping",  -1, "",	 -1, "issueToShipping" },
  { "issueAllBalanceToShipping", -10, "",	-10, "issueToShipping" },
  { "issueAllBalanceToShipping", -12, "",	-12, "issueToShipping" },
  { "issueAllBalanceToShipping", -13, "",	-13, "issueToShipping" },
  { "issueAllBalanceToShipping", -14, "",	-14, "issueToShipping" },
  { "issueAllBalanceToShipping", -15, "",	-15, "issueToShipping" },
  { "issueAllBalanceToShipping", -20, "",	-20, "issueToShipping" },

  { "issueLineBalanceToShipping",  -1, "",	 -1, "issueToShipping" },
  { "issueLineBalanceToShipping", -10, "",	-10, "issueToShipping" },
  { "issueLineBalanceToShipping", -12, "",	-12, "issueToShipping" },
  { "issueLineBalanceToShipping", -13, "",	-13, "issueToShipping" },
  { "issueLineBalanceToShipping", -14, "",	-14, "issueToShipping" },
  { "issueLineBalanceToShipping", -15, "",	-15, "issueToShipping" },
  { "issueLineBalanceToShipping", -20, "",	-20, "issueToShipping" },

  { "issueToShipping",	-1, "",		 	 -1, "postInvTrans" },
  { "issueToShipping",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."), 0, "" },
  { "issueToShipping",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
             "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  { "issueToShipping",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Cost Category."),0, ""},
  { "issueToShipping", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Next Shipment Number has not been set in "
			       "the Configure S/R window. Set that value and "
			       "try issuing to shipping again."), 0, "" },
  { "issueToShipping", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order is on Credit Hold and "
			       "must be taken off of Credit Hold before any "
			       "inventory may be issued to it."),	0, "" },
  { "issueToShipping", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order is on Packing Hold "
			       "and must be taken off of Packing Hold before "
			       "any inventory may be issued to it."),	0, "" },
  { "issueToShipping", -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order is on Return Hold. "
			       "The Customer must return all materials for a "
			       "related Return Authorization before any inven"
			       "tory may be issued to this Order."),	0, "" },
  { "issueToShipping", -15, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Sales Order is configured for Auto Registration. "
			       "The Customer Account does not have a Primary Contact. "
			       "A Primary Contact must be assigned to this Customer Account before any inven"
			       "tory may be issued to this Order."),	0, "" },
  { "issueToShipping", -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is not enough Inventory to "
                                           "issue the amount required of one "
                                           "of the Average Cost items requested.  "
                                           "Average Cost items may not have a negative quantity on hand."),0, ""},

  { "issueWoMaterial", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."),0, ""},
  { "issueWoMaterial", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),0, ""},
  { "issueWoMaterial", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Expected Count of Distribution Detail Records Posted for Controlled Item."),0, ""},
  { "issueWoMaterial", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Qty to Issue Must be Positive"),0, ""},

  { "login",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The specified Username does not exist in the specified "
                     "Database. Contact your Systems Administrator to report "
                     "this issue"),                                     0, "" },
  { "login",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The specified Username exists in the specified Database "
                     "but is not Active. Contact your Systems Administrator "
                     "to report this issue."),                          0, "" },
  { "login",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The specified Database is currently in Maintenance Mode "
                     "and can only be accessed by System Administators. "
                     "Contact your Systems Administrator to report this issue."),
                                                                        0, "" },

  { "massReplaceBomitem",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot make this BOM Item replacement "
				  "because it would create a recursive BOM."),
									0, "" },

  { "openAccountingPeriod", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot open this Accounting Period because "
				   "it is already open."),		0, "" },
  { "openAccountingPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot open this Accounting Period because "
				   "it is frozen."),			0, "" },
  { "openAccountingPeriod", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot open this Accounting Period because "
                                    "subsequent periods are closed."),			0, "" },
  { "openAccountingPeriod", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot open this Accounting Period because "
                                  "the fiscal year is closed."),			0, "" },

  { "openAccountingYearPeriod", -1, "", -1, "openAccountingPeriod" },
  { "openAccountingYearPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot open this Accounting Year because "
                                     "subsequent years are closed."),			0, "" },

  { "openRecurringItems", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot count open recurring items with an "
                                  "invalid type."),                     0, "" },
  { "openRecurringItems", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot count open recurring items without "
                                  "a valid parent item."),              0, "" },
  { "openRecurringItems", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Don't know how to count open recurring "
                                  "invoices."),                         0, "" },

  { "postAPCreditMemoApplication", -1,
      QT_TRANSLATE_NOOP("storedProcErrorLookup", "There are no A/P Credit Memo applications to post."),         0, "" },
  { "postAPCreditMemoApplication", -2,
      QT_TRANSLATE_NOOP("storedProcErrorLookup", "There are no A/P Credit Memo applications to post."),         0, "" },
  { "postAPCreditMemoApplication", -3,
      QT_TRANSLATE_NOOP("storedProcErrorLookup", "The total value of the applications that are you attempting to post "
         "is greater than the value of the A/P Credit Memo itself." ),  0, "" },
  { "postAPCreditMemoApplication", -4,
	QT_TRANSLATE_NOOP("storedProcErrorLookup", "At least one A/P Credit Memo application cannot be posted because "
           "there is no current exchange rate for its currency."),      0, "" },
  { "postAPCreditMemoApplication", -5,
	QT_TRANSLATE_NOOP("storedProcErrorLookup", "The A/P Credit Memo to apply was not found."),              0, "" },
  { "postAPCreditMemoApplication", -6,
	QT_TRANSLATE_NOOP("storedProcErrorLookup", "The amount to apply for this A/P Credit Memo is NULL."),    0, "" },
  { "postAPCreditMemoApplication", -7,
    QT_TRANSLATE_NOOP("storedProcErrorLookup", "The value of the applications that are you attempting to post "
                      "is greater than the balance of the target A/P open item." ),  0, "" },

  { "postARCreditMemoApplication", -1,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "There are no A/R Credit Memo applications to post."),	0, "" },
  { "postARCreditMemoApplication", -2,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "Either there are no A/R Credit Memo applications to post or there"
	       " is no exchange rate for one of the applications."),	0, "" },
  { "postARCreditMemoApplication", -3,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "The total value of the applications that you are attempting to "
	       "post is greater than the value of the A/R Credit Memo itself. "
               "Please reduce the applications to total less than the value "
               "of the Credit Memo."),                                  0, "" },
  { "postARCreditMemoApplication", -4,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "At least one A/R Credit Memo application cannot be posted "
	       "because there is no current exchange rate for its currency."),
									0, "" },
  { "postARCreditMemoApplication", -5,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "The A/R Credit Memo to apply was not found."),		0, "" },

  { "postBankAdjustment", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Bank Adjustment could not be posted "
				 "because the one or more required records do "
				 "not exist."),				0, "" },
  { "postBankAdjustment", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Bank Adjustment could not be posted "
				 "because the total adjustment is 0 so there "
				 "is nothing to post."),		0, "" },

  { "postBankReconciliation", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Bank Reconciliation could not be "
				     "posted because the Ledger Account could not "
				     "be verified."),		0, "" },
  { "createInvoice",
			-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Billing Approval cannot be posted "
			       "because it has already been posted."),	0, "" },

  { "createInvoices",
                -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Ledger Account Assignments for one or more of the "
                       "Billing Approvals that you are trying to post are not "
                       "configured correctly. Therefore, G/L Transactions "
                       "cannot be posted for these. You must contact your "
                       "Systems Administrator to have this corrected before "
                       "you may post these Billing Approvals."),       0, "" },

  { "createItemlocdistParent", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Itemsite Must be Lot/Serial or Location Controlled."), 0, "" },
  { "createItemlocdistParent", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Series is required."), 0, "" },
  { "createItemlocdistParent", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Failed to Create Parent Itemlocdist Record."), 0, "" },

  { "postCashReceipt", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be posted as "
			      "the amount distributed is greater than the "
			      "amount received. You must correct this before "
			      "you may post this Cash Receipt."),	0, "" },
  { "postCashReceipt", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be posted as "
			      "the amount received must be greater than zero. You must correct this before "
			      "you may post this Cash Receipt."),	0, "" },
  { "postCashReceipt", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be posted as "
                               "the A/R Account cannot be determined. You must "
			       "make an A/R Account Assignment for the "
			       "Customer Type to which this Customer is "
			       "assigned before you may post this Cash "
			       "Receipt."),				0, "" },
  { "postCashReceipt", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be posted as "
			      "the Bank Account cannot be determined. You must "
			      "make a Bank Account Assignment for this Cash "
			      "Receipt before you may post it." ),	0, "" },
  { "postCashReceipt", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be posted, "
			      "probably because the Customer's Prepaid Account "
			      "was not found."),			0, "" },
  { "postCashReceipt", -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Cash Receipt because the "
                            "credit card records could not be found."), 0, "" },

  { "postCCCashReceipt",  -1, "", -1, "createARCreditMemo" },
  { "postCCCashReceipt", -10, "", -1, "postCCcredit" },
  { "postCCCashReceipt", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Cash Receipt because the "
                                 "record of the credit card transaction either "
                                 "does not exist or is not consistent."),
                                                                        0, "" },

  { "postCCcredit",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Credit Card refund because the "
                            "default Bank Account for this Credit Card "
                            "could not be found."),                     0, "" },
  { "postCCcredit",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Credit Card refund because an "
                            "invalid id/reference-type pair was passed."),
                                                                        0, "" },
  { "postCCcredit",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Credit Card refund because the "
                            "credit card and refund records could not be "
                            "found."),                                  0, "" },
  { "postCCcredit",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Credit Card refund because the "
                            "credit card payment records is not for a refund."),
                                                                        0, "" },

  { "postCheck",  -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Payment because it has already "
			 "been posted."),				0, "" },
  { "postCheck",  -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Payment because the recipient "
			 "type is not valid."),				0, "" },
  { "postCheck",  -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Payment because the Expense "
			 "Category could not be found."),		0, "" },
  { "postCheck",  -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot post this Payment because the Ledger Account "
			 "against which it is to be posted is not valid."),
			 						0, "" },
  { "postCountTag",  -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                           "Cannot post this Count Tag because The total Count "
                           "Slip quantity is greater than the Count Tag "
                           "quantity."),                                0, "" },
  { "postCountTag",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                           "Cannot post this Count Tag because the total Count "
                           "Slip quantity is less than the Count Tag quantity "
                           "for a Lot/Serial-controlled Item Site."),   0, "" },
  { "postCountTag",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                           "Cannot post this Count Tag because the total Count "
                           "Slip quantity is less than the Count Tag quantity "
                           "and there is no default location."),        0, "" },
  { "postCountTag",  -4, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                           "Cannot post this Count Tag because the total Count "
                           "Slip quantity is less than the Count Tag quantity "
                           "and we don't post to default locations."),  0, "" },

  { "postCountTagLocation", -1, "", -1, "postCountTag" },
  { "postCountTagLocation", -2, "", -2, "postCountTag" },
  { "postCountTagLocation", -3, "", -3, "postCountTag" },
  { "postCountTagLocation", -4, "", -4, "postCountTag" },

  { "postCreditMemo",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Posting Distribution Detail Returned 0 Results."),  0, "" },
  { "postCreditMemo",	-10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Return cannot be posted because it "
			       "has already been posted."),	 0, "" },
  { "postCreditMemo",	-11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Return is on Hold and, thus, cannot "
			       "be posted."),			 0, "" },
  { "postCreditMemo",	-12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Sales Account Assignment for this Return "
			       "is not configured correctly. Because of "
			       "this, G/L Transactions cannot be posted for "
			       "this Return. You must contact your "
			       "Systems Administrator to have this corrected "
			       "before you may post this Return."),
								 0, "" },
  { "postCreditMemo",	-14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Misc. Charge Account Assignment for this Return "
			       "is not configured correctly. Because of "
			       "this, G/L Transactions cannot be posted for "
			       "this Return. You must contact your "
			       "Systems Administrator to have this corrected "
			       "before you may post this Return."),
								 0, "" },
  { "postCreditMemo",	-16, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Freight Account Assignment for this Return "
			       "is not configured correctly. Because of "
			       "this, G/L Transactions cannot be posted for "
			       "this Return. You must contact your "
			       "Systems Administrator to have this corrected "
			       "before you may post this Return."),
								 0, "" },
  { "postCreditMemo",	-18, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The A/R Account Assignment for this Return "
			       "is not configured correctly. Because of "
			       "this, G/L Transactions cannot be posted for "
			       "this Return. You must contact your "
			       "Systems Administrator to have this corrected "
			       "before you may post this Return."),
								 0, "" },

  { "postDistDetail", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Expected a Count of the Distribution Detail "
         "Records Posted."), 0, "" },
  { "postDistDetail", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Expected a Count of the Distribution Detail "
         "Records Posted."), 0, "" },
  { "postDistDetail", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Expected a Count of the Distribution Detail "
         "Records Posted."), 0, "" },
  { "postDistDetail", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Expected a Count of the Distribution Detail "
         "Records Posted."), 0, "" },

  { "postGLSeries", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post this G/L Series because the "
				 "Accounting Period is closed."), 0, "" },
  { "postGLSeries", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post this G/L Series because the G/L "
			   "Series Discrepancy Account was not found."),
								 0, "" },
  { "postGLSeriesNoSumm", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post this G/L Series because the "
				 "Debits and Credits are unbalanced."), 0, "" },
  { "postGLSeriesNoSumm", -4, "", -4, "postGLSeries" },
  { "postGLSeriesNoSumm", -5, "", -5, "postGLSeries" },

  { "postIntoInvBalance", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                                                "No accounting period exists for invhist_id %1, transaction date %2" ),
    0, "" },
  { "postIntoInvBalance", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup",
                                                "Average costed Item with negative balance for invhist_id %1, transaction date %2" ),
    0, "" },
  
  { "postInvoice",  -1, "", -1, "insertIntoGLSeries" },
  { "postInvoice",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."), 0, "" },
  { "postInvoice",  -4, "", -4, "insertIntoGLSeries" },
  { "postInvoice",  -5, "", -5, "postGLSeries" },
  { "postInvoice",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Posting Distribution Detail Returned 0 Results."),  0, "" },
  { "postInvoice", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because it has "
                                          "already been posted."), 0, "" },
  { "postInvoice", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because the Sales "
                                          "Account was not found."), 0, "" },
  { "postInvoice", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because there was an "
                                          "error processing Line Item taxes."), 0, "" },
  { "postInvoice", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because there was an "
                                          "error processing Misc. Line Item taxes."), 0, "" },
  { "postInvoice", -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because the Freight "
                                          "Account was not found."),	0, "" },
  { "postInvoice", -15, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because there was an "
                                          "error processing Freight taxes."),	 0, "" },
  { "postInvoice", -16, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because there was an "
                                          "error processing Tax Adjustments."), 0, "" },
  { "postInvoice", -17, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Invoice because the A/R "
                                          "Account was not found."), 0, "" },
  { "postInvoice", -18, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
             "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  
  { "postInvHist", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup",
             "Post into Inventory Balance for invhist_id=%1 was unsuccessful" ), 0, "" },
  
  { "postInvTrans",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post an inventory transaction because"
			       " the Item Site has no Control Method or the Item has an Item Type of Reference."), 0, "" },
  { "postInvTrans",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post an inventory transaction because"
			       " the transaction will cause an Average Costed Item to go negative which is not allowed."), 0, "" },
  { "postInvTrans",	-3, "",	 -3, "insertGLTransaction" },
  { "postInvTrans",	-4, "",	 -4, "insertGLTransaction" },
  { "postInvTrans",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Could not post this inventory transaction because"
			       " the transaction will cause the Item Qty. on Hand to go negative which is not allowed."), 0, "" },
  { "postInvTrans", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This transaction will cause an item to go negative"
             " and negative inventory is currently disallowed."), 0, "" },
  { "postInvTrans", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
             "Transaction series must be provided."), 0, "" },
  { "postInvTrans", -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
             "Could not find the itemlocSeries for the pInvhistId provided."), 0, "" },

  { "postPoReceipt",	-1, "",	 -1, "postReceipt" },
  { "postPoReceipt",	-2, "",	 -2, "postReceipt" },
  { "postPoReceipt",	-3, "",	 -3, "postReceipt" },
  { "postPoReceipt",	-4, "",	 -4, "postReceipt" },
  { "postPoReceipt",   -10, "",	-10, "postReceipt" },
  { "postPoReceipt",   -11, "", -11, "postReceipt" },
  { "postPoReceipt",   -12, "", -12, "postReceipt" },

  { "postProduction", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Production because the Work Order "
                             "status is not Exploded, Released, or InProcess."), 0, "" },
  { "postProduction", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Production because backflushing "
                             "component usage could not be completed due "
                             "to missing Item Sites."),	0, "" },
  { "postProduction", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to post this Production because of missing "
                           "Item Site or Cost Category."), 0, "" },
  { "postProduction", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."), 0, "" },
  { "postProduction", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  { "postProduction", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing cost category."), 0, "" },

  { "postReceipt",  -1, "",	 -1, "postInvTrans" },
  { "postReceipt",  -2, "",	 -2, "postInvTrans" },
  { "postReceipt",  -3, "",	 -3, "insertGLTransaction" },
  { "postReceipt",  -4, "",	 -4, "insertGLTransaction" },
  { "postReceipt",  -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."), 0, "" },
  { "postReceipt", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Receipt Line has already been posted."),
  									0, "" },
  { "postReceipt", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Receipt Line cannot be "
			   "posted because it has a quantity of 0."),	0, "" },
  { "postReceipt", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Purchase Order Receipt Line has no "
			   "Standard Cost assigned to it."), 0, "" },
  { "postReceipt", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cant post receipt for this order type."), 0, "" },
  { "postReceipt", -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site."), 0, "" },
  { "postReceipt", -16, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot not issue item to shipping. "
			   "No Sales Order item found against this PO Item."), 0, "" },
  { "postReceipt", -17, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot not issue item to shipping. "
			   "Inventory history not found."),	0, "" },
  { "postReceipt", -20, "", -1, "issueToShipping" },
  { "postReceipt", -21, "", -10, "issueToShipping" },
  { "postReceipt", -22, "", -12, "issueToShipping" },
  { "postReceipt", -23, "", -13, "issueToShipping" },
  { "postReceipt", -24, "", -14, "issueToShipping" },
  { "postReceipt", -25, "", -15, "issueToShipping" },
  { "postReceipt", -26, "", -12, "shipShipment" },
  { "postReceipt", -27, "", -13, "shipShipment" },
  { "postReceipt", -28, "", -14, "shipShipment" },
  { "postReceipt", -29, "", -15, "shipShipment" },
  { "postReceipt", -30, "", -1, "shipShipment" },
  { "postReceipt", -31, "", -3, "shipShipment" },
  { "postReceipt", -32, "", -4, "shipShipment" },
  { "postReceipt", -33, "", -5, "shipShipment" },
  { "postReceipt", -34, "", -6, "shipShipment" },
  { "postReceipt", -35, "", -8, "shipShipment" },
  { "postReceipt", -36, "", -50, "shipShipment" },
  { "postReceipt", -37, "", -99, "shipShipment" },
  { "postReceipt", -38, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  { "postReceipt", -39, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."), 0, "" },
  { "postReceipt", -40, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Can not post receipt for qty <= 0. Please correct qty and try again."), 0, "" },
  { "postReceipt", -41, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  { "postReceipt", -42, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Could not insert G/L transaction: no cost category found for itemsite_id."), 0, "" },

  { "postSoItemProduction", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "pItemlocSeries is Required when pPreDistributed."), 0, "" },
  { "postSoItemProduction", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "The postSoLineBalanceProduction function may only be used with Job costed item sites."), 0, "" },

  { "postPoReceipts",   -1, "",	 -1, "postPoReceipt" },
  { "postPoReceipts",   -3, "",	 -3, "postPoReceipt" },
  { "postPoReceipts",   -4, "",	 -4, "postPoReceipt" },
  { "postPoReceipts",  -10, "",	-10, "postPoReceipt" },
  { "postPoReceipts",  -12, "",	-12, "postPoReceipt" },

  { "postPoReturns", -1, "", -1, "postInvTrans" },
  { "postPoReturns", -3, "", -3, "insertGLTransaction" },
  { "postPoReturns", -4, "", -4, "insertGLTransaction" },
  { "postPoReturns", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required"), 0, ""  },
  { "postPoReturns", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required"), 0, ""  },
  { "postPoReturns", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "postPoReturns", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
              "Expected Count of Distribution Detail Records Posted for Controlled Item."),  0, "" },

  { "postValueIntoInvBalance", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An inventory balance record was not found "
                               "for item %1.  This is typically caused by that item missing standard costs"), 0, "" },

  { "postVoucher",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The Cost Category for one or more Item Sites "
			       "for the Purchase Order covered by this Voucher "
			       "is not configured with Purchase Price Variance "
			       "or P/O Liability Clearing Account Numbers or "
			       "the Vendor of this Voucher is not configured "
			       "with an A/P Account Number. Because of this, "
			       "G/L Transactions cannot be posted for this "
			       "Voucher."),
								0, "" },

  { "_raheadBeforeUpdateTrigger", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You do not have privileges to change "
                               "a Return Authorization."), 0, "" },
  { "_raheadBeforeUpdateTrigger", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Returns may not be saved with disposition "
                               " of Credit and Credit Method of None."), 0, "" },
  { "_raheadBeforeUpdateTrigger", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The authorization number may not be changed."),
                                   0, "" },

  { "recallShipment",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled because it "
			       "does not appear to have been shipped."),
								0, "" },
  { "recallShipment",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled because it "
			       "appears to have been invoiced."),
								0, "" },
  { "recallShipment",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled "
			       "because it has already been received "
			       "at its destination."),		0, "" },
  { "recallShipment",	-4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled because it "
                             "appears to have been invoiced and the invoice has been posted."),
                                                              0, "" },
  { "recallShipment",	-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled because it "
			       "contains one or more Line Items with Site/"
			       "Product Category/Customer combinations that "
			       "have not been properly described in Sales "
			       "Account Assignments. These assignments must be "
			       "made before G/L Transactions can be posted and"
			       "this Sales Order is allowed to be recalled."),
								0, "" },
  { "recallShipment",	-6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This shipment cannot be recalled "
                             "because the associated Transfer Order is closed."),		0, "" },

  { "releasePurchaseOrder",
			-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot release this Purchase Order because it "
			       "does not have any unreleased Purchase Order Items."),	0, "" },

  { "releaseTransferOrder", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot release this Transfer Order because "
                                   "it does not have any line items."), 0, "" },
  { "releaseUnusedBillingHeader",
			-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot release this Billing Header because it "
			       "has already been posted."),	0, "" },
  { "releaseUnusedBillingHeader",
			-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot release this Billing Header because it "
			       "has Line Items."), 		0, "" },

  { "relocateInventory", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "You cannot Relocate more inventory than is "
                                "available."),                  0, "" },

  { "replaceAllVoidedChecks", -1, "", -1, "replaceVoidedCheck" },

  { "replaceVoidedCheck", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot replace this voided Payment because "
				 "either it has not been voided, it has "
				 "already been posted, or it has already been"
				 "replaced."), 0, "" },

  { "returnCompleteShipment",
			-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Site Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },
  { "returnItemShipments", -1, "", -1, "postInvTrans" },
  { "returnItemShipments",
			-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Site Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },

  { "returnShipmentTransaction", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Cost Category for itemsite"), 0, "" },
  { "returnShipmentTransaction", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Cost Category for itemsite"), 0, "" },
  { "returnShipmentTransaction",
			-5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Either a Cost Category for the Items you are "
			       "trying to Return is not configured with a "
			       "Shipping Asset Account Number or a Customer "
			       "Type/Product Category/Site Sales Account "
			       "assignment does not exist . Because of this, " 
			       "G/L Transactions cannot be posted for this "
			       "Return. You must contact your Systems "
			       "Administrator to have this corrected before "
			       "you may Return this Shipment."),
								0, "" },
  { "returnShipmentTransaction", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Can not return shipment for this order type"), 0, "" },

  { "reverseapapplication", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The amount paid is less than the applied amount."),
                                	0, "" },
  { "reverseapapplication", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This application has already been reversed."),
                                	0, "" },
  { "reversearapplication", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The amount paid is less than the applied amount."),
                                	0, "" },
  { "reversearapplication", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This application has already been reversed."),
                                	0, "" },
  { "returnWoMaterial", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."), 0, "" },
  { "returnWoMaterial", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "No transaction found for invhist_id"), 0, "" },
  { "returnWoMaterial", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."), 0, "" },
  { "returnWoMaterial", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },
  { "returnWoMaterial", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An itemlocSeries is Required."), 0, "" },
  { "returnWoMaterial", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", 
         "Expected Count of Distribution Detail Records Posted for Controlled Item."), 0, "" },

  { "reverseCashReceipt", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be reversed as "
			      "the amount distributed is greater than the "
			      "amount received."),	0, "" },
  { "reverseCashReceipt", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be reversed as "
			      "the amount received must be greater than zero."),	0, "" },
  { "reverseCashReceipt", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be reversed as "
                               "the A/R Account cannot be determined. You must "
			       "make an A/R Account Assignment for the "
			       "Customer Type to which this Customer is "
			       "assigned before you may reverse this Cash "
			       "Receipt."),				0, "" },
  { "reverseCashReceipt", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be reversed as "
			      "the Bank Account cannot be determined. You must "
			      "make a Bank Account Assignment for this Cash "
			      "Receipt before you may reverse it." ),	0, "" },
  { "reverseCashReceipt", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Cash Receipt cannot be reversed, "
			      "probably because the Customer's Prepaid Account "
			      "was not found."),			0, "" },
  { "reverseCashReceipt", -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot reverse this Cash Receipt because the "
                            "credit card records could not be found."),  0, "" },

  { "saveAlarm", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "An alarm for this item already exists."), 0, "" },

  { "selectForBilling",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The quantity you have approved for Billing is "
			       "less than the quantity shipped. You may not "
			       "bill for less than the quantity shipped."),
								0, "" },

  { "selectPayment",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Document cannot be approved because it "
                                            "cannot be found or is the wrong document type."),
    0, "" },
  { "selectPayment",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Document cannot be approved because the "
                                            "balance is less than the amount plus discount."),
    0, "" },
  { "selectPayment",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Bank account company does not match "
                                            "the voucher distribution account company."),
    0, "" },
  
  { "shipShipment",  -1, "",	 -1, "postInvTrans"		},
  { "shipShipment",  -3, "",	 -3, "insertGLTransaction"	},
  { "shipShipment",  -4, "",	 -4, "insertGLTransaction"	},
  { "shipShipment",  -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Sales Order may not be shipped as it "
			    "contains one or more Line Items that have "
			    "Site/Product Category/Customer combinations "
			    "that have not been properly described in Sales "
			    "Account Assignments.  These assignments must be "
			    "made before G/L Transactions can be posted and "
			    "this Sales Order is allowed to ship."),	0, "" },
  { "shipShipment",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Transfer Order may not be shipped "
			    "because there is no Item Site for the "
			    "Transit Site."),			0, "" },
  { "shipShipment",  -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Shipment cannot be shipped because it "
			    "appears to have already shipped."),	0, "" },
  { "shipShipment", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Order is on Credit Hold and must be "
			    "taken off of Credit Hold before it "
			    "may be shipped."),				0, "" },
  { "shipShipment", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Order is on Packing Hold and must be "
			     "taken off of Packing Hold before it "
			     "may be shipped."),			0, "" },
  { "shipShipment", -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Order is on Return Hold. The Customer"
			     " must return all materials for a related Return "
			     "Authorization before this order may be shipped."),
			     						0, "" },
  { "shipShipment", -15, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The selected Order is on Shipping Hold and must "
			     "be taken off of Shipping Hold before it "
			     "may be shipped."),			0, "" },
  { "shipShipment", -50, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Shipment cannot be shipped because it "
			    "does not appear to exist."),		0, "" },
  { "shipShipment", -99, QT_TRANSLATE_NOOP("storedProcErrorLookup", "This Order may not be shipped because it has been "
			    "marked as Ship Complete and quantities for one or "
			    "more Line Items are still not completely issued. "
			    "Please correct this before shipping the Order."),
									0, "" },
  { "splitReceipt", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Only Purchase Order Receipts may be split."),		0, "" },
  { "splitReceipt", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Only posted receipts may be split."),		0, "" },
  { "splitReceipt", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Vouchered receitps may not be split."),		0, "" },
  { "splitReceipt", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Split quantity must me less than original receipt quantity."),		0, "" },
  { "splitReceipt", -5, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Split freight may not be greater than original freight."),		0, "" },
  { "splitReceipt", -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Receipt not found."),		0, "" },
  { "splitReceipt", -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The split quantity must be a positive number."),		0, "" },

  { "splitRecurrence", -10, "",                   -10, "createRecurringItems" },
  { "splitRecurrence", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot create recurring items without a valid "
                               "parent item to copy."),                 0, "" },

  { "sufficientInventoryToShipItem", -1,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot figure out which line item to issue."),	0, "" },
  { "sufficientInventoryToShipItem", -2,
	    QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is not enough Inventory to issue the amount required of "
	       "Item %1 in Site %2."),			0, "" },
  { "sufficientInventoryToShipItem", -3, 
            QT_TRANSLATE_NOOP("storedProcErrorLookup", "Item Number %1 in Site %2 is a Multiple Location or "
               "Lot/Serial controlled Item which is short on Inventory. "
               "This transaction cannot be completed as is. Please make "
               "sure there is sufficient Quantity on Hand before proceeding."),
								0, "" },
  { "sufficientInventoryToShipItem", -4,
    QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is not enough Inventory Reserved to issue the amount required of "
                      "Item %1 in Site %2."),			0, "" },
  { "sufficientInventoryToShipItem", -11,
            QT_TRANSLATE_NOOP("storedProcErrorLookup", "Invalid Order Type.  Only Sales Orders and Transfer Orders "
               "may be shipped from this window."),
								0, "" },                            
  { "sufficientInventoryToShipOrder", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot check inventory levels for"
                                             "an invalid item."),       0, ""},
  { "sufficientInventoryToShipOrder", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is not enough Inventory to "
                                             "issue the amount required of one "
                                             "of the items requested."),0, ""},
  { "sufficientInventoryToShipOrder", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "One of the requested items is a "
                                             "Multiple Location or Lot/Serial "
                                             "controlled Item which is sort on "
                                             "Inventory."),             0, ""},
  { "sufficientInventoryToShipOrder", -4, QT_TRANSLATE_NOOP("storedProcErrorLookup", "There is not enough Inventory Reserved to "
                                                            "issue the amount required of one "
                                                            "of the items requested."),0, ""},
  { "sufficientInventoryToShipOrder",-11, "",-11, "sufficientInventoryToShipItem"},

  { "thawAccountingPeriod", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot thaw this Accounting Period "
				     "because it is closed."), 0, "" },

  { "thawAccountingPeriod", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot thaw this Accounting Period "
				     "because it is not frozen."), 0, "" },

  { "todoItemMove",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot change the Sequence of a non-existent "
			       "To-Do List Item. Possible cause: no To-Do "
			       "List Item was selected."),
								0, "" },

  { "unreleasePurchaseOrder",
    -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot unrelease this Purchase Order because it "
                          "has in process Purchase Order Items."),	0, "" },
  
  { "updateTodoItem",	-1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be updated as "
			       "there is no assigned User."),	0, "" },
  { "updateTodoItem",	-2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be updated as "
			       "the Task Name is blank."),	0, "" },
  { "updateTodoItem",	-3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be updated as "
			       "there is no Due Date."),	0, "" },
  { "updateTodoItem",  -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "The To-Do List Item cannot be updated as "
			       "an invalid internal ID was supplied ."),
								 0, "" },

  { "voidAPOpenVoucher", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher Id % as apopen not found. "),				0, "" },
  { "voidAPOpenVoucher", -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher #% as vohead not found. "),				0, "" },
  { "voidAPOpenVoucher", -30, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher #% as applications exist. "),				0, "" },
  { "voidAPOpenVoucher", -40, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher #% due to unassigned G/L Accounts. "),				0, "" },
  { "voidAPOpenVoucher", -50, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher #% due to unassigned G/L Accounts. "),				0, "" },
  { "voidAPOpenVoucher", -60, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Voucher #% due to an unassigned A/P Account. "),				0, "" },
  { "voidAPOpenVoucher", -70, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Credit application failed with result %. "),				0, "" },

  { "voidCheck", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because either it has already "
			"been voided, posted, or replaced, or it has been "
			"transmitted electronically. If this Payment has been "
			"posted, try Void Posted Payment with the Payment Register "
			"window."),					0, "" },

  { "voidCreditMemo",  -1, "", -1, "insertIntoGLSeries" },
  { "voidCreditMemo",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "voidCreditMemo",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "pItemlocSeries is Required when pPreDistributed."),  0, "" },
  { "voidCreditMemo",  -4, "", -4, "insertIntoGLSeries" },
  { "voidCreditMemo",  -5, "", -5, "postGLSeries" },
  { "voidCreditMemo",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Posting Distribution Detail Returned 0 Results."),  0, "" },
  { "voidCreditMemo",  -7, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Failed to create GL entry, no accnt record found."),  0, "" },
  { "voidCreditMemo",  -8, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Failed to create GL entry, no accnt record found."),  0, "" },
  { "voidCreditMemo",  -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Failed to create GL entry, no accnt record found."),  0, "" },
  { "voidCreditMemo", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Credit Memo because it has "
                         "not been posted."),		 0, "" },
  { "voidCreditMemo", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Credit Memo because the Sales "
                         "Account was not found."),		 0, "" },
  { "voidCreditMemo", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Credit Memo as cmhead not found."),  0, "" },
  { "voidCreditMemo", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot Void Credit Memo as aropen not found."),  0, "" },
  { "voidCreditMemo", -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Credit Memo because there "
                         "A/R Applications posted against this Credit Memo."), 0, "" },

  { "voidInvoice",  -1, "", -1, "insertIntoGLSeries" },
  { "voidInvoice",  -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Missing Item Site or Cost Category."),  0, "" },
  { "voidInvoice",  -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "pItemlocSeries is Required when pPreDistributed."),  0, "" },
  { "voidInvoice",  -4, "", -4, "insertIntoGLSeries" },
  { "voidInvoice",  -5, "", -5, "postGLSeries" },
  { "voidInvoice",  -6, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Posting Distribution Detail Returned 0 Results."),  0, "" },
  { "voidInvoice", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Invoice because it has "
                         "not been posted."),		 0, "" },
  { "voidInvoice", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Invoice because the Sales "
                         "Account was not found."),		 0, "" },
  { "voidInvoice", -20, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Unable to void this Invoice because there "
                         "A/R Applications posted against this Invoice."), 0, "" },

  { "voidPostedCheck", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because it has already "
			       "been voided."),				0, "" },
  { "voidPostedCheck", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because the recipient "
			       "type is not valid."),			0, "" },
  { "voidPostedCheck", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because the Expense "
			       "Category could not be found."),		0, "" },
  { "voidPostedCheck", -13, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because the Ledger Account "
			       "to which the funds should be credited is not "
			       "valid."),				0, "" },
  { "voidPostedCheck", -14, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot void this Payment because the Payment has "
			       "has been reconciled in Bank Reconciliation."),		0, "" },

  { "replaceVoidedCheck", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot replace a Payment that is not voided "
                                 "or has already been posted or replaced."),
                                                                        0, "" },
  { "replaceVoidedCheck", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot replace this voided Payment because "
                                 "one of its line items has been reapproved "
                                 "for billing and is represented on another "
                                 "Payment."),                             0, "" },

  { "reserveSoLineBalance", -1, "", -1, "reserveSoLineQty" },
  { "reserveSoLineBalance", -2, "", -2, "reserveSoLineQty" },
  { "reserveSoLineBalance", -3, "", -3, "reserveSoLineQty" },

  { "reserveSoLineQty", -1, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot reserve more quantity than remaining on order."), 0, ""},
  { "reserveSoLineQty", -2, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot reserve negative quantities."), 0, ""},
  { "reserveSoLineQty", -3, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Cannot reserve more quantity than currently on hand and already reserved."), 0, ""},


  { "woClockIn",  -1, "", -1, "explodeWo" },
  { "woClockIn",  -2, "", -2, "explodeWo" },
  { "woClockIn",  -3, "", -3, "explodeWo" },
  { "woClockIn",  -9, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 cannot be Exploded as it seems to have "
			 "an invalid Order Quantity."),			0, "" },
  { "woClockIn", -10, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 has at least one Item in its Bill of "
			 "Materials with the Push issue method that has not "
			 "yet been issued. You must issue all Push Items "
			 "to this Work Order."),			0, "" },
  { "woClockIn", -11, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 has at least one Item in its Bill of "
			 "Materials with the Push issue method that does not "
			 "have the required quantity issued. You must issue "
			 "all Push Items to this Work Order."),		0, "" },
  { "woClockIn", -12, QT_TRANSLATE_NOOP("storedProcErrorLookup", "Work Order %1 is closed."),			0, "" },
};

void initErrorLookupHash()
{
  if (! ErrorLookupHash.isEmpty())
  {
    QMessageBox::critical(0, QCoreApplication::translate("storedProcErrorLookup", "ErrorLookupHash initialization error"),
		QCoreApplication::translate("storedProcErrorLookup", "ErrorLookupHash has already been initialized."));
    return;
  }

  unsigned int numElems = sizeof(errors) / sizeof(errors[0]);
  for (unsigned int i = 0; i < numElems; i++)
  {
    QPair<int, QString> currPair;
    if (errors[i].msgPtr == 0)
      currPair = qMakePair(errors[i].retVal, QCoreApplication::translate("storedProcErrorLookup", errors[i].msg));
    else
    {
      QList<QPair<int, QString> > proxyList;

      QString proxyname;
      if (errors[i].proxyName.isEmpty())
	proxyname = errors[i].procName.toUpper();
      else
	proxyname = errors[i].proxyName.toUpper();

      proxyList = ErrorLookupHash.values(proxyname);

      if (proxyList.size() > 0)
      {
	int proxyIndex;
	for (proxyIndex = 0; proxyIndex < proxyList.size(); proxyIndex++)
	{
	  if (proxyList.at(proxyIndex).first == errors[i].msgPtr)
	  {
	    currPair = qMakePair(errors[i].retVal,
				 QString(proxyList.at(proxyIndex).second));
	    break;
	  }
	}

	if (proxyIndex >= proxyList.size())
	{
          QMessageBox::critical(0, QCoreApplication::translate("storedProcErrorLookup", "Lookup Error"),
                                QCoreApplication::translate("storedProcErrorLookup",
                                   "Could not find (%1, %2) in ErrorLookupHash "
                                   "when trying to insert proxy entry for "
                                   "(%3, %4).")
			   .arg(proxyname).arg(errors[i].msgPtr)
			   .arg(errors[i].procName).arg(errors[i].retVal));
	  continue;
	}
      }
      else // couldn't find proxyList => forward reference
      {
	unsigned int j;
	for (j = i + 1; j < numElems; j++)
	{
	  if (errors[j].retVal == errors[i].msgPtr &&
	      errors[j].procName.toUpper() == proxyname)
	    {
	    currPair = qMakePair(errors[i].retVal,
				 QString(QCoreApplication::translate("storedProcErrorLookup", errors[j].msg)));
	    break;
	    }
	}
	if (j >= numElems)
	{
          QMessageBox::critical(0, QCoreApplication::translate("storedProcErrorLookup", "Lookup Error"),
                                QCoreApplication::translate("storedProcErrorLookup",
                                   "Could not find (%1, %2) in ErrorLookupHash "
                                   "when trying to insert proxy entry for "
                                   "(%3, %4).")
			  .arg(errors[i].proxyName).arg(errors[i].msgPtr)
			  .arg(errors[i].procName).arg(errors[i].retVal));
	continue;
	}
      }
    }

    ErrorLookupHash.insert(errors[i].procName.toUpper(), currPair);
  } // for
}


QString storedProcErrorLookup(const QString procName, const int retVal)
{
  QString returnStr = "";

  if (ErrorLookupHash.isEmpty())
    initErrorLookupHash();

  QList<QPair<int, QString> > list = ErrorLookupHash.values(procName.toUpper());

  for (int i = 0; i < list.size(); i++)
  {
    if (list.at(i).first == retVal)
    {
      returnStr = list.at(i).second;
      break;
    }
  }

  if (returnStr.isEmpty())
    returnStr = QCoreApplication::translate("storedProcErrorLookup", "A Stored Procedure failed to run properly.");

  returnStr = QString("<p>") + returnStr +
	      QString("<br>(%1, %2)<br>").arg(procName).arg(retVal);

  return returnStr;
}

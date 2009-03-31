// Define local variables
var _linenumCol = 1;
var _itemCol    = 2;	
var _qtyCol     = 7;
var _priceCol   = 9;

var _billAddrNo = "";
var _linenumber = 1;
var _populating = false;

var _extList = new Array();
var _taxList = new Array();

var _newMode    = 0;
var _editMode   = 1;
var _viewMode   = 2;

var _add		= mywindow.findChild("_add");
var _address		= mywindow.findChild("_address");
var _cancel		= mywindow.findChild("_cancel");
var _cust      		= mywindow.findChild("_cust");
var _custTab		= mywindow.findChild("_custTab");
var _extendedPrice	= mywindow.findChild("_extendedPrice");
var _item		= mywindow.findChild("_item");
var _itemGroup    	= mywindow.findChild("_itemGroup");
var _itemsTab		= mywindow.findChild("_itemsTab");
var _number		= mywindow.findChild("_number");
var _qty		= mywindow.findChild("_qty");
var _remove		= mywindow.findChild("_remove");
var _sale		= mywindow.findChild("_sale");
var _saleitem		= mywindow.findChild("_saleitem");
var _saleitems		= mywindow.findChild("_saleitems");
var _salesrep		= mywindow.findChild("_salesrep");
var _save		= mywindow.findChild("_save");
var _scheddate		= mywindow.findChild("_scheddate");
var _shipto		= mywindow.findChild("_shipto");
var _shipvia		= mywindow.findChild("_shipvia")
var _site		= mywindow.findChild("_site");
var _subtotal   	= mywindow.findChild("_subtotal");
var _tab		= mywindow.findChild("_tab");
var _tax		= mywindow.findChild("_tax");
var _taxauth		= mywindow.findChild("_taxauth");
var _total		= mywindow.findChild("_total");	
var _unitPrice  	= mywindow.findChild("_unitPrice");

// Set Columns
with (_saleitems)
{
  setColumn("Line#"  		, 40,  0, true,  "line_number");
  setColumn("Item"   		, -1,  0, true,  "item_number");
  setColumn("Customer P/N"   	, 100, 0, false, "customer_pn");
  setColumn("Substitute for"   	, 100, 0, false, "substitute_for");
  setColumn("Site"   		, 40,  0, false, "sold_from_site");
  setColumn("Status"   		, 40,  0, false, "status");
  setColumn("Quantity"		, 100, 0, true,  "qty_ordered");
  setColumn("Qty UOM"   	, 40,  0, false, "qty_uom");
  setColumn("Price" 		, 100, 0, true,  "net_unit_price");
  setColumn("Price UOM"   	, 40,  0, false, "price_uom");
  setColumn("Extended"		, 100, 0, true,  "extension");
  setColumn("Sched. Date"	, 100, 0, true,  "scheduled_date");
  setColumn("Promise Date"	, 80,  0, false, "promise_date");
  setColumn("Warranty"   	, 40,  0, false, "warranty");
  setColumn("Tax Type"   	, 80,  0, false, "tax_type");
  setColumn("Tax Code"   	, 80,  0, false, "tax_code");
  setColumn("Discount"   	, 40,  0, false, "discount_pct_from_list");
  setColumn("Create Order"   	, 40,  0, false, "create_order");
  setColumn("Overwrite P/O"   	, 40,  0, false, "overwrite_po_price");
  setColumn("Notes"   		,100,  0, false, "notes");
  setColumn("Alt. COS"   	,100,  0, false, "alternate_cos_account");
}

// Define connections
_add.clicked.connect(add);
_cancel.clicked.connect(cancel);
_cust["newId(int)"].connect(handleButtons);
_cust["newId(int)"].connect(populateCustomer);
_item["newId(int)"].connect(handleButtons);
_item["newId(int)"].connect(itemPrice);
_item["valid(bool)"].connect(_add["setEnabled(bool)"]);
_qty["textChanged(const QString&)"].connect(itemPrice);
_qty["textChanged(const QString&)"].connect(extension);
_remove.clicked.connect(remove);
_saleitems["rowSelected(int)"].connect(rowSelected);
_saleitems["valid(bool)"].connect(_remove["setEnabled(bool)"]);
_salesrep["newID(int)"].connect(handleButtons);
_save.clicked.connect(save);
_unitPrice.valueChanged.connect(extension);

// Disable until we have a line item to work with
_itemGroup.enabled = false;

// Set precision
_qty.setValidator(toolbox.qtyVal());

// Misc Defaults
handleItem();

// Define local functions
function add()
{
  _saleitems.insert();
  _saleitem.clear();
  _saleitems.setValue(_saleitem.currentIndex(),_linenumCol,_linenumber);
  _saleitems.setValue(_saleitem.currentIndex(),_taxCol,0);
  _itemGroup.enabled = true;
  _linenumber = _linenumber + 1;
  _item.setFocus();
}

function cancel()
{
  if (_sale.mode == _newMode)
  {
    params = new Object;
    params.number = _number.text;
    toolbox.executeDbQuery("simplesalesorder","releasesonumber",params);
  }
  
  mydialog.reject();
}

function extension()
{
  var ext = _qty.text * _unitPrice.localValue;
  _extendedPrice.setLocalValue(ext)

  if (_populating)  // No need to keep recalculating totals
    return;

  try
  {
    var row = _saleitem.currentIndex();
    var tax;

    // Remove original values from total
    _subtotal.localValue = _subtotal.localValue -_extList[row];
    _tax.localValue = _tax.localValue -_taxList[row];

    // Update item extension
    _extList[row] = ext;
    _subtotal.localValue = _subtotal.localValue + ext;

    // Recalculate tax
    var params = new Object;
    params.taxauth_id = _taxauth.id();
    params.item_number = _item.itemNumber();
    params.qty = _qty.text - 0;
    params.unit_price = _unitPrice.localValue - 0;
  
    var data = toolbox.executeDbQuery("simplesalesorder","itemtax",params);
    if (data.first())
      tax = data.value("item_tax") - 0;
    else
      tax = 0 - 0;
    _taxList[row] = tax;
    _tax.localValue = _tax.localValue + tax;

    // Update total
    _total.localValue = _subtotal.localValue + _tax.localValue;
  }
  catch (e)
  {
    print(e);
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  } 
}

function handleItem() 
{
  _item.setQuery("SELECT DISTINCT item_id, item_number, item_descrip1,"
               + "                item_descrip2, uom_name, item_type,"
               + "                item_config, item_upccode,"
               + "                item_descrip1 || item_descrip2 AS itemdescrip "
               + "FROM item "
               + "   JOIN uom      ON (item_inv_uom_id=uom_id)"
               + "   JOIN itemsite ON (item_id=itemsite_item_id) "
               + "WHERE ((itemsite_warehous_id=" + _site.id() + ")"
               + "   AND (item_active)"
               + "   AND (item_sold) "
               + "   AND (itemsite_sold)"
               + "   AND (itemsite_active));");
}

function handleButtons()
{
  var state = (_cust.id() != -1 && 
               _salesrep.id() != -1 &&
              (_saleitems.rowCountVisible() > 1 ||
               _item.number.length))
  _save.enabled = (state);
  _site.enabled = (!state);
  _taxauth.enabled = (!state);
}

function itemPrice()
{
  if (_populating) // Don't overwrite data stored in the db being populated
    return;

  var params = new Object;
  params.item_id = _item.id();
  params.cust_id = _cust.id();
  params.qty     = _qty.text;

  try
  {
    if (_qty.text && _item.id() > 0)
    {
      var data = toolbox.executeDbQuery("simplesalesorder","itemprice",params);
      if (data.first())
        _unitPrice.setLocalValue(data.value("itemprice"));
      else if (data.lastError())
        throw data.lastError().text;
      else
        throw "Price not found.  Please see your administrator.";
    }
  }
  catch (e)
  {
    print(e);
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
}

function populate()
{
  _populating = true;
  _sale.select();

  populateItems();

  if (_saleitems.rowCount())
  {
    _saleitem.setCurrentIndex(0);
    _saleitems.selectRow(0);
  }

  _populating = false;
  handleButtons();
  _cust.enabled = false;
}

function populateCustomer()
{
  if (_populating)
    return;

  var params = new Object;
  params.cust_id = _cust.id();

  var data = toolbox.executeDbQuery("simplesalesorder","fetchcustomer", params);
  if(data.first())
  {
    _salesrep.setId(data.value("cust_salesrep_id"));
    _billAddrNo = data.value("billto_addr_number");
    _address.setNumber(data.value("shipto_addr_number"));
    _taxauth.setId(data.value("cust_taxauth"));
    _shipvia.text = (data.value("cust_shipvia"));
    _site.setId(data.value("cust_preferred_warehous_id"));
  }
  else
  {
    _salesrep.setId(-1);
    _billAddrNo = "";
    _address.setNumber("");
    _taxauth.setId(-1);
    _shipvia.text = "";
    _site.setId(-1);
  }
}

function populateItems()
{
  _saleitems.populate(_sale.currentIndex());
  _itemGroup.enabled = (_saleitems.rowCount());
  _linenumber = _saleitems.rowCount() + 1;
  recalcTotals();
}

function prepare()
{
  // Set header data
  var data = toolbox.executeDbQuery("simplesalesorder", "fetchsonumber");
  if(data.first())
    _number.text = data.value("number");

  // Associate sale items to new header
  populateItems();
  add();
  _add.enabled = false;
  _tab.setCurrentIndex(0);
  _cust.setFocus();
}

function recalcTotals()
{
  var params = new Object;
  var data;
  var ext = 0-0;
  var tax = 0-0;
  _subtotal.clear();
  _tax.clear();
  _total.clear();

  for (var i = 0 - 0; i < _saleitems.rowCount(); i++)
  {
    ext = _saleitems.value(i,_qtyCol) * 
          _saleitems.value(i,_priceCol);
    _extList[i] = ext;
    _subtotal.localValue = _subtotal.localValue + ext;

    params.taxauth_id = _taxauth.id();
    params.item_number = _saleitems.value(i,_itemCol);
    params.qty = _saleitems.value(i,_qtyCol) - 0;
    params.unit_price = _saleitems.value(i,_priceCol) - 0;
    data = toolbox.executeDbQuery("simplesalesorder","itemtax",params);
    if (data.first())
      tax = data.value("item_tax") - 0;
    else
      tax = 0 - 0;
    _taxList[i] = tax;
    _tax.localValue = _tax.localValue + tax;
  }
  _total.localValue = _subtotal.localValue + _tax.localValue;
}

function remove()
{
  var num = _saleitems.selectedValue(_linenumCol);
  var row = _saleitem.currentIndex();
  var idx = 0;

  _itemGroup.enabled = false;
  _saleitem.clear();
  _saleitems.removeSelected();

  // Renumber Rows
  for (row; row < _saleitems.rowCount(); row++)
  {
    if (!_saleitems.isRowHidden(row))
    {
      _saleitems.setValue(row,_linenumCol,num);
      num++;
    }
  }
  _linenumber = num;

  // Select a valid row, otherwise enable/disable appropriate controls
  if (!_saleitems.rowCountVisible())
  {
    _itemGroup.enabled = false;
    _add.enabled = true;
    _remove.enabled = false;
    handleButtons();
    return;
  }
  else
  {
    while (_saleitems.isRowHidden(idx))
      idx++;
    _populating = true;
    _saleitem.setCurrentIndex(idx);
    _saleitems.selectRow(idx);
    popualting = false;
  }
}

function rowSelected(row)
{
  var currentRow = _saleitem.currentIndex(row);
  if (row == currentRow)
    return;

  if (_itemGroup.enabled)
  {
    if (_item.id() == -1)
    {
      _saleitems.selectRow(currentRow);
      var msg = "You must select an item or remove the current line."
      toolbox.messageBox("critical", mywindow, mywindow.windowTitle, msg);
      return;
    }

    else if (_scheddate.date.length == 0)
    {
      _saleitems.selectRow(currentRow);
      var msg = "You must enter a valid scheduled date or remove the current line."
      toolbox.messageBox("critical", mywindow, mywindow.windowTitle, msg);
      return;
    }

    else if (_qty.text.length == 0)
    {
      _saleitems.selectRow(currentRow);
      var msg = "You must enter a valid quantity or remove the current line."
      toolbox.messageBox("critical", mywindow, mywindow.windowTitle, msg);
      return;
    }
  }
  else
    _itemGroup.enabled = true;

  _populating = true;
  _saleitem.setCurrentIndex(row);
  _populating = false;
}

function save()
{
  if (_item.id() == -1)
    remove();

  _populating = true;
  _address.check();

  try
  {
    toolbox.executeBegin();
    _sale.save();
    _saleitem.save();
    toolbox.executeCommit();
    if (_sale.mode == _newMode)
      prepare();
    else
      mydialog.accept();
  }
  catch (e)
  {
    toolbox.executeRollback();
    print(e);
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
  finally
  {
    _populating = false;
  }
}

function set(input)
{
  if ("mode" in input)
  {
    _sale.setMode(input.mode);

    if (input.mode == _newMode)
      prepare();
    else if (input.mode == _viewMode)
    {
       _save.hide();
       _cancel.text = "Close";
       _add.hide();
       _remove.hide(); 
       _itemGroup.hide();
    }

    // Always disabled
    _number.enabled 	= false;
  }

  if ("filter" in input) // Use to populate an existing record
  {
    _sale.setFilter(input.filter);
    populate();
  }

  return 0;
}

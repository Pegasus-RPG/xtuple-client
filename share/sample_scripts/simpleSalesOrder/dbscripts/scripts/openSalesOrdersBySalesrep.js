// Define Variables
var _all		= mywindow.findChild("_all");
var _close 		= mywindow.findChild("_close");
var _edit 		= mywindow.findChild("_edit");
var _print 		= mywindow.findChild("_print");
var _orders		= mywindow.findChild("_orders");
var _salesrep	= mywindow.findChild("_salesrep");
var _selected	= mywindow.findChild("_selected");
var _view  		= mywindow.findChild("_view");

// Set up columns
with (_orders)
{
  addColumn("Order#",               75, 1, true, "cohead_number");
  addColumn("Customer#",            75, 1, true, "cust_number");
  addColumn("Name",                 -1, 1, true, "cust_name");
  addColumn("P/O#", 		  75, 1, true, "cohead_custponumber");
  addColumn("Sched. Date",          75, 1, true, "scheddate");
}

// Make connections
_all["toggled(bool)"].connect(fillList);
_close.clicked.connect(mywindow, "close");
_salesrep["newID(int)"].connect(fillList);

// Execute query
fillList();

function fillList()
{
  try
  {
    params = new Object();
    if (_selected.checked)
      params.salesrep_id = _salesrep.id();

    data = toolbox.executeDbQuery("salesOrdersCustom","detail",params);
    _orders.populate(data);
  }
  catch(e)
  {
    print(e);
  }
}

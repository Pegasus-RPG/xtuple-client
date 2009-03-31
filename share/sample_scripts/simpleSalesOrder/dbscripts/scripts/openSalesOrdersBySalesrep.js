// Define Variables
var _all	= mywindow.findChild("_all");
var _close 	= mywindow.findChild("_close");
var _edit 	= mywindow.findChild("_edit");
var _new	= mywindow.findChild("_new");
var _print 	= mywindow.findChild("_print");
var _orders	= mywindow.findChild("_orders");
var _salesrep	= mywindow.findChild("_salesrep");
var _selected	= mywindow.findChild("_selected");
var _view  	= mywindow.findChild("_view");

var _newMode 	= 0;
var _editMode 	= 1;
var _viewMode	= 2;

// Set up columns
with (_orders)
{
  addColumn("Order#",		75, 1, true, "cohead_number");
  addColumn("Customer#",	75, 1, true, "cust_number");
  addColumn("Name",		-1, 1, true, "cust_name");
  addColumn("P/O#",		75, 1, true, "cohead_custponumber");
  addColumn("Sched. Date",	75, 1, true, "scheddate");
}

// Make connections
_close.clicked.connect(mywindow, "close");

_all["toggled(bool)"].connect(fillList);
_salesrep["newID(int)"].connect(fillList);

_new.clicked.connect(orderNew);
_edit.clicked.connect(orderEdit);
_view.clicked.connect(orderView);

// Populate
fillList();

function fillList()
{
  try
  {
    params = new Object();
    if (_selected.checked)
      params.salesrep_id = _salesrep.id();

    data = toolbox.executeDbQuery("salesorderscustom","detail",params);
    _orders.populate(data);
  }
  catch(e)
  {
    print(e);
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
}

function orderEdit()
{
  orderOpen(_editMode,_orders.id());
}

function orderNew()
{
  orderOpen(_newMode,0);
}

function orderOpen(mode,number)
{
  try
  {
    var childwnd = toolbox.openWindow("simpleSalesOrder", mywindow, 0, 1);
    var params   = new Object;

    params.mode   = mode;
    if (mode) // Edit or View
      params.filter = "order_number='" + number + "'";

    var tmp = toolbox.lastWindow().set(params);
    var execval = childwnd.exec();
    if (execval)
      fillList();
  }
  catch(e)
  {
    print(e);
    toolbox.messageBox("critical", mywindow, mywindow.windowTitle, e);
  }
}

function orderView()
{
  orderOpen(_viewMode,_orders.id());
}

/* This is a sample script to show how to open a core application window from a
   scripted window.

   This sample has a window that lists the itemsites for a selected site.
   A pushbutton allows the user to view the item record for the selected
   itemsite.
 */

var _itemsite = mywindow.findChild("_itemsite");
var _site     = mywindow.findChild("_site");
var _view     = mywindow.findChild("_view");

_itemsite.addColumn(qsTr("Item Number"),     -1, Qt.AlignLeft, true, "item_number");
_itemsite.addColumn(qsTr("Loc. Controlled"), -1, Qt.AlignRight,true, "itemsite_loccntrl");
_itemsite.addColumn(qsTr("Control Method"),  -1, Qt.AlignRight,true, "itemsite_controlmethod");

function sFillList()
{
  var params = new Object;
  params.site = _site.id();

  var qry = toolbox.executeQuery('SELECT item_id, item_number, '
                               + '       itemsite_loccntrl, itemsite_controlmethod '
                               + 'FROM itemsite '
                               + '     JOIN item ON (itemsite_item_id=item_id) '
                               + 'WHERE itemsite_warehous_id=<? value("site") ?> '
                               + 'ORDER BY item_number;',
                               params);
  _itemsite.populate(qry);
}

function sViewItem()
{
  var params = new Object();
  params.item_id = _itemsite.id();
  params.mode    = "view";

  // here's the interesting line:    
  var newdlg = toolbox.openWindow("item", 0, 0, 0);
  /*                               ^^^^   ^  ^  ^- window flag
                                    |     |  +- modality
                                    |     +- parent
                                    +- open an instance of this class
   The "item" class is defined as a QWidget (look at item.ui). If we set the
   parent to mywindow then the item window will appear as a widget within
   mywindow, which we don't want. Therefore set the parent to 0, which means
   "no parent". If item.ui were a QMainWindow or QDialog then we could
   set the parent to mywindow and everything would work out OK. In fact,
   if it were a QDialog then we could set the modality to Qt.ApplicationModal
   to block all other input or Qt.WindowModal to block input to mywindow
   until the Item window were closed.
  */
 
  newdlg.set(params);
}

_site["newID(int)"].connect(sFillList);
_view.clicked.connect(sViewItem);

sFillList();

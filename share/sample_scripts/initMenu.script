// when the application is first started any script with the name
// initMenu is called. You can do a couple things here including
// disabling Action items and calling screens to be opened at start

// find and disable a menu action item
mainwindow.findChild("pd.enterNewItem").enabled=false;

// find and trigger a menu action
// in this case we are also checking to make
// sure it was found first
var lis = mainwindow.findChild("im.listItemSites");
if(lis != null)
{
  lis.trigger();
}

// find one of the module objects and call a slot directly
mainwindow.findChild("crmModule").sOpportunityTypes();

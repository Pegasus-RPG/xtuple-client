function randomClicked()
{
  toolbox.messageBox("information", mywindow, "Some Happened", "The Random button was clicked.");
}

var btnDelete = mywindow.findChild("_delete");
var layout = toolbox.widgetGetLayout(btnDelete);
var newbutton = toolbox.createWidget("QPushButton", mywindow, "_scriptButton");
newbutton.text="Random";
toolbox.layoutBoxInsertWidget(layout, -1, newbutton, 0, 0);
newbutton.clicked.connect(randomClicked);

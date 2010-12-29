var _input = mywindow.findChild("_input");
var _output = mywindow.findChild("_output");
var _proc;
var _program = mywindow.findChild("_program");
var _restart = mywindow.findChild("_restart");
var _var = mywindow.findChild("_var");

function sRestart()
{
  try {
    _input.clear();
    _output.clear();
    _proc = new QProcess(mywindow);
    var args = _program.text.split(/\s/);
    var cmd  = args.shift();
    if (_var.plainText.length)
    {
      var env = QProcessEnvironment.systemEnvironment();
      var add = _var.plainText.split(/\s+/);
      for (var i = 0; i < add.length; i++)
      {
        var one = add[i].split("=");
        env[one[0]] = one[1];
      }
      _proc.setProcessEnvironment(env);
    }
    _proc.start(cmd, args);

    _input.returnPressed.connect(sWriteToProc);
    _proc.readyRead.connect(sReadFromProc);
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sRestart exception @ " + e.lineNumber + ": " + e.message);
  }
}

function sReadFromProc()
{
  try {
    _input.clear();
    var stdout  = String(_proc.readAllStandardOutput());
    var stderr  = String(_proc.readAllStandardError());
    _output.plainText += String(stdout);
    if (stderr.length)
      QMessageBox.warning(mywindow, "Error", stderr);
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sRestart exception @ " + e.lineNumber + ": " + e.message);
  }
}

function sWriteToProc()
{
  try {
    _output.plainText = _input.text + '\n===========================================\n';
    _proc.write(QByteArray(_input.text));
    _proc.write(QByteArray('\n'));
  }
  catch (e)
  {
    QMessageBox.critical(mywindow, "Processing Error",
                         "sWriteToProc exception @ " + e.lineNumber + ": " + e.message);
  }
}

_program.returnPressed.connect(sRestart);
_restart.clicked.connect(sRestart);
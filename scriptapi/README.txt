This directory holds the interface between various classes and the
scripting engine available to the xTuple ERP GUI client. One of the
ways to expose a class to application scripts is to create a prototype
class.

If the class is part of the xTuple ERP sources then you don't need
a prototype class. Just add Q_INVOKABLE to the public methods that
you need to use in the appropriate header file and recompile. The
following method is for classes you /don't/ have control over, such
as Qt classes themselves.

Short Description:
$ edit newproto.h
$ awk -f h2cpp.awk newproto.h > newproto.cpp
$ edit newproto.cpp
$ edit scriptapi.pro      # add newproto.h and newproto.cpp
$ edit setupscriptapi.cpp # add #include "newproto.h" and setupNewProto(engine)

Long Description:
To add a prototype for the class named 'new', you have to create header
and implementation files, add these two files to the scriptapi project,
and register the new prototype class with the scripting engine.

$ edit newproto.h
  Create the header file. It's probably best to start with a copy
  of an existing small prototype .h. Change the #ifndef, #define,
  #includes, Q_DECLARE_METATYPE, setup* and construct* function
  names, and the class name and constructor at the top to refer to
  your new class.  Replace the Q_INVOKABLE methods of the copied
  file with signatures for the public methods in the class you want
  to expose. One way to get a complete list is to copy the class
  summary from the documentation page, paste it into your editor,
  and add Q_INVOKABLE.  You will probably have to replace symbolic
  constants with integer literals and references to enums with int.

$ awk -f h2cpp.awk newproto.h > newproto.cpp
  This awk script takes the edited header file and fills in the
  boilerplate code necessary to make most methods work. The awk
  script is really dumb, so:

$ edit newproto.cpp
  Edit the resulting C++ implementation by hand. You'll have to do
  things like move * from after NewProto:: to before it 
  remove data type names from function calls, remove
  const from the end of function call lines, fill in the occasional
  variable names where method parameters in the .h were declared
  with a type but no name.  Methods that return pointers will need
  their default return values changed to 0 or NULL.
  
  For example, when creating qtabwidgetproto.cpp, the following
  code was generated:
    QWidget QTabWidgetProto::*cornerWidget(int corner) const
    {
      QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
      if (item)
        return item->*cornerWidget(int corner) const;
      return QWidget();
    }

  This had to be changed to:
    QWidget *QTabWidgetProto::cornerWidget(int corner) const
    {
      QTabWidget *item = qscriptvalue_cast<QTabWidget*>(thisObject());
      if (item)
        return item->cornerWidget(corner);
      return 0;
    }

  If there's a reasonable QString representation of an object,
  modify the default implementation of the toString() method
  to reflect this and add toString() to the .h file. Otherwise
  remove or comment out the default toString().

$ edit scriptapi.pro      # add newproto.h and newproto.cpp
$ edit setupscriptapi.cpp # add #include "newproto.h" and setupNewProto(engine)

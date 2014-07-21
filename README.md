# The xTuple ERP Desktop Client Application

[![Build Status for xtuple/qt-client](https://travis-ci.org/xtuple/qt-client.png)](https://travis-ci.org/xtuple/qt-client)
[This repository](http://github.com/xtuple/qt-client) contains the source code
for the xTuple ERP desktop client. xTuple ERP is a collection of programs
designed to help you run your business.

## Getting Help

To learn more about:

* xTuple the company and our products, see http://www.xtuple.org and http://www.xtuple.com
* using the desktop client, see the [xTuple ERP Reference Guide (for users)](http://www.xtuple.org/sites/default/files/refguide/current/index.html)
* desktop client development, see our [desktop client wiki](http://github.com/xtuple/qt-client/wiki) (click on the wiki link on the right)
* our mobile-web client, see our [mobile-web client wiki](http://github.com/xtuple/xtuple/wiki)

## Development Quickstart

We're working on updating our [Development Environment Setup docs](http://www.xtuple.org/sites/default/files/dev/370/devGuide370/ch01.html) and building a Linux virtual machine you can use for development. Until that's done, here's the summary of what you need to do:

* Install Postgres 9.1, including libraries and header files. Start [here](http://www.postgresql.org/download/) and use the *File Browser*.
* Download the latest PostBooks _demo_ database from http://sourceforge.net/projects/postbooks/files/03%20PostBooks-databases/ and `pg_restore` it so you have some data to test with.
* Install Qt 4.8. For a development environment, using a Qt installer will probably suffice. If you plan to work with credit cards and use a Mac, you'll have to build Qt from source and [patch the SSL sources](https://bugreports.qt-project.org/browse/QTBUG-15344).
* Get the source code. See our [Git Usage](https://github.com/xtuple/xtuple/wiki/Basic-Git-Usage) guidelines for more information. The desktop client requires OpenRPT and CSVImp, which are included as git submodules, so don't forget
```
$ git submodule update --init --recursive
```
* Build:
```
$ cd openrpt && qmake && make
$ cd ../csvimp && qmake && make
$ cd .. && qmake && make
```

*Warnings*:

* This environment may not be suitable for building distributable versions of the client.
* If you open Qt Designer to view or edit a `.ui` user interface file, check the widget palette _before you do anything else_. If there is no section for xTuple widgets, *quit immediately* without saving any changes. Otherwise you risk losing important information about the user interface definition.

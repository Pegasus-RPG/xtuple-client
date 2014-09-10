# The xTuple ERP Desktop Client Application

[![Build Status for xtuple/qt-client](https://travis-ci.org/xtuple/qt-client.png)](https://travis-ci.org/xtuple/qt-client)
[This repository](http://github.com/xtuple/qt-client) contains the source code
for the xTuple ERP desktop client. xTuple ERP is a collection of programs
designed to help you run your business.

_Note to haxTuple participants_: Please read [this](https://github.com/xtuple/xtuple/wiki/haxTuple-2014) before you start and remember to use `haxtuple` as the base of your branches and as the destination of pull requests.

## Getting Help

To learn more about:

* xTuple the company and our products, see http://www.xtuple.org and http://www.xtuple.com
* using the desktop client, see the [xTuple ERP Reference Guide (for users)](http://www.xtuple.org/sites/default/files/refguide/current/index.html)
* desktop client development, see our [desktop client wiki](http://github.com/xtuple/qt-client/wiki) (click on the wiki link on the right)
* our mobile-web client, see our [mobile-web client wiki](http://github.com/xtuple/xtuple/wiki)

## Development Quickstart

We built a [Linux virtual machine](https://github.com/xtuple/xtuple-vagrant/blob/master/xtuple-desktop/README.md) you can use for desktop client development. If you want to set things up on your own, here's how:

* Install Postgres 9.1, including libraries and header files. We strongly suggest that you build from source.
  * Start [here](http://www.postgresql.org/download/) and use the *File Browser* to get a source bundle for 9.1.x.
  * Follow the instructions for [installing from source code](http://www.postgresql.org/docs/9.1/static/installation.html).
  * We recommend the following configuration options:

```Shell
        $ ./configure --prefix=/usr/local/pgsql/09.1.14 --with-perl --with-openssl --with-readline --with-libxml --with-libxslt
```

* Get a Postgres server instance up and running
* [Download](http://sourceforge.net/projects/postbooks/files/03%20PostBooks-databases/) the latest PostBooks _demo_ database and `pg_restore` it so you have some data to test with.
* Install Qt 4.8 (not Qt 5!). We strongly recommend that you build from source. If you plan to work with credit cards and use a Mac, you **must** build Qt from source and [patch the SSL sources](https://bugreports.qt-project.org/browse/QTBUG-15344).
  * [Download Qt](http://qt-project.org/downloads). If you don't see version 4.8 listed, click on the **Show Downloads** button and scroll down.
  * [Install Qt](http://qt-project.org/doc/qt-4.8/installation.html)
  * We recommend the following configuration options (the `-I` and `-L` options must match the `--prefix` option used when configuring Postgres):

```Shell
        $ ./configure -qt-zlib -qt-libtiff -qt-libpng -qt-libmng -qt-libjpeg -plugin-sql-psql -plugin-sql-odbc -plugin-sql-sqlite -I /usr/local/pgsql/09.1.14 -L /usr/local/pgsql/09.1.14 -lkrb5 -webkit -fontconfig -continue -nomake examples -nomake demos -prefix $HOME/Qt/Qt4.8.6
```

* Get the source code for the desktop client from xtuple's qt-client repository. See our [Git Usage](https://github.com/xtuple/xtuple/wiki/Basic-Git-Usage) guidelines for more information. The desktop client requires OpenRPT and CSVImp, which are included as git submodules, so don't forget
```Shell
        $ git submodule update --init --recursive
```
* Build:
```Shell
        $ cd openrpt && qmake && make
        $ cd ../csvimp && qmake && make
        $ cd .. && qmake && make
```

**Warning**:
If you open Qt Designer to view or edit a `.ui` user interface file, check the widget palette _before you do anything else_. If there is no section for xTuple widgets, _quit immediately_ without saving any changes. Otherwise you risk losing important information about the user interface definition.

*Note*:

On some Linux distributions you can build the desktop client using OpenRPT
and CSVImp development packages instead of compiling in the `openrpt` and
`csvimp` directories. In these cases, set the following environment variables:
- `OPENRPT_HEADERS` names the directory where the OpenRPT header files are
  installed
- `OPENRPT_LIBDIR` names the directory where the OpenRPT libraries are
  installed
- `OPENRPT_IMAGE_DIR` names the directory where the OpenRPT images are
  installed
- `CSVIMP_HEADERS` names the directory where the CSVImp header files are
  installed
- `OPENRPT_LIBDIR` names the directory where the OpenRPT libraries are
  installed

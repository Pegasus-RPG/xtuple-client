Dependencies:
- You must have Doxygen 1.6 or later.
- You must have the environment variable QTDIR set to the Qt installed directory
    and exported.
- You must have the Qt docs installed in QTDIR/doc/html.

Steps to run:

  1 $ svn co the-right-xtuple-code-url
  2 $ cd .../xtuple[/tag/tag_name]
  3 $ qmake ; make
  4 $ edit utilities/doxygen/Doxyfile.public
      # set PROJECT_NUMBER to match version.cpp
      # fix the URL in TAGFILES to match the Qt version we're using
  5 $ doxygen utilities/doxygen/Doxyfile.public

Then copy the resulting html directory to the web server to publish the docs.

Note: The build in step 3 is required to get the list of widgets
      from the .ui files.

Futures:

Develop guidelines for which Doxygen tags we want to use and what the style
should be like:
* How deep should we go into how the internals work?
* Should we restrict the docs to simply what the class and its components do?
* How rich should the See Also tagging be?

Look at turning on HAVE_DOT and using the various graph options.
Some of particular are listed below. However, even if we don't use
dot, we should look at turning on CLASS_DIAGRAMS. The inheritance
hierarchy is pretty flat right now but should get deeper over time.
If so, CLASS_DIAGRAMS would be helpful. It would be really nice if
we could figure out how to build diagrams of JavaScript object
inheritance if we ever start writing JS code that way.

Look at http://ww.mcternan.me.uk/mscgen and Doxygen's MSCGEN_PATH
option.  We could use Doxygen calling mscgen to document the default
signal/slot connections so scripters know what connections they
can/should break to accomplish a desired goal.

Look at turning on COLLABORATION_GRAPH. This would be more useful
for people working in the core. One problem with it, though, is
that the diagrams are really hairy - we have a lot of interdependencies.
On the other hand, this would provide good data on ways we could
simplify the code.

Look at turning off EXTRACT_ALL once we've documented everything.

Look into documenting stored procedures. We would have to write a
parser for PL/PGSQL and a filter to convert -- comments to C- or
C++-style comments. The filter could be run on all source files
with a .sql suffix.  We might be able to do the same with COMMENTS
on tables and their columns and/or views and their columns. Maybe
we should create COMMENTS on functions, too, so everyone gets them
in the db.

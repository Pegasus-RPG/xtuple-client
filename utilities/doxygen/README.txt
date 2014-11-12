Dependencies:
- You must have Doxygen 1.6 or later
- You must have the Qt docs installed in QTDIR/doc/html

Steps to run:

  1 $ git clone https://github.com/xtuple/qt-client
  2 $ git checkout the_proper_build_branch_or_tag
  3 $ cd .../qt-client
  4 $ qmake ; make
  5 $ edit utilities/doxygen/Doxyfile.public
      # set PROJECT_NUMBER to match version.cpp
      # fix the URL in TAGFILES to match the Qt version we're using
  6 $ doxygen utilities/doxygen/Doxyfile.public

Then copy the resulting html directory to the web server to publish the docs.

Note: The build in step 4 is required to get the list of widgets
      from the .ui files.

First pass at usage guidelines; comments welcome.
- document in the .cpp file instead of the .h as much as possible
  (e.g. describe methods in the .cpp but enums in the .h)
- if the only items in the .cpp are class members, start the .cpp with @class;
  if there are globals or statics in the .cpp, use @addtogroup and wrap them
  in @{ and @}
- use /** to start doxygen comments, not /*!
- use @ style directives, not backslashes
- use @brief instead of relying on doxygen to guess for you
- put % in front of words that you're using casually that happen to be
  names of classes/functions/... so they don't get converted to links
- tag order seems to be important. put the tags you use in the following order:
    /** @brief

        long description

        @dontinclude samplefile.js
          bits of samplefile.js using @skip, @until, ...

        more long description

        @param

        @return

        @see
        @see samplefile.js
     */
    the method/function definition
    /** @example */


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

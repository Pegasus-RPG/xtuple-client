Development for the Qt (C++) xTuple App
========

Download and Install Postgres 9.1 and the xTuple Database
------

You'll need to install Postgres 9.1.  There are several different ways you can do this.  Go to www.postgresql.org to get started.

After you've successfully installed Postgres and you're able to get a local server running, you'll need to install the xTuple demo database so that you'll have a database to log in to and some data to work with when testing your code.  Select the most recent version from the link below:

http://sourceforge.net/projects/postbooks/files/03%20PostBooks-databases/

And you'll want to download the database entitled postbooks_demo-(version).backup

The "demo" databases are set up with a fictional company that seems to have been running for a while, so whatever part of the application you're working on, there will surely be some data for you to test against.  Use the pg_restore terminal command to install the database .backup onto your Postgres server.

Download and Install Qt 4.8
------

There are several ways to do this as well.  Visit www.qt-project.org or www.developer.nokia.com to get started.


Set up your Forks
------

Now that we've installed all the necessary software to develop for xTuple, we'll need the xTuple source code, along with some other code needed to compile the app.

From github.com/xtuple, you'll want to "fork" the openrpt, csvimp, xtlib, and qt-client repos into your git profile.  Towards the top right of the page of each repository, there's a fork button.  This copies everything over to your git profile, so that you can safely manipulate the code without touching anything on the xTuple git.

Clone the Code
------

After setting up our forks, we'll use terminal to clone our freshly forked repos onto our machines.  When you call git clone from a terminal window, you pull the repository from github to your local machine.  By default, it'll be created in a new directory named after the repo you're cloning.

    git clone https://github.com/YOUR_GIT_NAME/qt-client.git

openrpt, csvimp and xtlib are submodules of the qt-client repository.  When you clone qt-client, you get these repos too, and you'll see them in your file system as subdirectories of qt-client.  They're special though; any manipulation or updating of these directories will be ignored by the qt-client repo - for example, changed openrpt code will not show up in a "git diff" called from the qt-client directory.  If you look at the directory structure through Github, you'll notice that submodules directories are green.

Branching off Master
------

When you first clone a repo, you start off in the "master" branch.  We want this branch to be clean, so we're not going to be writing code in it.  Instead, we'll create branches based off master, then work from those.  When you make changes in a branch, those changes are bound to those branch.  When you switch to another branch, you'll ditch the changes in the branch you were working in, and load up the changes you've made on the branch you're switching to.  Branching allows us to work on a number of different bugs and features at a time - a task that was an enormous hassle with SVN.

To create a new branch, and to check it out, call

    git checkout -b newBranchName

"checkout" is the command used to switch between branches.  Use -b when you want to create a new branch.  Use "git branch" to list your branches for the current repo.

Notice that, when you create a new branch, it's based off of the current branch you're in.  So when you want to start working on a new issue that you haven't touched yet, you'll want to switch to your master branch and make sure it's up to date with xTuple's master before creating a new branch to work with.  It's important that each branch approaches one issue at a time.  See the section entitled "Keeping up to date with xTuple's Master" on how to keep your master up to date.

After you've made some changes and you're ready to stick them in the master xTuple source, you'll want to add, commit, push, and issue a pull request.

Adding, Committing and Pushing Code
------

Now you're ready to commit.  Make use of the "git status" and "git diff" tools to make sure you're satisfied with the changes you've made.  "git status" will tell you that there are changes, but nothing is staged for commit.  To stage files for commit, you add them with "git add".  Then, you commit them with "git commit" (please use -m and leave a message detailing the commit).

At this point, everything's still local.  Your github page on the internet is completely unaware of newBranchName and the changes you've made.  To get this stuff up online, use: 

    git push origin branchName

Origin refers to the repository that you cloned from.  After pushing, you'll see on your github page for that repo that a new branch has been created.  You can use Git's diff tools to see the files and code changed against your master branch.

Pull Requests
------

So, you've got your new code on YOUR github, but not xTuple's.  Time to issue a pull request.  Click the pull request button.  The left side is the side we want to merge into (in this case, xTuple's master branch) and the right side is the side where the new code is coming from (your github's newBranch).  It's always a good idea to double check everything here using Github's diff tools - make sure your code looks good and make sure you're merging from the right branch into xTuple's master!

Issue the pull request, but don't merge it!  Someone else will look over the request using the diff tools and merge it into xTuple's master branch for you.  Set the incident you fixed to Resolved/Open, and whoever does the merge and tests it will close it.

Keeping up to date with xTuple's Master
------

Since everyone is pushing their code and pulling requests on xTuple code all the time, your local code will become dated pretty quickly.  We need to set up an easy way to keep our code up to date with xTuple's master on github.  To do this, we set up a remote.

    git remote add QTCLIENT git://github.com/xtuple/qt-client.git

Now we have two remotes: origin (you) and QTCLIENT (xTuple).  To get QTCLIENT/master code into our local masters, do the following:

    git checkout master

    git fetch QTCLIENT

    git merge QTCLIENT/master

    git submodule update --recursive

    git push origin master

What this does is 1) checks out our local master 2) fetches QTCLIENT things 3) merges QTCLIENT's master into our local master 4) updates the submodules (in this case, openrpt, csvimp and xtlib) and 5) pushes our now up-to-date local master to our github's internet master.

I recommend sticking those 5 commands in a shell script and running it a couple of times throughout the day to stay up to date.

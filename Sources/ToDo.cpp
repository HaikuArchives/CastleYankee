/* ToDo.cpp - for Castle Yankee

BY FIRST RELEASE
=================

Don't trigger class rebuild on window move/resize.
Pref for auto MW build.

MAYBE FOR FIRST RELEASE
========================

Template for auto-block.
Specify generated sources directory.
Auto backup.
Open MW project.


AFTER FIRST RELEASE
====================

Undo.
	- Q&D:  UndoAction has deleted and added text parts, taken from
	  DeleteText and InsertText hooks.  Paste groups both parts together,
	  as does drag.  Key-down appends insertion to previous UndoAction if
	  insert point is at end of that action, starts new action otherwise.
	  (DeleteText in key-down automatically creates new action.)  But how
	  to put multiple backspaces or forward-deletes together?
	- Did undo using BTextView's facilities (new in R3).
Section dividers.
	- Collapse/expand sections.
Inline handling
	- Have Method::SetCode() set "isInline".
	- Codegen.
Pure virtual method handling.
CodeView movement/selection by arrow keys
	Only Control key left
Save selection with code
	Done for Methods.
Connection to CodeWarrior
FTP
Search/replace
Custom font for CodeView
Syntax hiliting
	Make it good.
Clean up handling of switching selections in method/variable lists
	- When the methods list is active and you select an item (other
	  than the one already selected) in the vars list,
	  ClassWind::SetViewedCode() gets called twice.
	- When the vars list is active and you create a new method,
	  ClassWind::SetViewedCode() gets called three times.
Auto-typing
	- B_<something> automatically uppercase.
	- (Later:) Auto-type class names.
Templates for class codegen.
Templates for new methods, vars, classnames.
Template classes for BView subclasses, etc.
Balance parens/braces/brackets while typing.
Make sure file flushes on EZPObjectFile flush.  Add Flush() method to
	EZPBasicFile to support this.
Extend cheap notes hack to allow methods that are just one big comment.
	- Squish names.
	- Codegen autogenerates "void".
Make a class to handle file-creation.  Methods like GetFile(string name).
	- Have one subclass doing files, one doing FTPs.
Mechanism whereby you can add CY project files to your CW project, with
	appropriate CW project settings, so that during a build, CY builds
	the class C++ files.
Don't auto-block if the next line is already indented.
Live update when renaming a class, method, or variable.
Don't allow reopening of already open file!

BLUE SKY PIPE DREAMS
=====================

Log.
Expand to handle arbitrary text-container-hierarchy applications (see
	example project).  Allow some to have nice-looking (proportional) text.

*/

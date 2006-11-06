unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= apm_edit_about.ui apm_edit.ui runlog.ui 
IMAGES	= images/editcopy.png images/editcut.png images/editpaste.png images/filenew.png images/fileopen.png images/filesave.png images/module.png images/module_default.png images/module_missing.png images/print.png images/redo.png images/searchfind.png images/undo.png images/whatsthis.png 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= apm-edit.db
LANGUAGE	= C++

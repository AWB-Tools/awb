TEMPLATE	= app
LANGUAGE	= C++

CONFIG	+= qt warn_on release

FORMS	= apm_edit_about.ui \
	apm_edit.ui \
	runlog.ui \
	apm_edit_properties.ui

IMAGES	= images/editcopy.png \
	images/editcut.png \
	images/editpaste.png \
	images/filenew.png \
	images/fileopen.png \
	images/filesave.png \
	images/module.png \
	images/module_default.png \
	images/module_missing.png \
	images/print.png \
	images/redo.png \
	images/searchfind.png \
	images/undo.png \
	images/whatsthis.png

unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}

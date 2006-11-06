unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= awb_about.ui awb_runlog.ui awb_dialog.ui 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= awb.db
LANGUAGE	= C++

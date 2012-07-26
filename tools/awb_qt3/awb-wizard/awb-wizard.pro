unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= awb_wizard.ui awb_wizard_about.ui 
IMAGES	= images/filenew.png images/filesave.png images/whatsthis.png images/filebuild.png images/fileopen.png 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= awb-wizard.db
LANGUAGE	= C++

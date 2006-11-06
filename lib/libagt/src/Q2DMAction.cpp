// ==================================================
//Copyright (C) 2003-2006 Intel Corporation
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either version 2
//of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file  Q2DMAction.cpp
  * @brief Derives QAction to get back menu idx numbers 
  */

#include "Q2DMAction.h"

Q2DMAction::Q2DMAction ( QObject * parent, const char * name , bool toggle) : 
			QAction (parent,name,toggle) {menuidx=-1;mymenu=NULL;}
			
Q2DMAction::Q2DMAction ( const QString & text, const QIconSet & icon, 
			const QString & menuText, QKeySequence accel, QObject * parent, 
			const char * name, bool toggle) : 
			QAction (text,icon,menuText,accel,parent,name,toggle) {menuidx=-1;mymenu=NULL;}
			
Q2DMAction::Q2DMAction ( const QString & text, const QString & menuText, 
			QKeySequence accel, QObject * parent, const char * name, bool toggle) :
			QAction (text,menuText,accel,parent,name,toggle) {menuidx=-1;mymenu=NULL;}
			
Q2DMAction::~Q2DMAction () {}

int
Q2DMAction::getMenuId()
{
	if (mymenu!=NULL)
	{
		return mymenu->idAt(menuidx);
	}
	else
	{
		return -1;
	}
}

void 
Q2DMAction::addedTo ( int index, QPopupMenu * menu )
{
	menuidx = index;
	mymenu=menu;
}
	

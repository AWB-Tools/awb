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
  * @file LogMgr.cpp
  */

#include "asim/LogMgr.h"

/**
 * The instance is NULL at the beginning.
 */
LogMgr* LogMgr::_myInstance=NULL;

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
LogMgr* 
LogMgr::getInstance()
{
    return getInstance("logfile.txt");
}

/**
 * Returns the instance of the class. The first time this function
 * is called is when the instance is created. The other times just
 * returns the pointer.
 *
 * @return the instance of the class.
 */
LogMgr* 
LogMgr::getInstance(QString filename)
{
    if (_myInstance==NULL)
    {
        _myInstance = new LogMgr(filename);
        Q_ASSERT(_myInstance!=NULL);
    }
    return _myInstance;
}

/**
 * Destroys the unique instance of the class. The instance is
 * destroyed if it was previously created.
 *
 * @return void.
 */
void
LogMgr::destroy()
{
    if (_myInstance!=NULL)
    {
        delete _myInstance;
    }
}

/**
 * Creator of this class. Creates the log file filename.
 *
 * @return new object.
 */
LogMgr::LogMgr(QString filename)
{
    fileName= filename;
    fileObject = new QFile(filename);
    Q_ASSERT(fileObject!=NULL);
    ok = fileObject->open(IO_WriteOnly|IO_Append);
    if (!ok)
    {
        qWarning("Error trying to create log file: %s",filename.latin1());
    }
    else
    {
        fileObject->putch (13);
        fileObject->flush();
    }
    lastTimestamp = QTime::currentTime();
}

/**
 * Destructor of this class. Closes the file.
 *
 * @return destroys the object.
 */
LogMgr::~LogMgr()
{
    if (fileObject!=NULL)
    {
        fileObject->close();
        delete fileObject;
    }

    //if (stream!=NULL) { delete stream; }

}

/**
 * Chages the name where the log is written.
 *
 * @return correct change.
 */
bool
LogMgr::changeFileName(QString newfilename)
{
    if (newfilename==fileName) { return true; }
    ok=false;
    fileName= newfilename;
    fileObject->close();
    delete fileObject;
    fileObject = new QFile(fileName);
    ok = fileObject->open(IO_WriteOnly|IO_Append);
    lastTimestamp = QTime::currentTime();
    if (ok)
    {
        fileObject->putch (13);
        fileObject->flush();
    }

    return (ok);
}

/**
 * Clears the log file.
 *
 * @return void.
 */
void
LogMgr::clearLogFile()
{
    if (!ok) { return; }

    fileObject->close();
    fileObject->remove();
    ok = fileObject->open(IO_WriteOnly|IO_Append);
    if (!ok)
    {
        qWarning("Error trying to create log file: %s",fileName.latin1());
    }
    lastTimestamp = QTime::currentTime();
}

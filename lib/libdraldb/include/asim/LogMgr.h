// ==================================================
// Copyright (C) 2003-2006 Intel Corporation
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

/**
  * @file LogMgr.h
  */

#ifndef _DRALDB_LOGMGR_H
#define _DRALDB_LOGMGR_H

// QT Library
#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdatetime.h>

/**
  * @brief
  * This class conains the log of the database.
  *
  * @description
  * This class is used to store a global log of an application that
  * uses this database. Typical file access methods are defined to
  * allow an ease use of this class.
  *
  * @author Federico Ardanaz
  * @date started at 2002-07-15
  * @version 0.1
  */
class LogMgr
{
    public:
        static LogMgr* getInstance();
        static LogMgr* getInstance(QString filename);
        static void destroy();

    public:
        inline void addLog(QString msg);
        inline void flush();
        void clearLogFile();
        inline QTime getLastTimestamp();
        bool changeFileName(QString newfilename);

    protected:
        LogMgr(QString filename);
        ~LogMgr();

    private:
        QString  fileName; // Name of the log file.
        QFile*   fileObject; // Pointer to the file descriptor.
        bool ok; // 
        QTime lastTimestamp; // Last time a comment was inserted.

    private:
        static LogMgr* _myInstance;
};

/**
 * Returns the value of the lastTimestamp variable.
 *
 * @return last time stamp.
 */
QTime
LogMgr::getLastTimestamp()
{
    return lastTimestamp;
}

/**
 * Flushes the content of the log file.
 *
 * @return void.
 */
void
LogMgr::flush()
{
    if (!ok)
    {
        return;
    }
    fileObject->flush();
}

/**
 * Adds a new string to the log file.
 *
 * @return void.
 */
void
LogMgr::addLog(QString msg)
{
    if (!ok){
        return;
    }

    QTextStream stream(fileObject);
    stream.setEncoding(QTextStream::Latin1);

    QTime now = QTime::currentTime();
    QString nowstr = now.toString ("hh:mm:ss");
    stream << "[" << nowstr << "] " << msg << "\n";
    lastTimestamp = QTime::currentTime();
}

#endif

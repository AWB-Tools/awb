/*
 *Copyright (C) 2003-2006 Intel Corporation
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/****************************************************************************
** Form interface generated from reading ui file 'snapDialog.ui'
**
** Created: Wed May 29 22:09:11 2002
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef FORM1_H
#define FORM1_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;

class Form1 : public QDialog
{ 
    Q_OBJECT

public:
    Form1( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Form1();

    QGroupBox* GroupBox1;
    QLabel* TextLabel3;
    QLabel* TextLabel1;
    QCheckBox* CheckBoxSnap;
    QLineEdit* LineEditDst;
    QSlider* SliderDst;
    QPushButton* PushButton1;
    QPushButton* PushButton2;


public slots:
    virtual void Slider1_sliderMoved(int);

protected:
    QGridLayout* Form1Layout;
    QVBoxLayout* Layout14;
    QGridLayout* GroupBox1Layout;
    QVBoxLayout* Layout13;
    QHBoxLayout* Layout12;
    QVBoxLayout* Layout8;
    QVBoxLayout* Layout11;
    QHBoxLayout* Layout10;
    QHBoxLayout* Layout4;
};

#endif // FORM1_H

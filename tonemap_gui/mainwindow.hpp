/*************
**
** Project:      Imaginable :: tonemap (GUI)
** File info:    $Id$
** Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
** License:      GPLv3
**
**  This file is part of Imaginable.
**
**  Imaginable is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Imaginable is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Imaginable.  If not, see <http://www.gnu.org/licenses/>.
**
*************/


#ifndef IMAGINABLE__TINEMAP_GUI__MAINWINDOW__INCLUDED
#define IMAGINABLE__TINEMAP_GUI__MAINWINDOW__INCLUDED


#include "ui_mainwindow.h"


class QProgressBar;
class QLabel;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = NULL);

public slots:
	void openFile(void);
	void saveFile(void);

	void setSaturation(int);
	void setBlur(int);
	void setMix(int);
	void resetSliders(void);

private:
	QProgressBar *progress_bar;
	QString last_user_dir;
	QStringList filters;
	int open_filter;
	int save_filter;
};

#endif // IMAGINABLE__TINEMAP_GUI__MAINWINDOW__INCLUDED

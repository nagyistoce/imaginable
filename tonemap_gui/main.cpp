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


#include <QtGui/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QFileInfo>

#include "mainwindow.hpp"
#include "version.hpp"


int main(int argc,char *argv[])
{
	QApplication a(argc, argv);

	a.setOrganizationName("Kuzma Shapran");
	a.setApplicationName("tonemap");
	a.setApplicationVersion(QString::fromAscii(version::full_string()));

	QString locale = QLocale::system().name();

	QTranslator translator;
	QFileInfo app_fi(a.applicationFilePath());
	translator.load(QFileInfo(app_fi.path()).path()+"/lang/"+app_fi.fileName()+"_" + locale + ".qm");
	a.installTranslator(&translator);

	MainWindow w;
	w.show();

	int argi=1;
	for (bool ok=false; (!ok) && (argi != argc); ++argi)
		ok = w.loadFile(QString::fromLocal8Bit(argv[argi]));

	return a.exec();
}

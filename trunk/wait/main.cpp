/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009,2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#include <cerrno>
#include <cstdlib>
#include <iostream>

#include <QtCore/QMap>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMetaType>

#include "version.hpp"
#include "options.hpp"
#include "wait.hpp"


namespace {

void say_hello(void)
{
	QTextStream(stderr) << qPrintable(QString(
		"%1\n"
		"Copyright (C) 2009,%2 %3 <Kuzma.Shapran@gmail.com>\n"
		"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
		"\n"
		"This is free software; you are free to change and redistribute it.\n"
		"There is NO WARRANTY, to the extent permitted by law.\n"
		"\n"
		)
		.arg(QCoreApplication::applicationName())
		.arg(version::year())
		.arg(QCoreApplication::organizationName())
		);
}

void show_version(void)
{
	QTextStream(stderr) << qPrintable(QString(
		"Version %1 (built %2-%3-%4 %5:%6:%7)\n"
		"\n"
		)
		.arg(version::full_string())
		.arg(version::year())
		.arg(version::month() ,2,10,QChar('0'))
		.arg(version::day()   ,2,10,QChar('0'))
		.arg(version::hour()  ,2,10,QChar('0'))
		.arg(version::minute(),2,10,QChar('0'))
		.arg(version::second(),2,10,QChar('0'))
		);
}

Options *s_program_options;

}

const Options &program_options(void)
{
	return *s_program_options;
}

int main(int argc,char *argv[])
{
	QCoreApplication app(argc,argv);
	app.setOrganizationName("Kuzma Shapran");
	app.setApplicationName("Wait");
	app.setApplicationVersion(QString::fromAscii(version::full_string()));

	Options options;

	options.setFlag (     "--help");
	options.setAlias("-h","--help");
	options.setAlias("-?","--help");
	options.setInfo (     "--help"    ,"show this help and exit");

	options.setFlag (     "--no-hello");
	options.setAlias("-q","--no-hello");
	options.setInfo (     "--no-hello","do not show start message");

	options.setFlag (     "--version");
	options.setAlias("-V","--version");
	options.setInfo (     "--version" ,"show version and exit");

	s_program_options=&options;

	options.parse(QCoreApplication::arguments());


	if (options.flag("--help"))
	{
		say_hello();
		QTextStream(stderr) << "Possible options are:\n" << options.info()
			<< "\n"
			<< QString("Usage:\n%1 [Id] ...\n").arg(QFileInfo(QCoreApplication::argv()[0]).fileName());
		return EXIT_SUCCESS;
	}


	if (!options.flag("--no-hello"))
		say_hello();

	if (options.flag("--version"))
	{
		show_version();
		return EXIT_SUCCESS;
	}

	Wait wait;
	Q_UNUSED(wait);

	return app.exec();
}
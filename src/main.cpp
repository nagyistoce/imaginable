/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-02-16
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "version.hpp"
#include "options.hpp"
#include "root.hpp"

#include <cerrno>
#include <cstdlib>
#include <iostream>

#include <QtCore/QMap>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtDBus/QDBusConnection>


namespace {

void say_hello(void)
{
	QTextStream(stderr)<<qPrintable(QString(
		"Imaginable\n"
		"Copyright (C) %1 %2 <Kuzma.Shapran@gmail.com>\n"
		"License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n"
		"\n"
		"This is free software; you are free to change and redistribute it.\n"
		"There is NO WARRANTY, to the extent permitted by law.\n"
		"\n"
		)
		.arg(version::year())
		.arg(QCoreApplication::organizationName())
		);
}

void show_version(void)
{
	QTextStream(stderr)<<qPrintable(QString(
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

const char dbus_service_name[]="name.kuzmashapran.imaginable";

Options* s_program_options;

}

const Options& program_options(void)
{
	return *s_program_options;
}

int main(int argc,char* argv[])
{
	QCoreApplication app(argc,argv);
	app.setOrganizationName("Kuzma Shapran");
	app.setApplicationName("Imaginable");
	app.setApplicationVersion(QString::fromAscii(version::full_string()));

	Options options;
	options.setFlag("--help");
	options.setFlag("--no-hello");
	options.setFlag("--version");
	options.setFlag("--verbose");
	options.setAlias("-h","--help");
	options.setAlias("-?","--help");
	options.setAlias("-q","--no-hello");
	options.setAlias("-V","--version");
	options.setAlias("-v","--verbose");
	options.setInfo("--help","show this help and exit");
	options.setInfo("--version","show version and exit");
	options.setInfo("--no-hello","do not show start message");
	options.setInfo("--verbose","be a bit more verbose");
	options.setInfo("--input","input");
	s_program_options=&options;

	options.parse(QCoreApplication::arguments());


	if(options.flag("--help"))
	{
		say_hello();
		QTextStream(stderr)<<"Possible options are:\n"<<options.info();
		return EXIT_SUCCESS;
	}


	if(!options.flag("--no-hello"))
		say_hello();

	if(options.flag("--version"))
	{
		show_version();
		return EXIT_SUCCESS;
	}

	if(options.flag("--verbose"))
		QTextStream(stderr)<<"PID="<<QCoreApplication::applicationPid()<<"\n";


	if(!QDBusConnection::sessionBus().registerService(dbus_service_name))
	{
		QTextStream(stderr)<<qPrintable(QString("Cannot register D-Bus service '%1': Probably already running.\n").arg(dbus_service_name));
		return EXIT_FAILURE;
	}

	Root root;
	if(!root.init_DBus())
		return EXIT_FAILURE;

	return app.exec();
}

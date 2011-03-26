/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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

#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QPluginLoader>
#include <QtCore/QScopedPointer>

#include <options.hpp>
#include <resource_controller.hpp>

#include "version.hpp"


enum {
	OK = 0,
	NO_RESOURCE,
	RC_IS_NOT_A_FILE,
	RC_IS_RELATIVE,
	RC_LOAD_FAILED,
	RC_INSTANCE_FAILED,
	RC_CAST_FAILED
};

#define APPLICATION_NAME "Imaginable"
#define AUTHOR_NAME "Kuzma Shapran"
#define AUTHOR_EMAIL "Kuzma.Shapran@gmail.com"


namespace app {
	static Options program_options;
	static QString resource_name;
}

namespace {

void say_hello(void)
{
	QTextStream(stderr) << qPrintable(QString(
		"%1\n"
		"Abstract Resource Processor\n"
		"\n"
		"Copyright (C) 2010 - %2 %3 <" AUTHOR_EMAIL ">\n"
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
		"Version: %1 (built %2-%3-%4 %5:%6:%7)\n"
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

}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc,argv);
	app.setOrganizationName(AUTHOR_NAME);
	app.setApplicationName(APPLICATION_NAME);
	app.setApplicationVersion(QString::fromAscii(version::full_string()));

	app::program_options.setFlag ("--help");
	app::program_options.setAlias("--help","-h");
	app::program_options.setAlias("--help","-?");
	app::program_options.setInfo ("--help","show this help and exit");

	app::program_options.setFlag ("--version");
	app::program_options.setAlias("--version","-V");
	app::program_options.setInfo ("--version","show version and exit");

	app::program_options.setProperty("--resource",QString());
	app::program_options.setAlias   ("--resource","-R");
	app::program_options.setInfo    ("--resource","resource type");

	app::program_options.parse(QCoreApplication::arguments());


	say_hello();

	if (app::program_options.flag("--help"))
	{
		QTextStream(stderr) << "Possible options are:\n" << app::program_options.info();
		return OK;
	}

	if (app::program_options.flag("--version"))
	{
		show_version();
		return OK;
	}


	// determine resource type or die
	app::resource_name = QFileInfo(QString(argv[0])).fileName();
	QString default_application_name(QFileInfo(QCoreApplication::applicationFilePath()).fileName()+"-");
	if (app::resource_name.startsWith(default_application_name))
		app::resource_name = app::resource_name.mid(default_application_name.length());

	if (!app::program_options.property("--resource").toString().isEmpty())
		app::resource_name = app::program_options.property("--resource").toString();


	QSettings settings(APPLICATION_NAME,app::resource_name);
	if (!settings.value("controller").isValid())
	{
		QTextStream(stderr) << qPrintable(QString("Resource '%1' cannot be used: no controller specified").arg(app::resource_name)) << '\n';
		return NO_RESOURCE;
	}
	QString controller_filename = settings.value("controller").toString();
	QString controller_filepath = settings.value("controller_path").toString();

	QTextStream(stderr) << qPrintable(QString("Resource: '%1'").arg(app::resource_name)) << '\n';
	QTextStream(stderr) << qPrintable(QString("Description: '%1'").arg(settings.value("description").toString())) << '\n';


	// load resource module or die
	if ((!controller_filepath.isEmpty()) && (!QFileInfo(controller_filename).isAbsolute()))
	{
		if (   !controller_filepath.endsWith('/') && !controller_filename.startsWith('/'))
			controller_filename=controller_filepath+"/"+controller_filename;
		else if(controller_filepath.endsWith('/') &&  controller_filename.startsWith('/'))
			controller_filename=controller_filepath+controller_filename.mid(1);
		else
			controller_filename=controller_filepath+controller_filename;
	}
	if (QFileInfo(controller_filename).suffix().isEmpty())
		controller_filename += ".so";

	if (!QFileInfo(controller_filename).isAbsolute())
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: no absolute path specified")) << '\n';
		return RC_IS_RELATIVE;
	}
	else if (!QFileInfo(controller_filename).isFile())
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: file '%1' does not exist").arg(controller_filename)) << '\n';
		return RC_IS_NOT_A_FILE;
	}

	QTextStream(stderr) << qPrintable(QString("Controller: '%1'").arg(controller_filename)) << '\n';


	QScopedPointer<QPluginLoader> pluginLoader_ptr(new QPluginLoader(controller_filename));
	QPluginLoader *pluginLoader=pluginLoader_ptr.data();

	if (!pluginLoader->load())
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: load() failed: %1").arg(pluginLoader->errorString())) << '\n';
		return RC_LOAD_FAILED;
	}

	QObject *instance=pluginLoader->instance();
	if (!instance)
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: instance() failed: %1").arg(pluginLoader->errorString())) << '\n';
		return RC_INSTANCE_FAILED;
	}

	ResourceController *controller=qobject_cast<ResourceController*>(instance);
	if (!controller)
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: casting failed")) << '\n';
		return RC_CAST_FAILED;
	}

	int init_result=controller->init(app::program_options,settings);
	if (init_result)
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: init() failed")) << '\n';
		return init_result;
	}


	// load function modules for resource (according to configs & cmdline) or warn

	// open named pipe or die

	return app.exec();
}

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
#include <QtCore/QDir>
#include <QtNetwork/QLocalServer>

#include <resource_controller.hpp>
#include <options.hpp>
#include <settings.hpp>

#include "version.hpp"
#include "main.hpp"


enum {
	OK = 0,
	NO_RESOURCE,
	RC_IS_NOT_A_FILE,
	RC_IS_RELATIVE,
	RC_LOAD_FAILED,
	RC_INSTANCE_FAILED,
	RC_CAST_FAILED,
	SS_IS_RELATIVE,
	SS_LISTEN
};

#define APPLICATION_NAME "Imaginable"
#define AUTHOR_NAME "Kuzma Shapran"


namespace {

QString prepend_path(const QString &path,const QString &name)
{
	QString ret;
	if (   !path.endsWith('/') && !name.startsWith('/'))
		ret=path+"/"+name;
	else if(path.endsWith('/') &&  name.startsWith('/'))
		ret=path+name.mid(1);
	else
		ret=path+name;
	return ret;
}

}


Dispatcher::Dispatcher(QObject *_parent)
	: QObject(_parent)
{}

Dispatcher::~Dispatcher()
{}

void Dispatcher::say_hello(void)
{
	QTextStream(stderr) << qPrintable(QString(
		"%1\n"
		"Abstract Resource Processor\n"
		"\n"
		"Copyright (C) 2010 - %2 %3 <Kuzma.Shapran@gmail.com>\n"
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

void Dispatcher::show_version(void)
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

void Dispatcher::try_load_module(QString filename)
{
	QTextStream(stderr) << "[" << qPrintable(filename) << "]\n";
	// load module, check it and it's resource type, unload or add to loaded list


	QPluginLoader *pluginLoader=new QPluginLoader(filename,this);

	if (!pluginLoader->load())
	{
		QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: load() failed: %2").arg(filename).arg(pluginLoader->errorString())) << '\n';
		return;
	}

	Module *module;
	bool ok = false;
	do
	{
		QObject *instance=pluginLoader->instance();
		if (!instance)
		{
			QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: instance() failed: %2").arg(filename).arg(pluginLoader->errorString())) << '\n';
			break;
		}

		module=qobject_cast<Module*>(instance);
		if (!module)
		{
			QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: casting failed").arg(filename)) << '\n';
			break;
		}

		QString module_resource_type = module->resource_type();
		if (module_resource_type != resource_type)
		{
			QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: resource mismatch. Modules provides: '%2'").arg(filename).arg(module_resource_type)) << '\n';
			break;
		}

		if (module->provides().isEmpty())
		{
			QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: no functions provided").arg(filename)) << '\n';
			break;
		}

		foreach (FunctionDescription descr,module->provides())
			if (!descr.is_valid())
			{
				QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: provided function description is invalid").arg(filename)) << '\n';
				break;
			}

		foreach (FunctionDescription descr,module->depends_on())
			if (!descr.is_valid())
			{
				QTextStream(stderr) << qPrintable(QString("Module '%1' cannot be loaded: dependent function description is invalid").arg(filename)) << '\n';
				break;
			}

		ok = true;
	}
	while(false);
	if (!ok)
	{
		pluginLoader->unload();
	}
	else
	{
		QTextStream(stderr) << qPrintable(QString("Module '%1' loaded").arg(filename)) << '\n';

		QSharedPointer<Module> sh_module(module);

		plugin_by_module[sh_module] = pluginLoader;

		module_by_name[filename] = sh_module;
		module_name[sh_module] = filename;

		foreach (FunctionDescription descr,module->provides())
			function_provided_by[descr].insert(filename);

		foreach (FunctionDescription descr,module->depends_on())
			dependant_modules[descr].insert(filename);
	}
}

int Dispatcher::main(int /*argc*/, char *argv[])
{
	QCoreApplication *app=QCoreApplication::instance();

	app->setOrganizationName(AUTHOR_NAME);
	app->setApplicationName(APPLICATION_NAME);
	app->setApplicationVersion(QString::fromAscii(version::full_string()));

	Options program_options;

	program_options.setFlag ("--help");
	program_options.setAlias("--help","-h");
	program_options.setAlias("--help","-?");
	program_options.setInfo ("--help","show this help and exit");

	program_options.setFlag ("--version");
	program_options.setAlias("--version","-V");
	program_options.setInfo ("--version","show version and exit");

	program_options.setProperty("--config",QString());
	program_options.setAlias   ("--config","-c");
	program_options.setInfo    ("--config","resource type");

	program_options.setProperty("--no-modules",QString());
	program_options.setAlias   ("--no-modules","-M");
	program_options.setInfo    ("--no-modules","colon-separated module blacklist");

	program_options.setProperty("--extra-modules",QString());
	program_options.setAlias   ("--extra-modules","-e");
	program_options.setInfo    ("--extra-modules","colon-separated extra module list");

	program_options.setProperty("--no-module-paths",QString());
	program_options.setAlias   ("--no-module-paths","-P");
	program_options.setInfo    ("--no-module-paths","colon-separated module path blacklist");

	program_options.setProperty("--extra-module-paths",QString());
	program_options.setAlias   ("--extra-module-paths","-x");
	program_options.setInfo    ("--extra-module-paths","colon-separated extra module path list");

	program_options.parse(QCoreApplication::arguments());


	say_hello();

	if (program_options.flag("--help"))
	{
		QTextStream(stderr) << "Possible options are:\n" << program_options.info();
		return OK;
	}

	if (program_options.flag("--version"))
	{
		show_version();
		return OK;
	}




	// determine resource type or die
	config_name = QFileInfo(QString(argv[0])).fileName();
	QString default_application_prefix = (QFileInfo(QCoreApplication::applicationFilePath()).fileName())+"-";
	if (config_name.startsWith(default_application_prefix))
		config_name = config_name.mid(default_application_prefix.length());

	if (!program_options.property("--config").toString().isEmpty())
		config_name = program_options.property("--config").toString();


	Settings settings;
	// merge all settings
	{
		QSettings settings_file(APPLICATION_NAME,config_name);

		foreach (QString key,settings_file.allKeys())
			settings[key] = settings_file.value(key);

		foreach (QString key,program_options.allPublicOptions())
		{
			QString settings_key = key.startsWith("--") ? key.mid(QString("--").length()) : (key.startsWith("-") ? key.mid(QString("-").length()) : key);
			if ((program_options.property(qPrintable(key)).type() == QVariant::String)
			&&  (!program_options.property(qPrintable(key)).toString().isEmpty()))
				settings[settings_key] = program_options.property(qPrintable(key));
		}
	}


	if (!settings["controller"].isValid())
	{
		QTextStream(stderr) << qPrintable(QString("Resource '%1' cannot be used: no controller specified").arg(config_name)) << '\n';
		return NO_RESOURCE;
	}

	QString controller_filename = settings["controller"].toString();
	QString controller_filepath = settings["controller_path"].toString();
	server_filename             = settings["server_filename"].toString();

	QTextStream(stderr) << qPrintable(QString("Resource: '%1'").arg(config_name)) << '\n';
	QTextStream(stderr) << qPrintable(QString("Description: '%1'").arg(settings["description"].toString())) << '\n';


	// determine resource module file name
	if ((!controller_filepath.isEmpty()) && (!QFileInfo(controller_filename).isAbsolute()))
		controller_filename = prepend_path(controller_filepath,controller_filename);
	if (QFileInfo(controller_filename).suffix().isEmpty())
		controller_filename += ".so";

	if (QFileInfo(controller_filename).isRelative())
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: no absolute path specified")) << '\n';
		return RC_IS_RELATIVE;
	}

	if (!QFileInfo(controller_filename).isFile())
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: file '%1' does not exist").arg(controller_filename)) << '\n';
		return RC_IS_NOT_A_FILE;
	}

	QTextStream(stderr) << qPrintable(QString("Controller: '%1'").arg(controller_filename)) << '\n';


	// load resource module or die
	QPluginLoader *pluginLoader=new QPluginLoader(controller_filename,this);

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

	controller=qobject_cast<ResourceController*>(instance);
	if (!controller)
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: casting failed")) << '\n';
		return RC_CAST_FAILED;
	}

	int init_result=controller->init(settings);
	if (init_result)
	{
		QTextStream(stderr) << qPrintable(QString("Resource controller cannot be loaded: init() failed")) << '\n';
		return init_result;
	}


	// load function modules for resource (according to configs & cmdline) or warn
	resource_type = controller->type();

	// build full list (check resource type)
	QSet<QString> modules = settings      ["modules"].toString().split(':').toSet(); // user settings
	modules -=              settings   ["no-modules"].toString().split(':').toSet(); // program options
	modules +=              settings["extra-modules"].toString().split(':').toSet(); // program options

	QSet<QString> module_paths = settings   ["default-module-paths"].toString().split(':').toSet(); // system settings
	module_paths -=              settings["no-default-module-paths"].toString().split(':').toSet(); // user settings
	module_paths +=              settings           ["module-paths"].toString().split(':').toSet(); // user settings
	module_paths -=              settings        ["no-module-paths"].toString().split(':').toSet(); // program options
	module_paths +=              settings     ["extra-module-paths"].toString().split(':').toSet(); // program options


	foreach (QString module,modules)
	{
		if (module.startsWith(QDir::separator()))
		{
			try_load_module(module);
		}
		else if (module.contains(QDir::separator()))
		{
			QTextStream(stderr) << qPrintable(QString("Module '%1' skipped: relative path specified").arg(module)) << '\n';
		}
		else
		{
			foreach (QString module_path,module_paths)
			{
				module = prepend_path(module_path,module);
				try_load_module(module);
			}
		}
	}

	// build dep map. check cycles
	forever
	{
		QSet<QString> to_unload;

		for (QMap<FunctionDescription,QSet<QString> >::ConstIterator I=dependant_modules.constBegin(); I!=dependant_modules.constEnd(); ++I)
			if (!function_provided_by.contains(I.key()))
			{
				QTextStream(stderr) << qPrintable(QString("Function '%1' is not provided by any loaded module").arg(I.key().interface_toString())) << '\n';
				to_unload += I.value();
			}

		if (to_unload.empty())
			break;

		foreach (QString name,to_unload)
		{
			QTextStream(stderr) << qPrintable(QString("Unloading module '%1' due to lack of required functions").arg(name)) << '\n';
			QPluginLoader* pluginLoader;
			{
				QSharedPointer<Module> sh_module=module_by_name.take(name);

				pluginLoader=plugin_by_module.take(sh_module);
				module_name.remove(sh_module);

				for (QMap<FunctionDescription,QSet<QString> >::Iterator I=function_provided_by.begin(); I!=function_provided_by.end(); ++I)
					I.value().remove(name);
				for (QMap<FunctionDescription,QSet<QString> >::Iterator I=dependant_modules   .begin(); I!=dependant_modules   .end(); ++I)
					I.value().remove(name);
			}
			pluginLoader->unload();
		}
	}


	// open named pipe or die
	if (server_filename.isEmpty())
		server_filename = default_application_prefix+config_name;

	if (QFileInfo(server_filename).isRelative())
	{
		if (server_filename.contains(QDir::separator()))
		{
			QTextStream(stderr) << qPrintable(QString("Server socket cannot be opened: relative path specified")) << '\n';
			return SS_IS_RELATIVE;
		}
		else
			server_filename = prepend_path(QDir::tempPath(),server_filename);
	}

	server=new QLocalServer(this);

	QObject::connect(server,SIGNAL(newConnection()),this,SLOT(newConnection()));

	bool retry=true;
	forever
	{
		if (server->listen(server_filename))
		{
			QTextStream(stderr) << qPrintable(QString("Server is listening on '%1'").arg(server_filename)) << '\n';
			break;
		}
		else
		{
			QTextStream(stderr) << qPrintable(QString("Server socket cannot be opened: listen() failed: %1").arg(server->errorString())) << '\n';
			if (retry && (server->serverError() == QAbstractSocket::AddressInUseError))
			{
				retry = false;
				QTextStream(stderr) << qPrintable(QString("Trying to remove local server")) << '\n';
				server->close();
				QLocalServer::removeServer(server_filename);
			}
			else
				return SS_LISTEN;
		}
	}


	return app->exec();
}

void Dispatcher::newConnection(void)
{
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc,argv);
	Dispatcher dispatcher;

	return dispatcher.main(argc,argv);
}

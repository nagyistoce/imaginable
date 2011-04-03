/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2010 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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

#ifndef IMAGINABLE__IMAGINABLE__MAIN__INCLUDED
#define IMAGINABLE__IMAGINABLE__MAIN__INCLUDED


#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include <QtCore/QMultiMap>

#include <module.hpp>


class QString;
class ResourceController;
class QPluginLoader;
class QLocalServer;


class Dispatcher : public QObject
{
	Q_OBJECT

public:
	Dispatcher (QObject * =NULL);
	~Dispatcher ();

	int main (int argc, char *argv[]);

public slots:
	void newConnection (void);

private:
	QString config_name;
	QString server_filename;


	ResourceController *controller;
	QString resource_type;

	QMap<QSharedPointer<Module>,QPluginLoader*> plugin_by_module;
	QMap<QString,QSharedPointer<Module> > module_by_name;
	QMap<QSharedPointer<Module>,QString> module_name;

	QMap<FunctionDescription,QSet<QString> > function_provided_by;
	QMap<FunctionDescription,QSet<QString> > dependant_modules;

	QLocalServer *server;


	static void say_hello (void);
	static void show_version (void);

	void try_load_module (QString);
};


#endif // IMAGINABLE__IMAGINABLE__MAIN__INCLUDED

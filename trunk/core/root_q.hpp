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

#ifndef IMAGINABLE__CORE__ROOT_Q__INCLUDED
#define IMAGINABLE__CORE__ROOT_Q__INCLUDED


#include "root.hpp"
#include "image_q.hpp"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QTimer>
#include <QtCore/QPluginLoader>


class Image;

class Root_Q : public QObject, public Root
{
Q_OBJECT
public:
	Root_Q(QObject* parent=NULL);

	bool init(void);

signals:
	void imageCreated(qulonglong);
	void imageDeleted(qulonglong);

public:
	Q_PROPERTY(unsigned autoCloseTime READ autoCloseTime WRITE setAutoCloseTime)
	unsigned    autoCloseTime(void) const { return m_autoCloseTimer.interval()/(60*1000); }
	void     setAutoCloseTime(unsigned);
	void restartAutoCloseTimer(void) { setAutoCloseTime(autoCloseTime()); }

public slots:
	QString version(void) const;

	qulonglong createImage(void);
	Image*           image(qulonglong);
	bool          hasImage(qulonglong) const;
	uint       deleteImage(qulonglong);

	void message(int level,QString text,QString source="core",qulonglong Id=0ULL) const;
	void message(int level,QString text,qulonglong Id) const { message(level,text,"core",Id); }

	uint            loadPlugin       (QString fileName);
	QStringList  loadAllPlugins      (QString  dirName);
	QStringList autoLoadPlugins      (QStringList names);
	bool              isPluginLoaded (QString fileName) const;
	QString             pluginName   (QString fileName) const;
	QString             pluginVersion(QString fileName) const;
	QStringList         pluginsList  (void)             const;
	uint          unloadPlugin       (QString fileName);

	void quit(void);

protected:
	typedef QMap<qulonglong,Image_Q*> Images;
	Images m_images;

	qulonglong nextIndex(void);

private slots:
	void autoCloseTimeout(void);

private:
	QTimer m_autoCloseTimer;

	void restartAutoCloser(void);

	typedef QMap<QString,QPluginLoader*> Plugins;
	Plugins m_plugins;
};

#endif // IMAGINABLE__CORE__ROOT_Q__INCLUDED

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

#ifndef IMAGINABLE__CORE__OPTIONS__INCLUDED
#define IMAGINABLE__CORE__OPTIONS__INCLUDED


#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QVariant>


class Options : public QObject
{
	Q_OBJECT

public:
	Options(QObject * =NULL);

	void setFlag(const char *name) { setProperty(name,QVariant(false)); }

	void setAlias(const char *option,const char *alias);

	void setInfo(const char *option,QString info);


	bool parse(QStringList);
	bool parsed(void) const { return m_parsed; }

	QString info(void) const { makeInfo(); return m_info; }


	bool flag(const char *name) const { return property(name).toBool(); }

	QStringList unnamed(void) const
	{ return m_optionUnnamed; }

	bool hasProperty(const char *name) const { return property(name).isValid(); }

protected:
	QMap<QString,QString> m_optionAlias;
	QMap<QString,QString> m_optionInfo;

	void makeInfo(void) const;
	mutable QString m_info;

	QStringList m_optionUnnamed;

	bool m_parsed;
};

#endif // IMAGINABLE__CORE__OPTIONS__INCLUDED

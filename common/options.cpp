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


#include "options.hpp"


Options::Options(QObject *parent_)
	: QObject(parent_)
	, m_parsed(false)
{
}

void Options::setAlias(const char *option,const char *alias)
{
	m_optionAlias[QString(alias)]=option;
	if (!property(option).isValid())
		setProperty(option,QVariant(false));
}

void Options::setInfo(const char *option,QString info)
{
	m_optionInfo[QString(option)]=info;
	if (!property(option).isValid())
		setProperty(option,QVariant(false));
}

bool Options::parse(QStringList arguments)
{
	makeInfo();

	if (m_parsed)
		return false;
	m_parsed=true;

	QString option;
	for (int i=1; i<arguments.size(); ++i)
	{
		QString argument=arguments.at(i);
		if (option.isEmpty())
		{
			if (m_optionAlias.contains(argument))
				argument=m_optionAlias[argument];

			int eq=argument.indexOf('=');
			if (eq != -1)
			{
				arguments.insert(i+1,argument.mid(eq+1));
				argument=argument.left(eq);
			}

			if (dynamicPropertyNames().contains(QByteArray(qPrintable(argument))))
			{
				if (property(qPrintable(argument)).type() == QVariant::String)
					option=argument;
				else
					setProperty(qPrintable(argument),QVariant(true));
			}
			else
				m_optionUnnamed << argument;
		}
		else
		{
			setProperty(qPrintable(option),QVariant(argument));
			option=QString();
		}
	}
	return option.isEmpty();
}

void Options::makeInfo(void) const
{
	if (!m_info.isEmpty())
		return;

	int widest=0;
	foreach (QString option,m_optionInfo.keys())
	{
		int len=option.length();
		if (widest<len)
			widest=len;
	}
	++widest;

	foreach (QByteArray ba_option,dynamicPropertyNames())
	{
		QString option(ba_option);
		if (m_optionInfo.contains(option))
		{
			m_info+="\t"+option+QString(widest-option.length(),QChar(' '))+"-- ";
			if ( (property(qPrintable(option)).type() == QVariant::String)
			&&  (!property(qPrintable(option)).toString().isEmpty()) )
				m_info += "[='"+property(qPrintable(option)).toString()+"'] ";
			m_info += m_optionInfo.value(option)+"\n";
		}
	}
}

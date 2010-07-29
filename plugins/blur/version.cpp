
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


#include <cstdio>
#include <cstdlib>

#include <string>

#include "version.hpp"


namespace version {

unsigned major(void)
{
	return
#include "version-major.inl"
	;
}

unsigned minor(void)
{
	return
#include "version-minor.inl"
	;
}

const char* revision(void)
{
	return
#include "version-revision.auto.inl"
	;
}

unsigned number(void)
{
	return
#include "version-number.auto.inl"
	;
}

const char* label(void)
{
	return
#include "version-label.inl"
#ifdef _DEBUG
"-debug"
#endif
	;
}

time_t time(void)
{
	return
#include "version-time.auto.inl"
	;
}

static std::string full_str;

const char* full_string(void)
{
	if (full_str.empty())
	{
		char* c_str;
		if (asprintf(&c_str,"%d.%d.%s.%d%s%s",
			major,
			minor,
			revision,
			number,
			(*label && (label[0]!='-')) ? "-" : "",
			label )>0)
		{
			full_str=std::string(c_str);
			free(static_cast<void*>(c_str));
		}
	}
	return full_str.c_str();
}

static std::string ubuntu_style_str;

const char* ubuntu_style_string(void)
{
	if (ubuntu_style_str.empty())
	{
		const struct tm* _tm=localtime(&time);
		char* c_str;
		if (asprintf(&c_str,"%02d.%02d",
			_tm->tm_year-100,
			_tm->tm_mon+1 )>0)
		{
			ubuntu_style_str=std::string(c_str);
			free(static_cast<void*>(c_str));
		}
	}
	return ubuntu_style_str.c_str();
}

}

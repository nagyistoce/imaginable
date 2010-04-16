/*************
**
** Project:      Imaginable
** File info:    $Id: version.cpp 14 2010-04-08 09:31:32Z Kuzma.Shapran@gmail.com $
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


#include "version.hpp"

#include <cstdio>
#include <cstdlib>

#include <string>


namespace version {

const unsigned major =
#include "version-major.inl"
;

const unsigned minor =
#include "version-minor.inl"
;

const char* revision =
#include "version-revision.auto.inl"
;

const char* label =
"-"
#include "version-label.inl"
#ifdef _DEBUG
"-debug"
#endif
;

const unsigned number =
#include "version-number.auto.inl"
;

const time_t time =
#include "version-time.auto.inl"
;

std::string full_str;

const char* full_string(void)
{
	if(full_str.empty())
	{
		char* c_str;
		if(asprintf(&c_str,"%d.%d.%s.%d%s",
			major,
			minor,
			revision,
			number,
			label )>0)
		{
			full_str=std::string(c_str);
		free(static_cast<void*>(c_str));
		}
	}
	return full_str.c_str();
}

std::string ubuntu_style_str;

const char* ubuntu_style_string(void)
{
	if(ubuntu_style_str.empty())
	{
		const struct tm* _tm=localtime(&time);
		char* c_str;
		if(asprintf(&c_str,"%02d.%02d",
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

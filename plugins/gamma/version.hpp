/*************
**
** Project:      Imaginable
** File info:    $Id: version.hpp 14 2010-04-08 09:31:32Z Kuzma.Shapran@gmail.com $
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

#ifndef IMAGINABLE__PLUGINS__GAMMA__VERSION__INCLUDED
#define IMAGINABLE__PLUGINS__GAMMA__VERSION__INCLUDED


#include <ctime>


namespace version {

	extern const unsigned major;
	extern const unsigned minor;
	extern const char* revision;
	extern const unsigned number;
	extern const char* label;

	extern const char* full_string(void);

	extern const time_t time;

	inline unsigned year  (void) { return localtime(&time)->tm_year+1900; }
	inline unsigned month (void) { return localtime(&time)->tm_mon+1; }
	inline unsigned day   (void) { return localtime(&time)->tm_mday; }
	inline unsigned hour  (void) { return localtime(&time)->tm_hour; }
	inline unsigned minute(void) { return localtime(&time)->tm_min; }
	inline unsigned second(void) { return localtime(&time)->tm_sec; }

	extern const char* ubuntu_style_string(void);
	inline double ubuntu_style(void)
	{
		const struct tm* _tm=localtime(&time);
		return static_cast<double>(_tm->tm_year-100)+static_cast<double>(_tm->tm_mon+1)/100.;
	}
}

#endif // IMAGINABLE__PLUGINS__GAMMA__VERSION__INCLUDED

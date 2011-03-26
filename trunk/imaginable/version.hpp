/*************
**
** Project:      Imaginable
** File info:    $Id: version.hpp 31 2010-07-29 19:59:27Z Kuzma.Shapran $
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

#ifndef IMAGINABLE__IMAGINABLE__VERSION__INCLUDED
#define IMAGINABLE__IMAGINABLE__VERSION__INCLUDED


#include <ctime>


namespace version {

	unsigned    major      (void);
	unsigned    minor      (void);
	const char* revision   (void);
	unsigned    number     (void);
	const char* label      (void);

	const char* full_string(void);

	time_t      time       (void);

	unsigned    year       (void);
	unsigned    month      (void);
	unsigned    day        (void);
	unsigned    hour       (void);
	unsigned    minute     (void);
	unsigned    second     (void);

	const char* ubuntu_style_string(void);
	double      ubuntu_style(void);
}

#endif // IMAGINABLE__IMAGINABLE__VERSION__INCLUDED

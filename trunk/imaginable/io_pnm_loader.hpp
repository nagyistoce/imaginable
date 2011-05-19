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


#ifndef IMAGINABLE__PNM_LOADER__INCLUDED
#define IMAGINABLE__PNM_LOADER__INCLUDED


#include <istream>

#include "image.hpp"


namespace imaginable {

class PNM_loader
{
public:
	PNM_loader(Image&);
	~PNM_loader();

	void load(std::istream&);

private:
	Image& m_image;
};

inline std::istream& operator >> (std::istream& stream,PNM_loader pnm)
{
	pnm.load(stream);
	return stream;
}

}

#endif // IMAGINABLE__PNM_LOADER__INCLUDED

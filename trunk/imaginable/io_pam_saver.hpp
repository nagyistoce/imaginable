/*************
**
** Project:      Imaginable
** File info:    $Id$
** Author:       Copyright (C) 2009 - 2011 Kuzma Shapran <Kuzma.Shapran@gmail.com>
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


#ifndef IMAGINABLE__PAM_SAVER__INCLUDED
#define IMAGINABLE__PAM_SAVER__INCLUDED


#include <ostream>

#include "image.hpp"


namespace imaginable {

class PAM_saver
{
public:
	PAM_saver(const Image&);
	~PAM_saver();

	void save(std::ostream&) const;

private:
	const Image& m_image;
};

inline std::ostream& operator << (std::ostream& stream, const PAM_saver& pam)
{
	pam.save(stream);
	return stream;
}

}

#endif // IMAGINABLE__PAM_SAVER__INCLUDED

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


#ifndef IMAGINABLE__TOOLS_ROOF__INCLUDED
#define IMAGINABLE__TOOLS_ROOF__INCLUDED


#include "notifier.hpp"
#include "image.hpp"


namespace imaginable
{
	void roof_linear_straight(Image& img, unsigned planeName, double step,             const Progress_notifier &notifier = dont_notify);

	void roof_linear_diagonal(Image& img, unsigned planeName, double step,             const Progress_notifier &notifier = dont_notify);

	void roof_linear_8       (Image& img, unsigned planeName, double step,             const Progress_notifier &notifier = dont_notify);

	void roof_parabolic      (Image& img, unsigned planeName, double k,                const Progress_notifier &notifier = dont_notify);

	void roof_exponential    (Image& img, unsigned planeName, double a, size_t radius, const Progress_notifier &notifier = dont_notify);
}

#endif // IMAGINABLE__TOOLS_ROOF__INCLUDED

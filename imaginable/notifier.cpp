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


#include "time.hpp"
#include "notifier.hpp"


namespace imaginable
{

Dont_notify dont_notify;

double Timed_progress_notifier::now(void) const
{
	return getHighPrecTime();
}

void Timed_progress_notifier::update(double value) const
{
	double current = this->now();
	if (current - m_last_update > m_timeout)
	{
		m_last_update = current;
		m_notifier.update(value);
	}
}

void Scaled_progress_notifier::update(double value) const
{
	m_notifier.update(m_offset + m_scale * value);
}

}

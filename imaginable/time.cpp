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


#include <QtCore/QElapsedTimer>

#include "time.hpp"


struct HPT_wrapper
{
	QElapsedTimer elapsedTimer;

	HPT_wrapper()
	{
		elapsedTimer.start();
	}

	double elapsed(void) const
	{
		return static_cast<double>(elapsedTimer.nsecsElapsed())/1000000000.;
	}
} highPrecTime;

double getHighPrecTime(void)
{
	return highPrecTime.elapsed();
}

#ifdef __linux__
void setTimeSpec(struct timespec& time_s, double time)
{
	unsigned long time_i=static_cast<unsigned long>(time*1000000000.);
	time_s.tv_sec =time_i/1000000000;
	time_s.tv_nsec=time_i%1000000000;
}
#endif

void setTimeVal(struct timeval& time_v, double time)
{
	unsigned long time_i=static_cast<unsigned long>(time*1000000.);
	time_v.tv_sec =time_i/1000000;
	time_v.tv_usec=time_i%1000000;
}

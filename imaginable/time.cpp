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

#include <cstddef>


#if defined(_STRUCT_TIMEVAL) || defined(_TIMEVAL_DEFINED)
	#define USE_GETTIMEOFDAY 1
#endif

#ifndef USE_GETTIMEOFDAY
	static const double start_time=static_cast<double>(time(NULL));
#endif


#define USE_GETTIME 1


double getHighPrecTime(void)
{
#ifdef USE_GETTIME
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC_RAW,&now);
	return static_cast<double>(now.tv_sec)+(static_cast<double>(now.tv_nsec))/1000000000.;
#elif USE_GETTIMEOFDAY
	struct timeval now;
	gettimeofday(&now,NULL);
	return static_cast<double>(now.tv_sec)+(static_cast<double>(now.tv_usec))/1000000.;
#else
	#warning clock() is used for high precision timer
	return static_cast<double>(clock())/static_cast<double>(CLOCKS_PER_SEC)+static_cast<double>(time(NULL))-start_time;
#endif
}

void setTimeSpec(struct timespec& time_s,double time)
{
	unsigned long time_i=static_cast<unsigned long>(time*1000000000.);
	time_s.tv_sec =time_i/1000000000;
	time_s.tv_nsec=time_i%1000000000;
}

void setTimeVal(struct timeval& time_v,double time)
{
	unsigned long time_i=static_cast<unsigned long>(time*1000000.);
	time_v.tv_sec =time_i/1000000;
	time_v.tv_usec=time_i%1000000;
}

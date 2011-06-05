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


#include <boost/bind.hpp>

#include "time.hpp"
#include "tools.hpp"


namespace imaginable
{

TimedProgress::TimedProgress(progress_notifier notifier,double timeout)
	: m_notifier(notifier)
	, m_timeout(timeout)
	, m_last_update(0)
{
}

TimedProgress::~TimedProgress()
{
}

void TimedProgress::update(float value)
{
	double now_=this->now();
	if (now_ - m_last_update > m_timeout)
	{
		m_last_update = now_;
		m_notifier(value);
	}
}

double TimedProgress::now(void)
{
	return getHighPrecTime();
}

progress_notifier TimedProgress::notifier(void)
{
	return boost::bind(&TimedProgress::update,this,_1);
}


void scaled_notifier(progress_notifier notifier,float offset,float scale,float value)
{
	notifier(offset+scale*value);
}

}
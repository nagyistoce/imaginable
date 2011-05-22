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


#ifndef IMAGINABLE__TOOLS__INCLUDED
#define IMAGINABLE__TOOLS__INCLUDED


#include <boost/function.hpp>
#include <exception>

#include "image.hpp"


namespace imaginable
{
	class exception : public std::exception
	{
	public:
		typedef enum
		{
			NO_IMAGE=0,             //crop  rgb_to_hsv  box_blur tone_map
			NO_ALPHA_CHANNEL,       //crop
			FULLY_TRANSPARENT_IMAGE,//crop
			INVALID_COLOUR_SPACE,   //      rgb_to_hsv           tone_map
			INVALID_RADIUS,         //                  box_blur
			INVALID_PLANE,          //                  box_blur
			PLANE_EXISTS,           //                  box_blur
			NOT_AN_HDRI,            //                           tone_map
			INVALID_DATA            //                           tone_map
		} Type;
		explicit exception(const Type& type) throw()
			: std::exception()
			, m_type(type)
		{}
		~exception() throw()
		{}
		inline Type type(void) const throw() { return m_type; }
		inline const char* what() const throw()
		{
		switch(m_type)
			{
				case NO_IMAGE:                return "No image";
				case NO_ALPHA_CHANNEL:        return "No alpha channel";
				case FULLY_TRANSPARENT_IMAGE: return "Fully transparent image";
				case INVALID_COLOUR_SPACE:    return "Invalid colour space";
				case INVALID_RADIUS:          return "Invalid radius";
				case INVALID_PLANE:           return "Invalid plane";
				case PLANE_EXISTS:            return "Plane exists";
				case NOT_AN_HDRI:             return "Not an HDR image";
				case INVALID_DATA:            return "Invalid pixel data";
			}
			return std::exception::what();
		}
	private:
		exception(void)
			: std::exception()
		{}
		Type m_type;
	};


	typedef boost::function<void (float)> progress_notifier;

	inline void dont_notify(float) {}

	class TimedProgress
	{
	public:
		TimedProgress(progress_notifier,double timeout=0.33);
		~TimedProgress();

		void percent(float);

		virtual double now(void);

	private:
		progress_notifier m_notifier;
		double m_timeout;
		double m_last_update;
	};
}

#endif // IMAGINABLE__TOOLS__INCLUDED

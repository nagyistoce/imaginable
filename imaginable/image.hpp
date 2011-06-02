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


#ifndef IMAGINABLE__IMAGE__INCLUDED
#define IMAGINABLE__IMAGE__INCLUDED


#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

#include <map>
#include <set>
#include <string>

#include <stdint.h>


namespace imaginable {

class Image
{
public:
	typedef enum
	{
		IMAGE_EMPTY=0,
		IMAGE_MONO,
		IMAGE_GREY,
		IMAGE_RGB,
		IMAGE_HSV,
		IMAGE_HSL,
		IMAGE_MASK,
		IMAGE_UNUSUAL
	} t_colourSpace;

	typedef enum
	{
		PLANE_MONO=0,
		PLANE_GREY,
		PLANE_RED,
		PLANE_GREEN,
		PLANE_BLUE,
		PLANE_HUE,
		PLANE_HSV_SATURATION,
		PLANE_HSV_VALUE,
		PLANE_HSL_SATURATION,
		PLANE_HSL_LIGHTNESS,
		PLANE_ALPHA,
		PLANE__USER,
		PLANE__INTERNAL=0xffffffff
	} t_planeName;

	typedef std::set<unsigned> t_planeNames;

	typedef uint16_t pixel;

	static const pixel MAXIMUM=static_cast<pixel>(-1);

	typedef std::set<std::string> t_text_keys;

private:
	typedef boost::shared_array<pixel> t_plane;
	typedef std::map<unsigned,t_plane> t_planes;
	typedef std::map<unsigned,int> t_unloaded_planes;
	typedef std::map<std::string,std::string> t_text;

	size_t m_width;
	size_t m_height;
	pixel m_maximum;
	mutable t_planes m_plane;
	mutable t_unloaded_planes m_unloaded_planes;

	t_text m_text;

public:
	Image(void);
	Image(size_t width,size_t height);
	~Image();


	boost::shared_ptr<Image> copy(void) const;


	inline size_t width  (void) const { return m_width;  }
	inline size_t height (void) const { return m_height; }
	inline pixel  maximum(void) const { return m_maximum; }

	bool setWidth (size_t);
	bool setHeight(size_t);
	bool setSize(size_t width,size_t height);
	inline void setMaximum(pixel maximum) { m_maximum=maximum; }


	t_colourSpace colourSpace    (void) const;
	t_planeNames  planeNames     (void) const;
	inline bool   hasPlane       (unsigned planeName) const { return m_plane.find(planeName)!=m_plane.end(); }
	inline bool   hasTransparency(void) const { return hasPlane(PLANE_ALPHA); }
	inline size_t planesNumber   (void) const { return m_plane.size(); }

	bool          addPlane   (unsigned);
	bool          removePlane(unsigned);
	bool          renamePlane(unsigned from,unsigned to);

	const pixel* plane(unsigned) const;
	/* */ pixel* plane(unsigned);


	bool unloadPlane(unsigned) const;
	bool reloadPlane(unsigned) const;
	bool planeUnloaded(unsigned) const;


	inline bool   empty  (void) const { return ((!m_width) && (!m_height)); }
	inline bool   hasData(void) const { return ( (!empty()) && (!m_plane.empty()) ); }


	void clear(void);


	inline bool        hasText     (const std::string& key)                          const { return m_text.find(key)!=m_text.end(); }
	inline std::string text        (const std::string& key)                          const { t_text::const_iterator I=m_text.find(key); return (I!=m_text.end())?I->second:""; }
	t_text_keys        text_keys   (void)                                            const;
	inline void        setText     (const std::string& key,const std::string& value)       { m_text[key]=value; }
	inline void        removeText  (const std::string& key)                                { m_text.erase(key); }
	inline void        clearText   (void)                                                  { m_text.clear(); }
	inline void        copyTextFrom(const Image& src)                                      { m_text=src.m_text; }
};

}

#endif // IMAGINABLE__IMAGE__INCLUDED

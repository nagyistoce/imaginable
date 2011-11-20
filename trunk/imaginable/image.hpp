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
		COLOURSPACE_NONE=0,
		COLOURSPACE_ALPHA,
		COLOURSPACE_GRAY,
		COLOURSPACE_RGB,
		COLOURSPACE_HSL,
		COLOURSPACE_HSV,
		COLOURSPACE_HCY,
		COLOURSPACE_CUSTOM
	} ColourSpace;

	typedef enum
	{
		PLANE_ALPHA=0,    PLANE_TRASPARENT = PLANE_ALPHA,
		PLANE_GRAY,       PLANE_GREY = PLANE_GRAY, PLANE_MONO = PLANE_GRAY, PLANE_MONOCHROME = PLANE_GRAY,
		PLANE_RED,
		PLANE_GREEN,
		PLANE_BLUE,
		PLANE_HUE,
		PLANE_SATURATION,
		PLANE_LIGHTNESS,
		PLANE_VALUE,
		PLANE_CHROMA,
		PLANE_LUMA,          PLANE_Y = PLANE_LUMA,
		PLANE__USER,
		PLANE__INTERNAL=0xffffffff
	} PlaneName;

	typedef std::set<unsigned> PlaneNames;

	typedef uint16_t Pixel;

	static const Pixel MAXIMUM=static_cast<Pixel>(-1);

	typedef std::set<std::string> TextKeys;

private:
	typedef boost::shared_array<Pixel> Plane;
	typedef std::map<unsigned,Plane> Planes;
	typedef std::map<unsigned,int> UnloadedPlanes;
	typedef std::map<std::string,std::string> Text;

	size_t m_width;
	size_t m_height;
	Pixel m_maximum;
	mutable Planes m_plane;
	mutable UnloadedPlanes m_unloaded_planes;

	Text m_text;

public:
	Image(void);
	Image(size_t width,size_t height);
	~Image();


	boost::shared_ptr<Image> copy(void) const;


	inline size_t width  (void) const { return m_width;  }
	inline size_t height (void) const { return m_height; }
	inline Pixel  maximum(void) const { return m_maximum; }

	bool setWidth (size_t);
	bool setHeight(size_t);
	bool setSize(size_t width,size_t height);
	inline void setMaximum(Pixel maximum) { m_maximum=maximum; }


	ColourSpace colourSpace    (void) const;
	PlaneNames  planeNames     (void) const;
	inline bool   hasPlane       (unsigned planeName) const { return m_plane.find(planeName)!=m_plane.end(); }
	inline bool   hasTransparency(void) const { return hasPlane(PLANE_ALPHA); }
	inline size_t planesNumber   (void) const { return m_plane.size(); }

	bool          addPlane   (unsigned);
	bool          removePlane(unsigned);
	bool          renamePlane(unsigned from,unsigned to);

	const Pixel* plane(unsigned) const;
	/* */ Pixel* plane(unsigned);


	bool unloadPlane(unsigned) const;
	bool reloadPlane(unsigned) const;
	bool planeUnloaded(unsigned) const;


	inline bool   empty  (void) const { return ((!m_width) && (!m_height)); }
	inline bool   hasData(void) const { return ( (!empty()) && (!m_plane.empty()) ); }


	void clear(void);


	inline bool        hasText     (const std::string& key)                          const { return m_text.find(key)!=m_text.end(); }
	inline std::string text        (const std::string& key)                          const { Text::const_iterator I=m_text.find(key); return (I!=m_text.end())?I->second:""; }
	TextKeys           text_keys   (void)                                            const;
	inline void        setText     (const std::string& key,const std::string& value)       { m_text[key]=value; }
	inline void        removeText  (const std::string& key)                                { m_text.erase(key); }
	inline void        clearText   (void)                                                  { m_text.clear(); }
	inline void        copyTextFrom(const Image& src)                                      { m_text=src.m_text; }
};

typedef boost::shared_ptr<Image> SharedImage;

}

#endif // IMAGINABLE__IMAGE__INCLUDED

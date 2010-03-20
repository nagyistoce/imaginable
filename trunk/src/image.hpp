/* * * * * *
 *
 * Project:   Imaginable
 * Created:   2009-09-03
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$

#ifndef IMAGINABLE__IMAGE__INCLUDED
#define IMAGINABLE__IMAGE__INCLUDED


#include <stdint.h>

#include <map>
#include <set>
#include <string>

#include <boost/shared_array.hpp>


class Image
{
public:
	typedef enum
	{
		SPACE__CUSTOM=-1,
		SPACE__EMPTY=0,
		SPACE_LIGHTNESS,
		SPACE_MONO       = SPACE_LIGHTNESS,
		SPACE_MONOCHROME = SPACE_MONO,
		SPACE_GREY       = SPACE_LIGHTNESS,
		SPACE_GRAY       = SPACE_GREY,
		SPACE_RGB,///< note that RGB can mean RGBA
		SPACE_HSV,///< note that HSV can mean HSVA
		SPACE_HSL,///< note that HSL can mean HSLA
		SPACE_ALPHA,
		SPACE_MASK       = SPACE_ALPHA
	} ColourSpace;

	enum
	{
		PLANE_ALPHA=0,
		PLANE_RED,
		PLANE_GREEN,
		PLANE_BLUE,
		PLANE_HUE_SECTOR,
		PLANE_HUE,
		PLANE_HSV_SATURATION,
		PLANE_HSV_VALUE,
		PLANE_HSL_SATURATION,
		PLANE_LIGHTNESS,
		PLANE__USER    = 0x100,
		PLANE__INTERNAL=-1
	};
	typedef int ColourPlane;

	typedef std::set<ColourPlane> ColourPlaneSet;
	typedef uint16_t Pixel;
	typedef std::set<std::string> TextKeys;

protected:
	typedef boost::shared_array<Pixel> Plane;
	typedef std::map<ColourPlane,Plane> ColourPlanes;
	typedef std::map<ColourPlane,std::string> ColourPlaneNames;
	typedef std::map<std::string,std::string> Text;

public:
	Image(void);
	Image(size_t width,size_t height,Pixel maximum=static_cast<Pixel>(-1));
	virtual ~Image();


	Image* clone(void) const;
	void copyFrom(const Image& src) { src.copyTo(*this); }
	void copyTo(Image&) const;


	size_t width  (void) const { return m_width;   }
	size_t height (void) const { return m_height;  }
	size_t square (void) const { return m_width * m_height;  }
	Pixel  maximum(void) const { return m_maximum; }

	bool setWidth (size_t);
	bool setHeight(size_t);
	bool setSize(size_t width,size_t height);
	void setMaximum(Pixel maximum) { m_maximum=maximum; }


	/// returns true when image has non-zero dimension even if it does not have any colour plane
	bool empty  (void) const { return ( (!m_width) && (!m_height) ); }
	/// returns true when image has at least one pixel of data
	bool hasData(void) const { return ( (!empty()) && (!m_plane.empty()) ); }


	void clear(void);


	ColourSpace colourSpace(void) const;

	ColourPlaneSet planes  (void) const;
	bool        hasPlane   (ColourPlane plane) const { return m_plane.find(plane) != m_plane.end(); }
	bool        hasAlpha   (void) const { return hasPlane(PLANE_ALPHA); }
	size_t      planesCount(void) const { return m_plane.size(); }

	bool        addPlane   (ColourPlane);
	bool        removePlane(ColourPlane);
	bool        movePlane  (ColourPlane from,ColourPlane to);

	bool        planeHasName  (ColourPlane) const;
	std::string planeName     (ColourPlane) const;
	bool        setPlaneName  (ColourPlane,const std::string&);
	bool        erasePlaneName(ColourPlane);


	const Pixel* plane(ColourPlane) const;
	/* */ Pixel* plane(ColourPlane);


	bool        hasText     (const std::string& key)                          const { return m_text.find(key)!=m_text.end(); }
	std::string text        (const std::string& key)                          const;
	TextKeys    textKeys    (void)                                            const;
	bool        setText     (const std::string& key,const std::string& value);
	bool        eraseText   (const std::string& key);
	void        clearTexts  (void)                                            { m_text.clear(); }

protected:
	size_t m_width;
	size_t m_height;
	Pixel m_maximum;
	ColourPlanes m_plane;
	ColourPlaneNames m_planeName;
	Text m_text;
};

#endif // IMAGINABLE__IMAGE__INCLUDED

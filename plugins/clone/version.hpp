/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-11
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id: version.hpp 9 2010-03-20 09:53:22Z Kuzma.Shapran $

#ifndef IMAGINABLE__VERSION__INCLUDED
#define IMAGINABLE__VERSION__INCLUDED


#include <ctime>


namespace version {

	extern const unsigned major;
	extern const unsigned minor;
	extern const char* revision;
	extern const unsigned number;
	extern const char* label;

	extern const char* full_string(void);

	extern const time_t time;

	inline unsigned year  (void) { return localtime(&time)->tm_year+1900; }
	inline unsigned month (void) { return localtime(&time)->tm_mon+1; }
	inline unsigned day   (void) { return localtime(&time)->tm_mday; }
	inline unsigned hour  (void) { return localtime(&time)->tm_hour; }
	inline unsigned minute(void) { return localtime(&time)->tm_min; }
	inline unsigned second(void) { return localtime(&time)->tm_sec; }

	extern const char* ubuntu_style_string(void);
	inline double ubuntu_style(void)
	{
		const struct tm* _tm=localtime(&time);
		return static_cast<double>(_tm->tm_year-100)+static_cast<double>(_tm->tm_mon+1)/100.;
	}
}

#endif // IMAGINABLE__VERSION__INCLUDED

/* * * * * *
 *
 * Title:     Imaginable
 * Created:   2010-03-11
 * Author:    Kuzma Shapran <Kuzma.Shapran@gmail.com>
 * License:   GPLv3
 *
 * * * * * */
// $Id$


#include "version.hpp"

#include <cstdio>
#include <cstdlib>

#include <string>


namespace version {

const unsigned major =
#include "version-major.inl"
;

const unsigned minor =
#include "version-minor.inl"
;

const char* revision =
#include "version-revision.auto.inl"
;

const char* label =
"-"
#include "version-label.inl"
#ifdef _DEBUG
"-debug"
#endif
;

const unsigned number =
#include "version-number.auto.inl"
;

const time_t time =
#include "version-time.auto.inl"
;

std::string full_str;

const char* full_string(void)
{
	if(full_str.empty())
	{
		char* c_str;
		if(asprintf(&c_str,"%d.%d.%s.%d%s",
			major,
			minor,
			revision,
			number,
			label )>0)
		{
			full_str=std::string(c_str);
		free(static_cast<void*>(c_str));
		}
	}
	return full_str.c_str();
}

std::string ubuntu_style_str;

const char* ubuntu_style_string(void)
{
	if(ubuntu_style_str.empty())
	{
		const struct tm* _tm=localtime(&time);
		char* c_str;
		if(asprintf(&c_str,"%02d.%02d",
			_tm->tm_year-100,
			_tm->tm_mon+1 )>0)
		{
			ubuntu_style_str=std::string(c_str);
			free(static_cast<void*>(c_str));
		}
	}
	return ubuntu_style_str.c_str();
}

}

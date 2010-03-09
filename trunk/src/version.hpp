#include <time.h>

namespace version {
	extern const unsigned major;
	extern const unsigned minor;
	extern const char* revision;
	extern const char* label;
	extern const time_t built;

	inline unsigned year(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_year+1900;
	}
	inline unsigned month(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_mon+1;
	}
	inline unsigned day(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_mday;
	}
	inline unsigned hour(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_hour;
	}
	inline unsigned minute(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_min;
	}
	inline unsigned second(void)
	{
		struct tm* _tm=localtime(&built);
		return _tm->tm_sec;
	}
	inline double ubuntu_style_version(void)
	{
		struct tm* _tm=localtime(&built);
		return static_cast<double>(_tm->tm_year-100)+static_cast<double>(_tm->tm_mon+1)/100.;
	}
}

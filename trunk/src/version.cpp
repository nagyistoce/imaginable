// $Id: version.cpp 18 2010-03-07 22:49:04Z kuzma $

#include "version.hpp"

namespace version {

const unsigned major = 0;

const unsigned minor = 1;

const char* revision =
#include "version-svn.auto.inl"
;

const char* label ="-alpha"
#ifdef _DEBUG
"-debug"
#endif
;

const time_t built =
#include "version-build.auto.inl"
;

}

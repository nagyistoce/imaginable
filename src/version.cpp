// $Id: version.cpp 18 2010-03-07 22:49:04Z kuzma $

#include "version.hpp"

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

}

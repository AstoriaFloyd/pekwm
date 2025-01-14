//
// Debug.hh for pekwm
// Copyright (C) 2021 Claes Nästén <pekdon@gmail.com>
// Copyright (C) 2012-2013 Andreas Schlick <ioerror@lavabit.com>
//
// This program is licensed under the GNU GPL.
// See the LICENSE file for more information.
//

#ifndef _PEKWM_DEBUG_HH_
#define _PEKWM_DEBUG_HH_

#include "Compat.hh"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>

namespace Debug
{
	enum Level {
		LEVEL_ERR,
		LEVEL_WARN,
		LEVEL_INFO,
		LEVEL_DEBUG,
		LEVEL_TRACE
	};

	Level getLevel(const std::string& str);
	bool isLevel(Level level);
	Level getLevel(void);
	void setLevel(Level level);

	void doAction(const std::string& action);

	std::ostream& getStream(const char* prefix);
	std::ostream& getStream(const char* file, int line, const char* prefix);
	bool setLogFile(const std::string& path);
}

#define USER_INFO(M)				\
	Debug::getStream("") << M << std::endl;
#define USER_WARN(M)						\
	Debug::getStream("WARNING: ") << M << std::endl;

#define P_TRACE(M)							\
	if (Debug::isLevel(Debug::LEVEL_TRACE)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "TRACE:   ") \
			<< M << std::endl;				\
	}

#define P_DBG(M)							\
	if (Debug::isLevel(Debug::LEVEL_DEBUG)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "DEBUG:   ") \
			<< M << std::endl;				\
	}

#define P_LOG(M)							\
	if (Debug::isLevel(Debug::LEVEL_INFO)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "") << M << std::endl; \
	}

#define P_LOG_IF(C, M)							\
	if ((C) && Debug::isLevel(Debug::LEVEL_INFO)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "") << M << std::endl; \
	}

#define P_WARN(M)							\
	if (Debug::isLevel(Debug::LEVEL_WARN)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "WARNING: ") \
			<< M << std::endl;				\
	}

#define P_ERR(M)							\
	if (Debug::isLevel(Debug::LEVEL_ERR)) {				\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "ERROR:   ") \
			<< M << std::endl;				\
	}

#define P_ERR_IF(C, M)							\
	if ((C) && Debug::isLevel(Debug::LEVEL_ERR)) {			\
		Debug::getStream(__PRETTY_FUNCTION__, __LINE__, "ERROR:   ") \
			<< M << std::endl;				\
	}

#define FMT_HEX(V)				\
	"0x" << std::hex << (V) << std::dec

#endif // _PEKWM_DEBUG_HH_

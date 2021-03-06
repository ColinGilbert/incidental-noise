/*******************************************************************************
Copyright (c) 2005-2015 David Williams and Matthew Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*******************************************************************************/

#ifndef __PolyVox_ExceptionsImpl_H__
#define __PolyVox_ExceptionsImpl_H__

#include "PolyVox/Config.h"
#include "PolyVox/Exceptions.h"

#include "PolyVox/Impl/LoggingImpl.h" // Exceptions can log when they are thrown.

#include <cstdlib>  // For std::exit
#include <iostream> // For std::cerr
#include <sstream>
#include <csignal>

#ifdef POLYVOX_THROW_ENABLED

	namespace PolyVox
	{
		namespace Impl
		{
			template< typename ExceptionType, typename ... Args >
			void polyvox_throw(Args const& ... messageArgs)
			{
				std::string message = argListToString(messageArgs...);
#ifdef POLYVOX_LOG_ERROR_ENABLED
				getLoggerInstance()->logErrorMessage(message);
#endif
				throw ExceptionType(message);
			}

			template< typename ExceptionType, typename ... Args >
			void polyvox_throw_if(bool condition, Args const& ... messageArgs)
			{
				if (condition) { polyvox_throw<ExceptionType>(messageArgs...); }
			}
		}
	}

	#define POLYVOX_THROW(type, ...) PolyVox::Impl::polyvox_throw<type>(__VA_ARGS__)
	#define POLYVOX_THROW_IF(condition, type, ...) PolyVox::Impl::polyvox_throw_if<type>(condition, __VA_ARGS__)

#else

	// This stuff should possibly be in the 'Impl' namespace, but it may be complex and it's not
	// clear if this ability to disable throwing of exceptions is useful in the long term anyway.
	namespace PolyVox
	{
		typedef void(*ThrowHandler)(std::exception& e, const char* file, int line);

		inline void defaultThrowHandler(std::exception& e, const char* file, int line)
		{
			std::stringstream ss; \
				ss << "\n"; \
				ss << "    PolyVox exception thrown!"; \
				ss << "    ========================="; \
				ss << "    PolyVox has tried to throw an exception but it was built without support"; \
				ss << "    for exceptions. In this scenario PolyVox will call a 'throw handler'"; \
				ss << "    and this message is being printed by the default throw handler."; \
				ss << "\n"; \
				ss << "    If you don't want to enable exceptions then you should try to determine why"; \
				ss << "    this exception was thrown and make sure it doesn't happen again. If it was"; \
				ss << "    due to something like an invalid argument to a function then you should be"; \
				ss << "    able to fix it quite easily by validating parameters as appropriate. More"; \
				ss << "    complex exception scenarios (out of memory, etc) might be harder to fix and"; \
				ss << "    you should replace this default handler with something which is more"; \
				ss << "    meaningful to your users."; \
				ss << "\n"; \
				ss << "    Exception details"; \
				ss << "    -----------------"; \
				ss << "    Line:    " << line; \
				ss << "    File:    " << file; \
				ss << "    Message: " << e.what(); \
				ss << "\n"; \
				PolyVox::Impl::getLoggerInstance()->logFatalMessage(ss.str()); \
				POLYVOX_HALT(); \
		}

		inline ThrowHandler& getThrowHandlerInstance()
		{
			static ThrowHandler s_fThrowHandler = &defaultThrowHandler;
			return s_fThrowHandler;
		}

		inline ThrowHandler getThrowHandler()
		{
			return getThrowHandlerInstance();
		}

		inline void setThrowHandler(ThrowHandler fNewHandler)
		{
			getThrowHandlerInstance() = fNewHandler;
		}
	}

	#define POLYVOX_THROW_IF(condition, type, message) \
		/* We use the do...while(0) construct in our macros (for reasons see here: http://stackoverflow.com/a/154138) \
		but Visual Studio gives unhelpful 'conditional expression is constant' warnings. The recommended solution \
		(http://stackoverflow.com/a/1946485) is to disable these warnings. */ \
		POLYVOX_MSC_WARNING_PUSH \
		POLYVOX_DISABLE_MSC_WARNING(4127) \
		do \
		{ \
			if ((condition)) \
			{ \
				std::stringstream ss; \
				ss << message; \
				PolyVox::Impl::getLoggerInstance()->logErrorMessage(ss.str()); \
				type except = (type)(ss.str()); \
				getThrowHandler()((except), __FILE__, __LINE__); \
			} \
		} while(0) \
		POLYVOX_MSC_WARNING_POP

	#define POLYVOX_THROW(type, message) \
		/* We use the do...while(0) construct in our macros (for reasons see here: http://stackoverflow.com/a/154138) \
		but Visual Studio gives unhelpful 'conditional expression is constant' warnings. The recommended solution \
		(http://stackoverflow.com/a/1946485) is to disable these warnings. */ \
		POLYVOX_MSC_WARNING_PUSH \
		POLYVOX_DISABLE_MSC_WARNING(4127) \
		do \
		{ \
			std::stringstream ss; \
			ss << message; \
			PolyVox::Impl::getLoggerInstance()->logErrorMessage(ss.str()); \
			type except = (type)(ss.str()); \
			getThrowHandler()((except), __FILE__, __LINE__); \
		} while(0) \
		POLYVOX_MSC_WARNING_POP

#endif

#endif //__PolyVox_ExceptionsImpl_H__

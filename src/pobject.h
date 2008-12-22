/*
 * Copyright 2005, 2008 Fernando Silveira <fsilveira@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Fernando Silveira.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if !defined(_POBJECT_H_)
#define _POBJECT_H_

#define bool jboolean
#define TRUE JNI_TRUE
#define FALSE JNI_FALSE

/*
#if defined(DEBUG) && !defined(NDEBUG)

#include <assert.h>

#define POBJECT_ASSERT_RETURN(expr, retval) assert(expr)
#define POBJECT_ASSERT(expr) assert(expr)
#elif defined(DEBUG)
#define POBJECT_ASSERT(expr) do {					\
	if (!(expr)) {							\
		fprintf(stderr, "PJNI: Erro interno em %s:%d: \"%s\".",	\
		    __FILE__, __LINE__, #expr);				\
		return;							\
	}								\
} while (0)
#define POBJECT_ASSERT_RETURN(expr, retval) do {			\
	if (!(expr)) {							\
		fprintf(stderr, "PJNI: Erro interno em %s:%d: \"%s\".",	\
		    __FILE__, __LINE__, #expr);				\
		return retval;						\
	}								\
} while (0)
#else
#define POBJECT_ASSERT(expr) do {					\
	if (!(expr)) {							\
		return;							\
	}								\
} while (0)
#define POBJECT_ASSERT_RETURN(expr, retval) do {			\
	if (!(expr)) {							\
		return retval;						\
	}								\
} while (0)
#endif
*/

#if defined(DEBUG)

#define PJNI_ASSERT_RETURN(cond, ret) do {				\
	if (!(cond)) {							\
		fprintf(stderr, "%s:%d: %s: Assertion `%s' failed. "	\
		    "Returning `%s'.\n",				\
		    __FILE__, __LINE__, __FUNCTION__, #cond, #ret);	\
		fflush(stderr);						\
		return (ret);						\
	}								\
} while (0)

#define PJNI_ASSERT_RETURN_VOID(cond)  do {				\
	if (!(cond)) {							\
		fprintf(stderr, "%s:%d: %s: Assertion `%s' failed. "	\
		    "Returning.\n",					\
		    __FILE__, __LINE__, __FUNCTION__, #cond);		\
		fflush(stderr);						\
		return;							\
	}								\
} while (0)

#define POBJECT_WARN_ENOSYS() do {					\
	printf("%s:%d: POBJECT_ERROR_ENOSYS\n", __FILE__, __LINE__);	\
	fflush(stdout);							\
} while (0)

#else

#define PJNI_ASSERT_RETURN(cond, ret) do {				\
	if (!(cond))							\
		return (ret);						\
} while (0)

#define PJNI_ASSERT_RETURN_VOID(cond)  do {				\
	if (!(cond))							\
		return;							\
} while (0)

#define POBJECT_WARN_ENOSYS() do { } while (0)

#endif

#endif /* !defined(_POBJECT_H_) */

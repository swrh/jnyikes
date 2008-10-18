#ifndef POBJECT_H_
#define POBJECT_H_

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

#endif /* POBJECT_H_ */

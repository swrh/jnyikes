/*
 * Copyright 2005-2012 Fernando Silveira <fsilveira@gmail.com>
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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <jni.h>

#include "jyo.h"

#define JY_DEBUG_ERROR
/*
#define JY_DEBUG_VERBOSE
*/
#if 1
#define DEBUG_BOOL(x) do { fprintf(stderr, "%s:%d: %s = %s;\n", __FILE__, __LINE__, #x, x == JY_FALSE ? "JY_FALSE" : "JY_TRUE"); fflush(stderr); } while (0)
#define DEBUG_BYTE(x) do { fprintf(stderr, "%s:%d: %s = %d;\n", __FILE__, __LINE__, #x, (int) (char) x); fflush(stderr); } while (0)
#define DEBUG_CHAR(x) do { fprintf(stderr, "%s:%d: %s = '%c';\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_SHORT(x) do { fprintf(stderr, "%s:%d: %s = %hd;\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_INT(x) do { fprintf(stderr, "%s:%d: %s = %d;\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_LONG(x) do { fprintf(stderr, "%s:%d: %s = %ld;\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_DOUBLE(x) do { fprintf(stderr, "%s:%d: %s = %e;\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_STR(x) do { fprintf(stderr, "%s:%d: %s = \"%s\";\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#define DEBUG_PTR(x) do { fprintf(stderr, "%s:%d: %s = %p;\n", __FILE__, __LINE__, #x, x); fflush(stderr); } while (0)
#else
#define DEBUG_BOOL(x) do { } while (0)
#define DEBUG_BYTE(x) do { } while (0)
#define DEBUG_CHAR(x) do { } while (0)
#define DEBUG_SHORT(x) do { } while (0)
#define DEBUG_INT(x) do { } while (0)
#define DEBUG_LONG(x) do { } while (0)
#define DEBUG_DOUBLE(x) do { } while (0)
#define DEBUG_STR(x) do { } while (0)
#define DEBUG_PTR(x) do { } while (0)
#endif

static const char *g_str_clazz_string = "java/lang/String";
static const char *g_str_clazz_object = "java/lang/Object";

struct st_method_ll {
	struct st_llist ll;
	char *name;
	char *sign;
	int rettype;
	jmethodID jmid;
};

static void jyo_free_method_ll(struct st_method_ll **mll);

static void
jyo_free_method_ll(struct st_method_ll **mll)
{
	JY_ASSERT_RETURN_VOID(mll != NULL);

	while (*mll != NULL) {
		if ((*mll)->name != NULL)
			free((*mll)->name);
		if ((*mll)->sign != NULL)
			free((*mll)->sign);
		lldel((void **)mll, *mll);
	}
}

/*
 * XXX Funções de liberação de memória.
 */

/** XXX Libera uma estrutura de propriedade. */
static void
jyo_property_free(struct st_jyo_property *p)
{
	JY_ASSERT_RETURN_VOID(p != NULL);

	if (p->method_name != NULL) {
		free(p->method_name);
		p->method_name = NULL;
	}

	if (p->data != NULL) {
		free(p->data);
		p->data = NULL;
	}
}

/** XXX Libera a lista ligada de propriedades. */
static void
jyo_property_ll_free(struct st_jyo_property_ll **head_ll)
{
	struct st_jyo_property_ll *p_ll;

	JY_ASSERT_RETURN_VOID(head_ll != NULL);

	if (*head_ll == NULL)
		return;

	/* XXX Libera o conteudo de cada "st_property". */
	for (p_ll = *head_ll; p_ll != NULL;
	    p_ll = (struct st_jyo_property_ll *)p_ll->ll.next) {
		jyo_property_free(&p_ll->st);
	}

	/* XXX Destroi a lista ligada. */
	lldestroy((void **)head_ll);
}

/** XXX Libera toda a estrutura "st_jyo". */
void
jyo_free(struct st_jyo *p)
{
	JY_ASSERT_RETURN_VOID(p != NULL);

	if (p->clazz != NULL) {
		free(p->clazz);
		p->clazz = NULL;
	}

	jyo_property_ll_free(&p->properties);

	p->error = JY_ESUCCESS;
}

/** XXX Inicializa uma estrutura "st_jyo". */
int
jyo_init(struct st_jyo *p, char *clazz)
{
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);

	memset(p, 0, sizeof(struct st_jyo));

	if (clazz != NULL) {
		p->clazz = strdup(clazz);
		if (p->clazz == NULL)
			return JY_EENOMEM;
	}

	return JY_ESUCCESS;
}


/*
 * XXX Funções de uso geral.
 */

/** XXX Retorna o tamanho de um tipo de dado "e_jyo_type". */
static size_t
jyo_get_type_size(enum e_jyo_type t)
{
	switch (t) {
		case JYO_TBOOLEAN:
			return sizeof(jy_bool);
		case JYO_TBYTE:
			return sizeof(char);
		case JYO_TCHAR:
			return sizeof(char);
		case JYO_TSHORT:
			return sizeof(short);
		case JYO_TINT:
			return sizeof(int);
		case JYO_TLONG:
			return sizeof(long);
		case JYO_TFLOAT:
			return sizeof(float);
		case JYO_TDOUBLE:
			return sizeof(double);
			/*
		case JYO_TUINT:
			return sizeof(unsigned int);
		case JYO_TULONG:
			return sizeof(unsigned long);
			*/
		case JYO_TJYO:
			return sizeof(struct st_jyo);
		case JYO_TSTRING:
		default:
			return (size_t)JY_EEINVAL;
	}
}

/** XXX Retorna os nomes dos tipos. */
const char *
jyo_get_type_name(enum e_jyo_type t)
{
#define CASE_RETURN(x) case x: return #x
	switch (t) {
		CASE_RETURN(JYO_TDOUBLE);
		CASE_RETURN(JYO_TFLOAT);
		CASE_RETURN(JYO_TINT);
		CASE_RETURN(JYO_TLONG);
		CASE_RETURN(JYO_TJYO);
		CASE_RETURN(JYO_TSTRING);
		CASE_RETURN(JYO_TUINT);
		CASE_RETURN(JYO_TULONG);
		CASE_RETURN(JYO_TBOOLEAN);
		CASE_RETURN(JYO_TBYTE);
		CASE_RETURN(JYO_TCHAR);
		CASE_RETURN(JYO_TSHORT);
		CASE_RETURN(JYO_TVOID);
		CASE_RETURN(JYO_TJCLASS);
		default:
			return NULL;
	}
#undef CASE_NAME
}

/*
 * XXX Adiciona propriedade "method name" na lista "p->proprierties", com o dado "orig_data"
 */
int
jyo_set_property(struct st_jyo *p, const char *method_name, enum e_jyo_type data_type, const void *orig_data)
{
	struct st_jyo_property_ll *p_ll;
	size_t data_size;
	void *data;

	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(method_name != NULL, JY_EEINVAL);

	if (jyo_error(p) != JY_ESUCCESS)
		return JY_EEINVAL;

	/*
	 * XXX verifica o tamanho do orig_data
	 */
	if (data_type == JYO_TSTRING) {
		if (orig_data != NULL) {
			data_size = strlen((char *)orig_data) + 1;
DEBUG_STR(method_name);
DEBUG_STR((char *)orig_data);
		} else
			data_size = 0;
	} else {
		if (orig_data != NULL) {
			data_size = jyo_get_type_size(data_type);
			JY_ASSERT_RETURN(data_size != JY_EEINVAL, JY_EEINVAL);
DEBUG_STR(method_name);
		} else
			data_size = 0;
	}

	if (data_size > 0 ) {
/*	JY_ASSERT_RETURN(data_size > 0, JY_EEINVAL);*/
		data = malloc(data_size);
		if (data == NULL) {
			p->error = JY_EENOMEM;
			return JY_EENOMEM;
		}
		memcpy(data, orig_data, data_size);
	} else
		data = NULL;

	p_ll = malloc(sizeof(struct st_jyo_property_ll));
	if (p_ll == NULL) {
		if (data_size > 0 )
			free(data);

		p->error = JY_EENOMEM;
		return JY_EENOMEM;
	}

	memset(p_ll, 0, sizeof(struct st_jyo_property_ll));

	p_ll->st.data_type = data_type;
	p_ll->st.data = data;

	p_ll->st.method_name = strdup(method_name);
	if (p_ll->st.method_name == NULL) {
		jyo_property_ll_free(&p_ll);

		p->error = JY_EENOMEM;
		return JY_EENOMEM;
	}

	llappend((void *)&p->properties, p_ll);

	return JY_ESUCCESS;
}

/** TODO COMMENT */
static int
jyo_get_static_mid(JNIEnv *jenv, const char *clazz, const char *method, char *sig, jclass *jcls, jmethodID *jmid)
{
	JY_ASSERT_RETURN(clazz != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(method != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(sig != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jmid != NULL, JY_EEINVAL);

	*jcls = (*jenv)->FindClass(jenv, clazz);
	if ((*jcls == NULL) || ((*jenv)->ExceptionCheck(jenv))) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not find class \"%s\".\n", clazz);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (*jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, *jcls);

		*jcls = NULL;
		*jmid = NULL;

		return JY_ENOJCLASS;
	}

	*jmid = (*jenv)->GetStaticMethodID(jenv, *jcls, method, sig);
	if ((*jmid == NULL) || ((*jenv)->ExceptionCheck(jenv))) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not find static method \"%s\" with signature \"%s\" for class \"%s\".\n", method, sig, clazz);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, *jcls);
		*jcls = NULL;
		*jmid = NULL;

		return JY_ENOTFOUND;
	}

	return JY_ESUCCESS;
}

/** TODO COMMENT */
static char *
jyo_get_method_signature(enum e_jyo_type type_ret, void *type_ret_detail, enum e_jyo_type type_param, const char *type_param_detail)
{
#define SWITCH_TYPE_CAT_s(type, param) do {				\
	switch (type) {							\
		case JYO_TBOOLEAN:				\
			strcat(s, "Z");					\
			break;						\
		case JYO_TBYTE:					\
			strcat(s, "B");					\
			break;						\
		case JYO_TCHAR:					\
			strcat(s, "C");					\
			break;						\
		case JYO_TSHORT:				\
			strcat(s, "S");					\
			break;						\
		case JYO_TUINT:					\
		case JYO_TINT:					\
			strcat(s, "I");					\
			break;						\
		case JYO_TULONG:				\
		case JYO_TLONG:					\
			strcat(s, "J");					\
			break;						\
		case JYO_TFLOAT:				\
			strcat(s, "F");					\
			break;						\
		case JYO_TDOUBLE:				\
			strcat(s, "D");					\
			break;						\
		case JYO_TVOID:					\
			strcat(s, "V");					\
			break;						\
		case JYO_TSTRING:				\
			strcat(s, "L");					\
			strcat(s, g_str_clazz_string);			\
			strcat(s, ";");					\
			break;						\
		case JYO_TJYO:				\
			strcat(s, "L");					\
			strcat(s, ((struct st_jyo *)param)->clazz);	\
			strcat(s, ";");					\
			break;						\
		case JYO_TJCLASS:				\
			strcat(s, "L");					\
			strcat(s, (char *)param);			\
			strcat(s, ";");					\
			break;						\
	}								\
} while (0)

	char *s;
	size_t slen;

	/* XXX Verifica os parâmetros passados. */
	if (type_param == JYO_TJCLASS)
		JY_ASSERT_RETURN(type_param_detail != NULL, NULL);
	else if (type_param != JYO_TSTRING)
		JY_ASSERT_RETURN(jyo_get_type_size(type_param) > 0, NULL);

	if (type_ret == JYO_TJCLASS)
		JY_ASSERT_RETURN(type_ret_detail != NULL, NULL);
	else if (type_ret != JYO_TSTRING)
		JY_ASSERT_RETURN(jyo_get_type_size(type_ret) > 0, NULL);

	slen = 1;	/* EOS - '\000' */

	if (type_ret == JYO_TJCLASS)		/* Lpath/Class; */
		slen += strlen(type_ret_detail) + 2;
	else if (type_ret == JYO_TSTRING)	/* Lpath/Class; */
		slen += strlen(g_str_clazz_string) + 2;
	else						/* T */
		slen += 1;

	if (type_param == JYO_TJCLASS)		/* Lpath/Class; */
		slen += strlen(type_param_detail) + 2;
	else if (type_param == JYO_TSTRING)	/* Lpath/Class; */
		slen += strlen(g_str_clazz_string) + 2;
	else if (type_param == JYO_TJYO)	/* Lpath/Class; */
		slen += strlen(((struct st_jyo *)type_param_detail)->clazz) + 2;
	else						/* T */
		slen += 1;

	slen += 2;	/* () */

	s = malloc(slen);
	if (s == NULL)
		return NULL;
	*s = '\000';

	strcat(s, "(");

	SWITCH_TYPE_CAT_s(type_param, type_param_detail);
	strcat(s, ")");
	SWITCH_TYPE_CAT_s(type_ret, type_ret_detail);

	return s;

#undef SWITCH_TYPE_CAT_s
}

/** XXX TODO Envia a estrutura "st_jyo" à máquina virtual. */
int
jyo_send(JNIEnv *jenv, struct st_jyo *p, const char *clazz, const char *method)
{
	int ret;
	jobject jobj;
	jclass jcls;
	jmethodID jmid;
	char *sig;
	jboolean jret;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(clazz != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(method != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p->clazz != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p->error == JY_ESUCCESS, p->error);

	sig = jyo_get_method_signature(JYO_TBOOLEAN, NULL, JYO_TJCLASS, p->clazz);
	if (sig == NULL)
		return JY_EENOMEM;

	ret = jyo_get_static_mid(jenv, clazz, method, sig, &jcls, &jmid);

	free(sig);
	if (ret != JY_ESUCCESS)
		return ret;

	ret = jyo_p2j(jenv, p, &jobj);
	if (ret != JY_ESUCCESS) {
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return ret;
	}

	/* XXX 20060408 - fsilveira - reparo: era CallStaticVoidMethod() (errado) */
	jret = (*jenv)->CallStaticBooleanMethod(jenv, jcls, jmid, jobj);
	(*jenv)->DeleteLocalRef(jenv, jcls);
	(*jenv)->DeleteLocalRef(jenv, jobj);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	return jret == JNI_FALSE ? JY_EEXCEPTION : JY_ESUCCESS;
}

/** XXX Verifica o estado de erro ocorrido na estrutura "st_jyo". */
int
jyo_error(struct st_jyo *p)
{
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);

	return p->error;
}

/** XXX TODO COMMENT */
static jobject
jyo_new_jobject(JNIEnv *jenv, const char *clazz)
{
	jclass jcls;
	jmethodID jmid;
	jobject jobj;

	JY_ASSERT_RETURN(jenv != NULL, NULL);
	JY_ASSERT_RETURN(clazz != NULL, NULL);

	jcls = (*jenv)->FindClass(jenv, clazz);
	if ((jcls == NULL) || ((*jenv)->ExceptionCheck(jenv))) {
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
			fflush(stderr);
		}

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return NULL;
	}

	jmid = (*jenv)->GetMethodID(jenv, jcls, "<init>", "()V");
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return NULL;
	}

	jobj = (*jenv)->NewObject(jenv, jcls, jmid);
	(*jenv)->DeleteLocalRef(jenv, jcls);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return NULL;
	}

	return jobj;
}

/** XXX TODO COMMENT */
static int
jyo_fill_jobject_property(JNIEnv *jenv, jobject j, struct st_jyo *p, struct st_jyo_property *pp)
{
	jclass jcls;
	jmethodID jmid;
	jobject new_jobj;
	jstring new_jstr;
	char *sig;
	int ret, rettype;

	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return JY_EEINVAL;
	}

#define JY_GET_METHOD(jenv, jcls, jmid, set, sig, tret, dret, tpar, dpar) \
do {									\
	if ((*jenv)->ExceptionCheck(jenv))				\
		(*jenv)->ExceptionClear(jenv);				\
	if (sig != NULL)						\
		free(sig);						\
									\
	sig = jyo_get_method_signature(tret, dret, tpar, dpar);	\
	JY_ASSERT_RETURN(sig != NULL, JY_EEINVAL);		\
									\
	jmid = (*jenv)->GetMethodID(jenv, jcls, set, sig);		\
} while (0)

	sig = NULL;
	jmid = 0;

	/* XXX Tenta achar o metodo com tipo de retorno "void". */
	if (jmid == 0) {
		JY_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, JYO_TVOID, NULL, pp->data_type, pp->data);
		rettype = JYO_TVOID;
	}

	/* XXX Tenta achar o metodo com tipo de retorno "boolean". */
	if (jmid == 0) {
		JY_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, JYO_TBOOLEAN, NULL, pp->data_type, pp->data);
		rettype = JYO_TBOOLEAN;
	}

	if ((jmid == 0) && (pp->data_type == JYO_TJYO)) {
		/* Tenta achar o metodo com tipo de parametro java/lang/Object e retorno "void". */
		if (jmid == 0) {
			JY_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, JYO_TVOID, NULL, JYO_TJCLASS, g_str_clazz_object);
			rettype = JYO_TVOID;
		}

		/* Tenta achar o metodo com tipo de parametro java/lang/Object e retorno "boolean". */
		if (jmid == 0) {
			JY_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, JYO_TBOOLEAN, NULL, JYO_TJCLASS, g_str_clazz_object);
			rettype = JYO_TBOOLEAN;
		}
	}

#if 0
	sig = jyo_get_method_signature(JYO_TVOID, NULL, pp->data_type, pp->data);
	JY_ASSERT_RETURN(sig != NULL, JY_EEINVAL);

	jmid = (*jenv)->GetMethodID(jenv, jcls, pp->method_name, sig);
	if ((jmid == 0) && (pp->data_type == JYO_TJYO)) {
		if ((*jenv)->ExceptionCheck(jenv))
			(*jenv)->ExceptionClear(jenv);

		free(sig);
		sig = jyo_get_method_signature(JYO_TVOID, NULL, JYO_TJCLASS, g_str_clazz_object);
		JY_ASSERT_RETURN(sig != NULL, JY_EEINVAL);

		jmid = (*jenv)->GetMethodID(jenv, jcls, pp->method_name, sig);
	}
#endif

	if ((jmid == 0) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not find method \"%s\" with signature \"%s\" of an object of class \"%s\" at \"%s:%d\".\n", pp->method_name, sig, p->clazz, __FILE__, __LINE__);
#endif

		free(sig);

		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return JY_ENOTFOUND;
	}

	if (pp->data_type != JYO_TVOID)
		JY_ASSERT_RETURN(pp->data != NULL, JY_EEINVAL);

	switch (pp->data_type) {
		case JYO_TBOOLEAN:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jboolean) *((jboolean *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jboolean) *((jboolean *)pp->data));
			break;
		case JYO_TBYTE:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jbyte) *((char *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jbyte) *((char *)pp->data));
			break;
		case JYO_TCHAR:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jchar) *((char *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jchar) *((char *)pp->data));
			break;
		case JYO_TSHORT:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jshort) *((short *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jshort) *((short *)pp->data));
			break;
		case JYO_TINT:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jint) *((int *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jint) *((int *)pp->data));
			break;
		case JYO_TLONG:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jlong) *((long *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jlong) *((long *)pp->data));
			break;
		case JYO_TFLOAT:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jfloat) *((float *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jfloat) *((float *)pp->data));
			break;
		case JYO_TDOUBLE:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jdouble) *((double *)pp->data));
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jdouble) *((double *)pp->data));
			break;
		case JYO_TVOID:
			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid);
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid);
			break;
		case JYO_TJYO:
			ret = jyo_p2j(jenv, (struct st_jyo *)pp->data, &new_jobj);
			if (ret != JY_ESUCCESS) {
#ifdef JY_DEBUG_ERROR
				fprintf(stderr, "%s while converting an \"st_jyo\" structure to \"jobject\" at \"%s:%d\".\n", jy_strerror(ret), __FILE__, __LINE__);
				fflush(stderr);
#endif
				free(sig);
				(*jenv)->DeleteLocalRef(jenv, jcls);
				return ret;
			}

			if (rettype == JYO_TVOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, new_jobj);
			else if (rettype == JYO_TBOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, new_jobj);
			(*jenv)->DeleteLocalRef(jenv, new_jobj);

			break;
		case JYO_TSTRING:
			new_jstr = (*jenv)->NewStringUTF(jenv, (char *)pp->data);
			if (new_jstr != NULL) {
				if (rettype == JYO_TVOID)
					(*jenv)->CallVoidMethod(jenv, j, jmid, new_jstr);
				else if (rettype == JYO_TBOOLEAN)
					(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, new_jstr);
				(*jenv)->DeleteLocalRef(jenv, new_jstr);
				fflush(stderr);
#ifdef JY_DEBUG_ERROR
			} else {
				fprintf(stderr, "Error converting a C string to Java String at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
			}

			break;

		case JYO_TJCLASS:
		default:
			JY_WARN_ENOSYS();
			/*(*jenv)->DeleteLocalRef(jenv, jcls);
			return JY_EENOSYS;*/
			break;
	}

	if ((*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Exception ocurred while calling method \"%s\" with signature \"%s\" of an object of class \"%s\".\n", pp->method_name, sig, p->clazz);
#endif
		free(sig);
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return JY_EEXCEPTION;
	}

	free(sig);

	return JY_ESUCCESS;
}

/** TODO COMMENT */
static int
jyo_fill_jobject(JNIEnv *jenv, jobject j, struct st_jyo *p)
{
	struct st_jyo_property_ll *p_ll;
	jclass jcls;
	int ret;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);

	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return JY_EEINVAL;
	}

	for (p_ll = p->properties; p_ll != NULL;
	    p_ll = (struct st_jyo_property_ll *)p_ll->ll.next) {
#ifdef JY_DEBUG_VERBOSE
		printf("DEBUG: configurando propriedade \"%s\" do objeto de classe \"%s\".\n", p_ll->st.method_name, p->clazz);
		fflush(stdout);
#endif

		ret = jyo_fill_jobject_property(jenv, j, p, &p_ll->st);
		if (ret != JY_ESUCCESS) {
			(*jenv)->DeleteLocalRef(jenv, jcls);
			return ret;
		}
	}

	(*jenv)->DeleteLocalRef(jenv, jcls);

	return JY_ESUCCESS;
}

/** XXX TODO Converte a estrutura "st_jyo" em um objeto Java. */
int
jyo_p2j(JNIEnv *jenv, struct st_jyo *p, jobject *j)
{
	int ret;

	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	memset(j, 0, sizeof(jobject));
	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p->clazz != NULL, JY_EEINVAL);

	if (jyo_error(p) != JY_ESUCCESS)
		return JY_EEINVAL;

	*j = jyo_new_jobject(jenv, p->clazz);
	if (*j == NULL)
		return JY_ENOJCLASS;

	ret = jyo_fill_jobject(jenv, *j, p);
	if (ret != JY_ESUCCESS) {
		(*jenv)->DeleteLocalRef(jenv, *j);
		*j = NULL;
		return ret;
	}

	return JY_ESUCCESS;
}

static char *
jyo_get_jclass_name(JNIEnv *jenv, jclass jcls, jobject strObj)
{
	jmethodID midGetClass;
	jobject clsObj;
	jclass jCls;
	jmethodID midGetFields;
	jstring _name;

	const char *str;
	char *ret;

	JY_ASSERT_RETURN(jenv != NULL, NULL);
	JY_ASSERT_RETURN(jcls != NULL, NULL);

/*	strObj = (*jenv)->AllocObject(jenv, jcls);
	JY_ASSERT_RETURN(strObj != NULL, NULL);*/

	midGetClass = (*jenv)->GetMethodID(jenv, jcls, "getClass", "()Ljava/lang/Class;");
	JY_ASSERT_RETURN(midGetClass != NULL, NULL);

	clsObj = (*jenv)->CallObjectMethod(jenv, strObj, midGetClass);
/*	(*jenv)->DeleteLocalRef(jenv, strObj);*/
	JY_ASSERT_RETURN(clsObj != NULL, NULL);

	jCls = (*jenv)->GetObjectClass(jenv, clsObj);
	JY_ASSERT_RETURN(jCls != NULL, NULL);

	midGetFields = (*jenv)->GetMethodID(jenv, jCls, "getName", "()Ljava/lang/String;");
	(*jenv)->DeleteLocalRef(jenv, jCls);
	JY_ASSERT_RETURN(midGetFields != NULL, NULL);

	_name = (jstring)(*jenv)->CallObjectMethod(jenv, clsObj, midGetFields);
	(*jenv)->DeleteLocalRef(jenv, clsObj);
	JY_ASSERT_RETURN(_name != NULL, NULL);

	str = (*jenv)->GetStringUTFChars(jenv, _name, 0);
	JY_ASSERT_RETURN(str != NULL, NULL);

	ret = strdup(str);

	(*jenv)->ReleaseStringUTFChars(jenv, _name, str);

	(*jenv)->DeleteLocalRef(jenv, _name);

	return ret;
}

static const char *
jyo_method_str_get_param(const char *str)
{
	const char *p;

	JY_ASSERT_RETURN(str != NULL, NULL);

	p = strchr(str, (int) '(');
	JY_ASSERT_RETURN(p != NULL, NULL);
	JY_ASSERT_RETURN(p != str, NULL);
	p++;

	return p;
}

static const char *
jyo_method_str_get_name(const char *str)
{
	const char *pparam, *pname, *p;

	JY_ASSERT_RETURN(str != NULL, NULL);

	pparam = jyo_method_str_get_param(str);
	JY_ASSERT_RETURN(pparam != NULL, NULL);

	pname = str;
	for (;;) {
		p = strchr(pname, (int) ' ');
		if ((p == NULL) || (++p >= pparam))
			return pname;
		pname = p;
	}

	return pname;
}

static const char *
jyo_method_str_get_type(const char *str)
{
	JY_ASSERT_RETURN(str != NULL, NULL);

	if (strlen(str) > 7)
		if (strncmp(str, "public ", 7) == 0)
			return str + 7;
	return NULL;
}

static int
jyo_str_get_type(const char *str)
{
	int len;

	JY_ASSERT_RETURN(str != NULL, JY_EEINVAL);

	if (strchr(str, ' ') != NULL)
		len = (int) (strchr(str, ' ') - str);
	else
		len = strlen(str);

#define JY_STRING_TYPE_BOOLEAN "boolean"
#define JY_STRING_TYPE_BOOLEAN_LENGTH 7
#define JY_STRING_TYPE_BYTE "byte"
#define JY_STRING_TYPE_BYTE_LENGTH 4
#define JY_STRING_TYPE_CHAR "char"
#define JY_STRING_TYPE_CHAR_LENGTH 4
#define JY_STRING_TYPE_SHORT "short"
#define JY_STRING_TYPE_SHORT_LENGTH 5
#define JY_STRING_TYPE_INT "int"
#define JY_STRING_TYPE_INT_LENGTH 3
#define JY_STRING_TYPE_LONG "long"
#define JY_STRING_TYPE_LONG_LENGTH 4
#define JY_STRING_TYPE_FLOAT "float"
#define JY_STRING_TYPE_FLOAT_LENGTH 5
#define JY_STRING_TYPE_DOUBLE "double"
#define JY_STRING_TYPE_DOUBLE_LENGTH 6
#define JY_STRING_TYPE_VOID "void"
#define JY_STRING_TYPE_VOID_LENGTH 4
#define JY_STRING_TYPE_STRING "java.lang.String"
#define JY_STRING_TYPE_STRING_LENGTH 16

#define ISTYPE(x) (strncmp(str, JY_STRING_TYPE_##x, len < JY_STRING_TYPE_##x##_LENGTH ? len : JY_STRING_TYPE_##x##_LENGTH) == 0)

	if (ISTYPE(BOOLEAN))
		return JYO_TBOOLEAN;
	else if (ISTYPE(BYTE))
		return JYO_TBYTE;
	else if (ISTYPE(CHAR))
		return JYO_TCHAR;
	else if (ISTYPE(SHORT))
		return JYO_TSHORT;
	else if (ISTYPE(INT))
		return JYO_TINT;
	else if (ISTYPE(LONG))
		return JYO_TLONG;
	else if (ISTYPE(FLOAT))
		return JYO_TFLOAT;
	else if (ISTYPE(DOUBLE))
		return JYO_TDOUBLE;
	else if (ISTYPE(VOID))
		return JYO_TVOID;
	else if (ISTYPE(STRING))
		return JYO_TSTRING;

	return JY_EEINVAL;
}

static const char *
jyo_java_basename(const char *str)
{
	const char *pend, *p0, *p1, *p2;

	if (strchr(str, '.') == NULL)
		return str;

	p0 = strchr(str, '(');
	p1 = strchr(str, ',');
	p2 = strchr(str, ' ');

	pend = p0;
	if (p1 > pend)
		pend = p1;
	if (p2 > pend)
		pend = p2;
	if (pend == NULL)
		pend = str + strlen(str);

	p0 = str;
	for (;;) {
		p1 = strchr(p0, (int) '.');
		if ((p1 == NULL) || (++p1 >= pend))
			return p0;
		p0 = p1;
	}

	return p0;
}

static void
jyo_java_convert_str(char *str, char o, char d)
{
	char *p;

	JY_ASSERT_RETURN_VOID(str != NULL);

	for (p = strchr(str, (int)o); p != NULL; p = strchr(p, (int)o))
		*p = d;
}

static struct st_method_ll *
jyo_method_str_to_m(const char *str)
{
	const char *pparam, *pname, *ptype;
	char *temp;
	struct st_method_ll *m;

	JY_ASSERT_RETURN(str != NULL, NULL);

	pparam = jyo_method_str_get_param(str);
	JY_ASSERT_RETURN(pparam != NULL, NULL);

	if (pparam[0] != ')') {
		JY_WARN_ENOSYS();
		return NULL;
	}

	m = malloc(sizeof(struct st_method_ll));
	JY_ASSERT_RETURN(m != NULL, NULL);
	memset(m, 0, sizeof(struct st_method_ll));

	pname = jyo_method_str_get_name(str);
	JY_ASSERT_RETURN(pname != NULL, NULL);

	pname = jyo_java_basename(pname);

	ptype = jyo_method_str_get_type(str);
	JY_ASSERT_RETURN(ptype != NULL, NULL);

	m->sign = malloc(strlen(str) + 1); /* FIXME */
	JY_ASSERT_RETURN(m->sign != NULL, NULL);
	*m->sign = '\000';

	strcat(m->sign, "(");
	strcat(m->sign, ")");

#define CATTYPE(t, m, cat) case JYO_T##t: strcat(m->sign, cat); m->rettype = JYO_T##t; break
	switch (jyo_str_get_type(ptype)) {
		CATTYPE(BOOLEAN, m, "Z");
		CATTYPE(BYTE, m, "B");
		CATTYPE(CHAR, m, "C");
		CATTYPE(SHORT, m, "S");
		CATTYPE(INT, m, "I");
		CATTYPE(LONG, m, "J");
		CATTYPE(FLOAT, m, "F");
		CATTYPE(DOUBLE, m, "D");
		/* CATTYPE(VOID, m, "V"); */
		CATTYPE(STRING, m, "Ljava/lang/String;");

		default:
			strcat(m->sign, "L");
			strncat(m->sign, ptype, strchr(ptype, ' ') - ptype);
			strcat(m->sign, ";");
			m->rettype = JYO_TJCLASS;
			break;
	}

	/* FIXME */
	temp = strdup(m->sign);
	if (temp != NULL) {
		free(m->sign);
		m->sign = temp;
	}

	m->name = malloc(pparam - 1 - pname + 1); /* FIXME */
	JY_ASSERT_RETURN(m->name != NULL, NULL);
	*m->name = '\000';
	strncat(m->name, pname, pparam - 1 - pname);

	return m;
}

static jy_bool
jyo_method_str_is_from_java(const char *str)
{
	const char *pname;

	JY_ASSERT_RETURN(str != NULL, JY_FALSE);

	pname = jyo_method_str_get_name(str);
	JY_ASSERT_RETURN(pname != NULL, JY_FALSE);

	if ((strncmp(pname, "java.", 5) == 0) || (strncmp(pname, "javax.", 6) == 0))
		return JY_TRUE;

	return JY_FALSE;
}

static jy_bool
jyo_method_str_void_param(const char *str)
{
	const char *pparam;

	JY_ASSERT_RETURN(str != NULL, JY_FALSE);

	pparam = jyo_method_str_get_param(str);
	JY_ASSERT_RETURN(pparam != NULL, JY_FALSE);

	if (pparam[0] == ')')
		return JY_TRUE;
	return JY_FALSE;
}

static jy_bool
jyo_method_str_void_return(const char *str)
{
	const char *ptype;

	JY_ASSERT_RETURN(str != NULL, JY_FALSE);

	ptype = jyo_method_str_get_type(str);
	JY_ASSERT_RETURN(ptype != NULL, JY_FALSE);

	if (strncmp(ptype, "void ", 5) == 0)
		return JY_TRUE;
	return JY_FALSE;
}

static int
jyo_get_method_sign_list(JNIEnv *jenv, jclass cls, struct st_method_ll **mll)
{
	jobject strObj;
	jmethodID midGetClass;
	jobject clsObj;
	jclass jCls;
	jmethodID midGetFields;
	jobjectArray jobjArray;
	jsize len;
	jsize i;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(cls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(mll != NULL, JY_EEINVAL);

	*mll = NULL;

#define LAME_ASSERT(x) do {						\
	if (!(x)) {							\
		fprintf(stderr, "%s:%d: LAME_ASSERT\n",			\
		    __FILE__, __LINE__);				\
		return JY_EEINVAL;				\
	}								\
} while (0)

	LAME_ASSERT(cls != NULL);

	strObj = (*jenv)->AllocObject(jenv, cls);
	LAME_ASSERT(strObj != NULL);

	midGetClass = (*jenv)->GetMethodID(jenv, cls, "getClass", "()Ljava/lang/Class;");
	LAME_ASSERT(midGetClass != NULL);

	clsObj = (*jenv)->CallObjectMethod(jenv, strObj, midGetClass);
	LAME_ASSERT(clsObj != NULL);

	(*jenv)->DeleteLocalRef(jenv, strObj);

	jCls = (*jenv)->GetObjectClass(jenv, clsObj);
	LAME_ASSERT(jCls != NULL);

	midGetFields = (*jenv)->GetMethodID(jenv, jCls, "getMethods", "()[Ljava/lang/reflect/Method;");
	LAME_ASSERT(midGetFields != NULL);

	(*jenv)->DeleteLocalRef(jenv, jCls);

	jobjArray = (jobjectArray)(*jenv)->CallObjectMethod(jenv, clsObj, midGetFields);
	LAME_ASSERT(jobjArray != NULL);

	(*jenv)->DeleteLocalRef(jenv, clsObj);

	len = (*jenv)->GetArrayLength(jenv, jobjArray);

	for (i = 0 ; i < len ; i++) {
		jclass _methodClazz;
		jmethodID mid;
		jstring _name;
		const char *str;
		struct st_method_ll *m;

		jobject _strMethod = (*jenv)->GetObjectArrayElement(jenv, jobjArray , i) ;
		LAME_ASSERT(_strMethod != NULL);

		_methodClazz = (*jenv)->GetObjectClass(jenv, _strMethod) ;
		LAME_ASSERT(_methodClazz != NULL);

		mid = (*jenv)->GetMethodID(jenv, _methodClazz , "toString" , "()Ljava/lang/String;") ;
		LAME_ASSERT(mid != NULL);

		(*jenv)->DeleteLocalRef(jenv, _methodClazz);

		_name = (jstring)(*jenv)->CallObjectMethod(jenv, _strMethod , mid ) ;
		LAME_ASSERT(_name != NULL);

		(*jenv)->DeleteLocalRef(jenv, _strMethod);

		str = (*jenv)->GetStringUTFChars(jenv, _name, 0);
		LAME_ASSERT(str != NULL);

		if (!jyo_method_str_is_from_java(str) &&
		    !jyo_method_str_void_return(str) &&
		    jyo_method_str_void_param(str)) {
			m = jyo_method_str_to_m(str);
			if (m != NULL) {
				jyo_java_convert_str(m->sign, '.', '/');
				m->jmid = (*jenv)->GetMethodID(jenv, cls, m->name, m->sign);
				/* printf("%s: %s%s (%#010x) (%s)\n", __FUNCTION__, m->name, m->sign, (unsigned int)m->jmid, str); */
				if ((m->jmid == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
					fprintf(stderr, "Could not get method id at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
					if ((*jenv)->ExceptionCheck(jenv)) {
						(*jenv)->ExceptionDescribe(jenv);
						(*jenv)->ExceptionClear(jenv);
					}
					fflush(stderr);

					return JY_EEINVAL;
				}

				llappend((void **)mll, (void *)m);
			}
		}

		fflush(stdout);
		(*jenv)->ReleaseStringUTFChars(jenv, _name, str);

		(*jenv)->DeleteLocalRef(jenv, _name);
	}

	(*jenv)->DeleteLocalRef(jenv, jobjArray);

	return JY_ESUCCESS;
}

static int
jyo_fetch_property_boolean(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	jy_bool val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (jy_bool) (*jenv)->CallBooleanMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_BOOL(val);

	return ret;
}

static int
jyo_fetch_property_byte(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	char val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (char) (*jenv)->CallByteMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_BYTE(val);

	return ret;
}

static int
jyo_fetch_property_char(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	char val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (char) (*jenv)->CallCharMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_CHAR(val);

	return ret;
}

static int
jyo_fetch_property_short(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	short val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (short) (*jenv)->CallShortMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_SHORT(val);

	return ret;
}

static int
jyo_fetch_property_int(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	int val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (int) (*jenv)->CallIntMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);

	DEBUG_STR(m->name);
	DEBUG_INT(val);

	return ret;
}

static int
jyo_fetch_property_long(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	long val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (long) (*jenv)->CallLongMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_LONG(val);

	return ret;
}

static int
jyo_fetch_property_float(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	float val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (float) (*jenv)->CallFloatMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_DOUBLE(val);

	return ret;
}

static int
jyo_fetch_property_double(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	double val;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	val = (double) (*jenv)->CallDoubleMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return JY_EEXCEPTION;
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_DOUBLE(val);

	return ret;
}

static int
jyo_fetch_property_string(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	jstring jstr;
	const char *str;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	jstr = (jstring)(*jenv)->CallObjectMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);

		if (jstr != NULL)
			(*jenv)->DeleteLocalRef(jenv, jstr);

		return JY_EEXCEPTION;
	}

	DEBUG_STR(m->name);

	if (jstr == NULL) {
		DEBUG_STR("Deu STR = NULL");
		str = NULL;
	} else {
		str = (*jenv)->GetStringUTFChars(jenv, jstr, 0);
		if (str == NULL) {
			(*jenv)->DeleteLocalRef(jenv, jstr);
			JY_ASSERT_RETURN(str != NULL, JY_EEINVAL);
		}
	}

	ret = jyo_set_property(p, m->name, m->rettype, (void *)str);
	DEBUG_STR(str);

	if (jstr != NULL) {
		(*jenv)->ReleaseStringUTFChars(jenv, jstr, str);
		/*free(str);*/
	}

	/* XXX Está certo rodar o DeleteLocalRef() com ponteiro NULL, NÃO
	 * APAGUE! Se isto não for feito, acontece um JNI leak. */
	(*jenv)->DeleteLocalRef(jenv, jstr);

	return ret;
}

static int
jyo_fetch_property_jyo(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	int ret;
	struct st_jyo *pnew;
		jobject jnew;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	jnew = (*jenv)->CallObjectMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);

		if (jnew != NULL);
			(*jenv)->DeleteLocalRef(jenv, jnew);

		return JY_EEXCEPTION;
	}

	if (jnew == NULL) {
		DEBUG_STR("Deu JY = NULL");
		pnew = NULL;
		ret = jyo_set_property(p, m->name, JYO_TJYO, (void *)pnew);
		return ret;
	}

	pnew = malloc(sizeof(struct st_jyo));
	if (pnew == NULL) {
		(*jenv)->DeleteLocalRef(jenv, jnew);
		JY_ASSERT_RETURN(pnew != NULL, JY_EENOMEM);
	}

	ret = jyo_j2p(jenv, jnew, pnew);
	(*jenv)->DeleteLocalRef(jenv, jnew);
	if (ret == JY_ESUCCESS)
		ret = jyo_set_property(p, m->name, JYO_TJYO, (void *)pnew);
	else
		jyo_free(pnew);
	free(pnew); /* This pointer has already been used. Free it. */

	return ret;
}

static int
jyo_fetch_property(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *m)
{
	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(m != NULL, JY_EEINVAL);

	switch (m->rettype) {
		case JYO_TBOOLEAN:
			return jyo_fetch_property_boolean(jenv, jcls, j, p, m);
			break;
		case JYO_TBYTE:
			return jyo_fetch_property_byte(jenv, jcls, j, p, m);
			break;
		case JYO_TCHAR:
			return jyo_fetch_property_char(jenv, jcls, j, p, m);
			break;
		case JYO_TSHORT:
			return jyo_fetch_property_short(jenv, jcls, j, p, m);
			break;
		case JYO_TINT:
			return jyo_fetch_property_int(jenv, jcls, j, p, m);
			break;
		case JYO_TLONG:
			return jyo_fetch_property_long(jenv, jcls, j, p, m);
			break;
		case JYO_TFLOAT:
			return jyo_fetch_property_float(jenv, jcls, j, p, m);
			break;
		case JYO_TDOUBLE:
			return jyo_fetch_property_double(jenv, jcls, j, p, m);
			break;
		case JYO_TSTRING:
			return jyo_fetch_property_string(jenv, jcls, j, p, m);
			break;
		case JYO_TJCLASS:
			return jyo_fetch_property_jyo(jenv, jcls, j, p, m);
			break;
		default:
			break;
	}

	JY_WARN_ENOSYS();
	return JY_EENOSYS;
}

/*
 * XXX Preenche o "p"(estrutura C) utilizando o "j" como fonte, baseado na lista "mll".
 */
static int
jyo_fetch_property_list(JNIEnv *jenv, jclass jcls, jobject j, struct st_jyo *p, const struct st_method_ll *mll)
{
	int ret;

	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(jcls != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(mll != NULL, JY_EEINVAL);

	for (; mll != NULL; mll = (struct st_method_ll *)mll->ll.next) {
		ret = jyo_fetch_property(jenv, jcls, j, p, mll);
		if (ret != JY_ESUCCESS) {
			return ret;
		}
	}

	return JY_ESUCCESS;
}

/** XXX TODO Converte o objeto Java (j) em uma estrutura "st_jyo" (p). */
int
jyo_j2p(JNIEnv *jenv, jobject j, struct st_jyo *p)
{
	jclass jcls;
	char *str;
	int ret;
	struct st_method_ll *mll;


	JY_ASSERT_RETURN(jenv != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(j != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);

	memset(p, 0, sizeof(struct st_jyo));

	/*
	 * XXX Pega classe java que foi passada
	 */
	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return JY_EEINVAL;
	}

	/*
	 * XXX Extrai a assinatura da classe java
	 */
	str = jyo_get_jclass_name(jenv, jcls, j);
	if (str == NULL) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Could not get class name at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);

		return JY_EEINVAL;
	}

	/*
	 * XXX Inicia a estrutura com a assinatura da classe.
	 */
	ret = jyo_init(p, str);
	free(str);

	if (ret != JY_ESUCCESS) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Error %s while initializing jyo structure at \"%s:%d\".\n", jy_strerror(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);

		jyo_free(p);
		return ret;
	}

	/*
	 * XXX Lista em mll os metodos sem parametros e que nao retornam void
	 * da classe jcls (os getters).
	 */
	ret = jyo_get_method_sign_list(jenv, jcls, &mll);
	if ((ret != JY_ESUCCESS) || (mll == NULL)) {
#ifdef JY_DEBUG_ERROR
		if (mll == NULL)
			fprintf(stderr, "Found no methods from jobject at \"%s:%d\".\n", __FILE__, __LINE__);
		else
			fprintf(stderr, "Error %s while getting method list from jobject at \"%s:%d\".\n", jy_strerror(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);
		jyo_free(p);
		jyo_free_method_ll(&mll);

		return ret;
	}

	/*
	 * XXX
	 * Preenche o "p"(estrutura C) utilizando o "j" como fonte, baseado na lista "mll".
	 */
	ret = jyo_fetch_property_list(jenv, jcls, j, p, mll);
/*	jyo_free_method_ll(&mll);*/
	(*jenv)->DeleteLocalRef(jenv, jcls);

	if ((ret != JY_ESUCCESS) || (mll == NULL)) {
#ifdef JY_DEBUG_ERROR
		fprintf(stderr, "Error %s while getting method list from jobject at \"%s:%d\".\n", jy_strerror(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

	jyo_free_method_ll(&mll);
		jyo_free(p);

		return ret;
	}

jyo_free_method_ll(&mll);

	return JY_ESUCCESS;
}

/**
 * XXX
 * TODO Lê a propriedade de uma estrutura "st_jyo".
 * Passa só o ponteiro.
 */
int
jyo_get_property(struct st_jyo *p, char *getter, enum e_jyo_type data_type, void **data)
{
	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(getter != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(data != NULL, JY_EEINVAL);

	JY_WARN_ENOSYS();
	return JY_EENOSYS;
}

/**
 * XXX
 * TODO Lê dados da estrutura "st_jyo" e copia em "data".
 * Duplica os dados (aloca memória).
 */
int
jyo_get_property_copy(struct st_jyo *p, char *getter, enum e_jyo_type data_type, void **buf)
{
	struct st_jyo_property_ll *reg_atual;
	size_t data_size;

	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(getter != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(buf != NULL, JY_EEINVAL);

DEBUG_STR("copy");
DEBUG_STR(getter);
DEBUG_STR(p->clazz);

	/* XXX varre lista e compara "getter" */
	for (reg_atual = p->properties;
	    reg_atual != NULL;
	    reg_atual = (struct st_jyo_property_ll *)reg_atual->ll.next) {
		if (strncmp(reg_atual->st.method_name, getter, 255) == 0)
			break;
	}

	if (reg_atual == NULL)
		return JY_ENOTFOUND;

	/* XXX para casos de string ou objeto nulo */
	if (data_type == JYO_TSTRING || data_type == JYO_TJYO) {
		if (reg_atual->st.data == NULL) {
			*buf = NULL;
			return JY_ESUCCESS;
		}
	}

	if (data_type == JYO_TSTRING) {
		data_size = strlen((char *)reg_atual->st.data) + 1;
	} else {
		data_size = jyo_get_type_size(data_type);
		JY_ASSERT_RETURN(data_size != JY_EEINVAL, JY_EEINVAL);
	}

	JY_ASSERT_RETURN(data_size > 0, JY_EEINVAL);
	*buf = malloc(data_size);
	if (*buf == NULL) {
		p->error = JY_EENOMEM;
		return JY_EENOMEM;
	}

	memcpy(*buf, reg_atual->st.data, data_size);

	if (data_type == JYO_TSTRING) {
		((char *)*buf)[data_size-1] = '\000';
DEBUG_STR((char *)*buf);
	} else
DEBUG_PTR(*buf);

	return JY_ESUCCESS;
}

/*
 * XXX
 * grava no buffer passado(buf) de tamanho "buf_size", e sem alocar nada, o
 * dado da estrutura "st_jyo", retornado pelo "getter"
 */
int
jyo_get_property_buf(struct st_jyo *p, char *getter, enum e_jyo_type data_type, void *buf, size_t buf_size)
{
	struct st_jyo_property_ll *reg_atual;

	JY_ASSERT_RETURN(p != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(getter != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(buf != NULL, JY_EEINVAL);
	JY_ASSERT_RETURN(buf_size > 0, JY_EEINVAL);

DEBUG_STR("buf");
DEBUG_STR(getter);
DEBUG_STR(p->clazz);

	if (data_type != JYO_TSTRING) {
		JY_ASSERT_RETURN(jyo_get_type_size(data_type) != JY_EEINVAL, JY_EEINVAL);
		JY_ASSERT_RETURN(jyo_get_type_size(data_type) <= buf_size, JY_EEINVAL);
		buf_size = jyo_get_type_size(data_type);
	}

	/* XXX varre lista e compara "getter" */
	for (reg_atual = p->properties;
	    reg_atual != NULL;
	    reg_atual = (struct st_jyo_property_ll *)reg_atual->ll.next) {
		if (strncmp(reg_atual->st.method_name, getter, 255) == 0)
			break;
	}

	if (reg_atual == NULL)
		return JY_ENOTFOUND;

	/* XXX para casos de string ou objeto nulo */
	if (data_type == JYO_TSTRING || data_type == JYO_TJYO) {
		if (reg_atual->st.data == NULL) {
			/* buf = NULL;*/
			memset (buf, 0, buf_size);
			return JY_ESUCCESS;
		}
	}

	if (data_type == JYO_TSTRING) {
		/* XXX ajusta o buf */
		if ((strlen((char *)reg_atual->st.data) + 1) < buf_size)
			buf_size = strlen((char *)reg_atual->st.data) + 1;
	}

	memcpy(buf, reg_atual->st.data, buf_size);
	if (data_type == JYO_TSTRING) {
		((char *)buf)[buf_size-1] = '\000';
DEBUG_STR((char *)buf);
	} else
DEBUG_INT(*((int *)buf));

	return JY_ESUCCESS;
}

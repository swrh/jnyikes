#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <jni.h>

#include "pjni.h"
#include "pobject.h"

#define POBJECT_DEBUG_ERROR
/*
#define POBJECT_DEBUG_VERBOSE
*/
#if 1
#define DEBUG_BOOL(x) do { fprintf(stderr, "%s:%d: %s = %s;\n", __FILE__, __LINE__, #x, x == FALSE ? "FALSE" : "TRUE"); fflush(stderr); } while (0)
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


void
pobject_free_method_ll(struct st_method_ll **mll) {
	PJNI_ASSERT_RETURN_VOID(mll != NULL);

	while (*mll != NULL) {
		if ((*mll)->name != NULL)
			free((*mll)->name);
		if ((*mll)->sign != NULL)
			free((*mll)->sign);
		lldel((void **) mll, *mll);
	}
}

/*
 * Funções de liberação de memória.
 */

/** Libera uma estrutura de propriedade. */
static void
pobject_property_free(struct st_pobject_property *p)
{
	PJNI_ASSERT_RETURN_VOID(p != NULL);

	if (p->method_name != NULL) {
		free(p->method_name);
		p->method_name = NULL;
	}

	if (p->data != NULL) {
		free(p->data);
		p->data = NULL;
	}
}

/** Libera a lista ligada de propriedades. */
static void
pobject_property_ll_free(struct st_pobject_property_ll **head_ll)
{
	struct st_pobject_property_ll *p_ll;

	PJNI_ASSERT_RETURN_VOID(head_ll != NULL);

	if (*head_ll == NULL)
		return;

	/* Libera o conteudo de cada "st_property". */
	for (p_ll = *head_ll; p_ll != NULL;
	    p_ll = (struct st_pobject_property_ll *) p_ll->ll.next) {
		pobject_property_free(&p_ll->st);
	}

	/* Destroi a lista ligada. */
	lldestroy((void **) head_ll);
}

/** Libera toda a estrutura "st_pobject". */
void
pobject_free(struct st_pobject *p)
{
	PJNI_ASSERT_RETURN_VOID(p != NULL);

	if (p->clazz != NULL) {
		free(p->clazz);
		p->clazz = NULL;
	}

	pobject_property_ll_free(&p->properties);

	p->error = POBJECT_SUCCESS;
}

/** Inicializa uma estrutura "st_pobject". */
int
pobject_init(struct st_pobject *p, char *clazz)
{
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);

	memset(p, 0, sizeof(struct st_pobject));

	if (clazz != NULL) {
		p->clazz = strdup(clazz);
		if (p->clazz == NULL)
			return POBJECT_ERROR_ENOMEM;
	}

	return POBJECT_SUCCESS;
}


/*
 * Funções de uso geral.
 */

/** Retorna o tamanho de um tipo de dado "e_pobject_type". */
static size_t
pobject_get_type_size(enum e_pobject_type t)
{
	switch (t) {
		case POBJECT_TYPE_BOOLEAN:
			return sizeof(bool);
		case POBJECT_TYPE_BYTE:
			return sizeof(char);
		case POBJECT_TYPE_CHAR:
			return sizeof(char);
		case POBJECT_TYPE_SHORT:
			return sizeof(short);
		case POBJECT_TYPE_INT:
			return sizeof(int);
		case POBJECT_TYPE_LONG:
			return sizeof(long);
		case POBJECT_TYPE_FLOAT:
			return sizeof(float);
		case POBJECT_TYPE_DOUBLE:
			return sizeof(double);
			/*
		case POBJECT_TYPE_UINT:
			return sizeof(unsigned int);
		case POBJECT_TYPE_ULONG:
			return sizeof(unsigned long);
			*/
		case POBJECT_TYPE_POBJECT:
			return sizeof(struct st_pobject);
		case POBJECT_TYPE_STRING:
		default:
			return (size_t) POBJECT_ERROR_EINVAL;
	}
}

/** Retorna os nomes dos tipos. */
const char *
pobject_get_type_name(enum e_pobject_type t)
{
#define CASE_RETURN(x) case x: return #x
	switch (t) {
		CASE_RETURN(POBJECT_TYPE_DOUBLE);
		CASE_RETURN(POBJECT_TYPE_FLOAT);
		CASE_RETURN(POBJECT_TYPE_INT);
		CASE_RETURN(POBJECT_TYPE_LONG);
		CASE_RETURN(POBJECT_TYPE_POBJECT);
		CASE_RETURN(POBJECT_TYPE_STRING);
		CASE_RETURN(POBJECT_TYPE_UINT);
		CASE_RETURN(POBJECT_TYPE_ULONG);
		CASE_RETURN(POBJECT_TYPE_BOOLEAN);
		CASE_RETURN(POBJECT_TYPE_BYTE);
		CASE_RETURN(POBJECT_TYPE_CHAR);
		CASE_RETURN(POBJECT_TYPE_SHORT);
		CASE_RETURN(POBJECT_TYPE_VOID);
		CASE_RETURN(POBJECT_TYPE_JCLASS);
		default:
			return NULL;
	}
#undef CASE_NAME
}

/** Retorna os nomes dos erros. */
const char *
pobject_get_error_name(enum e_pobject_error t)
{
#define CASE_RETURN(x) case x: return #x
	switch (t) {
		CASE_RETURN(POBJECT_SUCCESS);
		/* CASE_RETURN(POBJECT_ERROR_SUCCESS); */
		CASE_RETURN(POBJECT_ERROR_EDEADLK);
		CASE_RETURN(POBJECT_ERROR_EINVAL);
		CASE_RETURN(POBJECT_ERROR_ENOMEM);
		CASE_RETURN(POBJECT_ERROR_ENOSYS);
		CASE_RETURN(POBJECT_ERROR_INTERNAL);
		CASE_RETURN(POBJECT_ERROR_NOJCLASS);
		CASE_RETURN(POBJECT_ERROR_NOTFOUND);
		CASE_RETURN(POBJECT_ERROR_EXCEPTION);
		default:
			return NULL;
	}
#undef CASE_NAME
}

/*
 * Adiciona propriedade "method name" na lista "p->proprierties", com o dado "orig_data"
 */
int
pobject_set_property(struct st_pobject *p, const char *method_name, enum e_pobject_type data_type, const void *orig_data)
{
	struct st_pobject_property_ll *p_ll;
	size_t data_size;
	void *data;

	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(method_name != NULL, POBJECT_ERROR_EINVAL);

	if (pobject_error(p) != POBJECT_SUCCESS)
		return POBJECT_ERROR_EINVAL;

	/*
	 * verifica o tamanho do orig_data
	 */
	if (data_type == POBJECT_TYPE_STRING) {
		if (orig_data != NULL) {
			data_size = strlen((char *) orig_data) + 1;
DEBUG_STR(method_name);
DEBUG_STR((char *)orig_data);
		} else 
			data_size = 0;
	} else {
		if (orig_data != NULL) {
			data_size = pobject_get_type_size(data_type);
			PJNI_ASSERT_RETURN(data_size != POBJECT_ERROR_EINVAL, POBJECT_ERROR_EINVAL);
DEBUG_STR(method_name);
		} else 
			data_size = 0;
	}

	if (data_size > 0 ) {
/*	PJNI_ASSERT_RETURN(data_size > 0, POBJECT_ERROR_EINVAL);*/
		data = malloc(data_size);
		if (data == NULL) {
			p->error = POBJECT_ERROR_ENOMEM;
			return POBJECT_ERROR_ENOMEM;
		}
		memcpy(data, orig_data, data_size);
	} else
		data = NULL;
	
	p_ll = malloc(sizeof(struct st_pobject_property_ll));
	if (p_ll == NULL) {
		if (data_size > 0 )
			free(data);

		p->error = POBJECT_ERROR_ENOMEM;
		return POBJECT_ERROR_ENOMEM;
	}

	memset(p_ll, 0, sizeof(struct st_pobject_property_ll));

	p_ll->st.data_type = data_type;
	p_ll->st.data = data;

	p_ll->st.method_name = strdup(method_name);
	if (p_ll->st.method_name == NULL) {
		pobject_property_ll_free(&p_ll);

		p->error = POBJECT_ERROR_ENOMEM;
		return POBJECT_ERROR_ENOMEM;
	}

	llappend((void **) &p->properties, p_ll);

	return POBJECT_SUCCESS;
}

/** TODO COMMENT */
static int
pobject_get_static_mid(JNIEnv *jenv, const char *clazz, const char *method, char *sig, jclass *jcls, jmethodID *jmid)
{
	PJNI_ASSERT_RETURN(clazz != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(method != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(sig != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jmid != NULL, POBJECT_ERROR_EINVAL);

	*jcls = (*jenv)->FindClass(jenv, clazz);
	if ((*jcls == NULL) || ((*jenv)->ExceptionCheck(jenv))) {
#ifdef POBJECT_DEBUG_ERROR
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

		return POBJECT_ERROR_NOJCLASS;
	}

	*jmid = (*jenv)->GetStaticMethodID(jenv, *jcls, method, sig);
	if ((*jmid == NULL) || ((*jenv)->ExceptionCheck(jenv))) {
#ifdef POBJECT_DEBUG_ERROR
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

		return POBJECT_ERROR_NOTFOUND;
	}

	return POBJECT_SUCCESS;
}

/** TODO COMMENT */
static char *
pobject_get_method_signature(enum e_pobject_type type_ret, void *type_ret_detail, enum e_pobject_type type_param, const char *type_param_detail)
{
#define SWITCH_TYPE_CAT_s(type, param) do {				\
	switch (type) {							\
		case POBJECT_TYPE_BOOLEAN:				\
			strcat(s, "Z");					\
			break;						\
		case POBJECT_TYPE_BYTE:					\
			strcat(s, "B");					\
			break;						\
		case POBJECT_TYPE_CHAR:					\
			strcat(s, "C");					\
			break;						\
		case POBJECT_TYPE_SHORT:				\
			strcat(s, "S");					\
			break;						\
		case POBJECT_TYPE_UINT:					\
		case POBJECT_TYPE_INT:					\
			strcat(s, "I");					\
			break;						\
		case POBJECT_TYPE_ULONG:				\
		case POBJECT_TYPE_LONG:					\
			strcat(s, "J");					\
			break;						\
		case POBJECT_TYPE_FLOAT:				\
			strcat(s, "F");					\
			break;						\
		case POBJECT_TYPE_DOUBLE:				\
			strcat(s, "D");					\
			break;						\
		case POBJECT_TYPE_VOID:					\
			strcat(s, "V");					\
			break;						\
		case POBJECT_TYPE_STRING:				\
			strcat(s, "L");					\
			strcat(s, g_str_clazz_string);			\
			strcat(s, ";");					\
			break;						\
		case POBJECT_TYPE_POBJECT:				\
			strcat(s, "L");					\
			strcat(s, ((struct st_pobject *) param)->clazz);\
			strcat(s, ";");					\
			break;						\
		case POBJECT_TYPE_JCLASS:				\
			strcat(s, "L");					\
			strcat(s, (char *) param);			\
			strcat(s, ";");					\
			break;						\
	}								\
} while (0)

	char *s;
	size_t slen;

	/* Verifica os parâmetros passados. */
	if (type_param == POBJECT_TYPE_JCLASS)
		PJNI_ASSERT_RETURN(type_param_detail != NULL, NULL);
	else if (type_param != POBJECT_TYPE_STRING)
		PJNI_ASSERT_RETURN(pobject_get_type_size(type_param) > 0, NULL);

	if (type_ret == POBJECT_TYPE_JCLASS)
		PJNI_ASSERT_RETURN(type_ret_detail != NULL, NULL);
	else if (type_ret != POBJECT_TYPE_STRING)
		PJNI_ASSERT_RETURN(pobject_get_type_size(type_ret) > 0, NULL);

	slen = 1;	/* EOS - '\000' */

	if (type_ret == POBJECT_TYPE_JCLASS)		/* Lpath/Class; */
		slen += strlen(type_ret_detail) + 2;
	else if (type_ret == POBJECT_TYPE_STRING)	/* Lpath/Class; */
		slen += strlen(g_str_clazz_string) + 2;
	else						/* T */
		slen += 1;

	if (type_param == POBJECT_TYPE_JCLASS)		/* Lpath/Class; */
		slen += strlen(type_param_detail) + 2;
	else if (type_param == POBJECT_TYPE_STRING)	/* Lpath/Class; */
		slen += strlen(g_str_clazz_string) + 2;
	else if (type_param == POBJECT_TYPE_POBJECT)	/* Lpath/Class; */
		slen += strlen(((struct st_pobject *) type_param_detail)->clazz) + 2;
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

/** TODO Envia a estrutura "st_pobject" à máquina virtual. */
int
pobject_send(JNIEnv *jenv, struct st_pobject *p, const char *clazz, const char *method)
{
	int ret;
	jobject jobj;
	jclass jcls;
	jmethodID jmid;
	char *sig;
	jboolean jret;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(clazz != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(method != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p->clazz != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p->error == POBJECT_SUCCESS, p->error);

	sig = pobject_get_method_signature(POBJECT_TYPE_BOOLEAN, NULL, POBJECT_TYPE_JCLASS, p->clazz);
	if (sig == NULL)
		return POBJECT_ERROR_ENOMEM;

	ret = pobject_get_static_mid(jenv, clazz, method, sig, &jcls, &jmid);

	free(sig);
	if (ret != POBJECT_SUCCESS)
		return ret;

	ret = pobject_p2j(jenv, p, &jobj);
	if (ret != POBJECT_SUCCESS) {
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return ret;
	}

	/* 20060408 - fsilveira - reparo: era CallStaticVoidMethod() (errado) */
	jret = (*jenv)->CallStaticBooleanMethod(jenv, jcls, jmid, jobj);
	(*jenv)->DeleteLocalRef(jenv, jcls);
	(*jenv)->DeleteLocalRef(jenv, jobj);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	return jret == JNI_FALSE ? POBJECT_ERROR_EXCEPTION : POBJECT_SUCCESS;
}

/** Verifica o estado de erro ocorrido na estrutura "st_pobject". */
int
pobject_error(struct st_pobject *p)
{
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);

	return p->error;
}

/** TODO COMMENT */
static jobject
pobject_new_jobject(JNIEnv *jenv, const char *clazz)
{
	jclass jcls;
	jmethodID jmid;
	jobject jobj;

	PJNI_ASSERT_RETURN(jenv != NULL, NULL);
	PJNI_ASSERT_RETURN(clazz != NULL, NULL);

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

/** TODO COMMENT */
static int
pobject_fill_jobject_property(JNIEnv *jenv, jobject j, struct st_pobject *p, struct st_pobject_property *pp)
{
	jclass jcls;
	jmethodID jmid;
	jobject new_jobj;
	jstring new_jstr;
	char *sig;
	int ret, rettype;

	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return POBJECT_ERROR_EINVAL;
	}

#define POBJECT_GET_METHOD(jenv, jcls, jmid, set, sig, tret, dret, tpar, dpar) \
do {									\
	if ((*jenv)->ExceptionCheck(jenv))				\
		(*jenv)->ExceptionClear(jenv);				\
	if (sig != NULL)						\
		free(sig);						\
									\
	sig = pobject_get_method_signature(tret, dret, tpar, dpar);	\
	PJNI_ASSERT_RETURN(sig != NULL, POBJECT_ERROR_EINVAL);		\
									\
	jmid = (*jenv)->GetMethodID(jenv, jcls, set, sig);		\
} while (0)

	sig = NULL;
	jmid = 0;

	/* Tenta achar o metodo com tipo de retorno "void". */
	if (jmid == 0) {
		POBJECT_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, POBJECT_TYPE_VOID, NULL, pp->data_type, pp->data);
		rettype = POBJECT_TYPE_VOID;
	}

	/* Tenta achar o metodo com tipo de retorno "boolean". */
	if (jmid == 0) {
		POBJECT_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, POBJECT_TYPE_BOOLEAN, NULL, pp->data_type, pp->data);
		rettype = POBJECT_TYPE_BOOLEAN;
	}

	if ((jmid == 0) && (pp->data_type == POBJECT_TYPE_POBJECT)) {
		/* Tenta achar o metodo com tipo de parametro java/lang/Object e retorno "void". */
		if (jmid == 0) {
			POBJECT_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, POBJECT_TYPE_VOID, NULL, POBJECT_TYPE_JCLASS, g_str_clazz_object);
			rettype = POBJECT_TYPE_VOID;
		}
	
		/* Tenta achar o metodo com tipo de parametro java/lang/Object e retorno "boolean". */
		if (jmid == 0) {
			POBJECT_GET_METHOD(jenv, jcls, jmid, pp->method_name, sig, POBJECT_TYPE_BOOLEAN, NULL, POBJECT_TYPE_JCLASS, g_str_clazz_object);
			rettype = POBJECT_TYPE_BOOLEAN;
		}
	}

#if 0
	sig = pobject_get_method_signature(POBJECT_TYPE_VOID, NULL, pp->data_type, pp->data);
	PJNI_ASSERT_RETURN(sig != NULL, POBJECT_ERROR_EINVAL);

	jmid = (*jenv)->GetMethodID(jenv, jcls, pp->method_name, sig);
	if ((jmid == 0) && (pp->data_type == POBJECT_TYPE_POBJECT)) {
		if ((*jenv)->ExceptionCheck(jenv))
			(*jenv)->ExceptionClear(jenv);

		free(sig);
		sig = pobject_get_method_signature(POBJECT_TYPE_VOID, NULL, POBJECT_TYPE_JCLASS, g_str_clazz_object);
		PJNI_ASSERT_RETURN(sig != NULL, POBJECT_ERROR_EINVAL);

		jmid = (*jenv)->GetMethodID(jenv, jcls, pp->method_name, sig);
	}
#endif

	if ((jmid == 0) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Could not find method \"%s\" with signature \"%s\" of an object of class \"%s\" at \"%s:%d\".\n", pp->method_name, sig, p->clazz, __FILE__, __LINE__);
#endif

		free(sig);

		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return POBJECT_ERROR_NOTFOUND;
	}

	if (pp->data_type != POBJECT_TYPE_VOID)
		PJNI_ASSERT_RETURN(pp->data != NULL, POBJECT_ERROR_EINVAL);

	switch (pp->data_type) {
		case POBJECT_TYPE_BOOLEAN:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jboolean) *((jboolean *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jboolean) *((jboolean *) pp->data));
			break;
		case POBJECT_TYPE_BYTE:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jbyte) *((char *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jbyte) *((char *) pp->data));
			break;
		case POBJECT_TYPE_CHAR:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jchar) *((char *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jchar) *((char *) pp->data));
			break;
		case POBJECT_TYPE_SHORT:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jshort) *((short *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jshort) *((short *) pp->data));
			break;
		case POBJECT_TYPE_INT:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jint) *((int *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jint) *((int *) pp->data));
			break;
		case POBJECT_TYPE_LONG:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jlong) *((long *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jlong) *((long *) pp->data));
			break;
		case POBJECT_TYPE_FLOAT:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jfloat) *((float *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jfloat) *((float *) pp->data));
			break;
		case POBJECT_TYPE_DOUBLE:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, (jdouble) *((double *) pp->data));
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, (jdouble) *((double *) pp->data));
			break;
		case POBJECT_TYPE_VOID:
			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid);
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid);
			break;
		case POBJECT_TYPE_POBJECT:
			ret = pobject_p2j(jenv, (struct st_pobject *) pp->data, &new_jobj);
			if (ret != POBJECT_SUCCESS) {
#ifdef POBJECT_DEBUG_ERROR
				fprintf(stderr, "%s while converting an \"st_pobject\" structure to \"jobject\" at \"%s:%d\".\n", pobject_get_error_name(ret), __FILE__, __LINE__);
				fflush(stderr);
#endif
				free(sig);
				(*jenv)->DeleteLocalRef(jenv, jcls);
				return ret;
			}

			if (rettype == POBJECT_TYPE_VOID)
				(*jenv)->CallVoidMethod(jenv, j, jmid, new_jobj);
			else if (rettype == POBJECT_TYPE_BOOLEAN)
				(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, new_jobj);
			(*jenv)->DeleteLocalRef(jenv, new_jobj);

			break;
		case POBJECT_TYPE_STRING:
			new_jstr = (*jenv)->NewStringUTF(jenv, (char *) pp->data);
			if (new_jstr != NULL) {
				if (rettype == POBJECT_TYPE_VOID)
					(*jenv)->CallVoidMethod(jenv, j, jmid, new_jstr);
				else if (rettype == POBJECT_TYPE_BOOLEAN)
					(void)(*jenv)->CallBooleanMethod(jenv, j, jmid, new_jstr);
				(*jenv)->DeleteLocalRef(jenv, new_jstr);
				fflush(stderr);
#ifdef POBJECT_DEBUG_ERROR
			} else {
				fprintf(stderr, "Error converting a C string to Java String at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
			}

			break;

		case POBJECT_TYPE_JCLASS:
		default:
			POBJECT_WARN_ENOSYS();
			/*(*jenv)->DeleteLocalRef(jenv, jcls);
			return POBJECT_ERROR_ENOSYS;*/
			break;
	}

	if ((*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Exception ocurred while calling method \"%s\" with signature \"%s\" of an object of class \"%s\".\n", pp->method_name, sig, p->clazz);
#endif
		free(sig);
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		(*jenv)->DeleteLocalRef(jenv, jcls);
		return POBJECT_ERROR_EXCEPTION;
	}

	free(sig);

	return POBJECT_SUCCESS;
}

/** TODO COMMENT */
static int
pobject_fill_jobject(JNIEnv *jenv, jobject j, struct st_pobject *p)
{
	struct st_pobject_property_ll *p_ll;
	jclass jcls;
	int ret;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);

	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return POBJECT_ERROR_EINVAL;
	}

	for (p_ll = p->properties; p_ll != NULL;
	    p_ll = (struct st_pobject_property_ll *) p_ll->ll.next) {
#ifdef POBJECT_DEBUG_VERBOSE
		printf("DEBUG: configurando propriedade \"%s\" do objeto de classe \"%s\".\n", p_ll->st.method_name, p->clazz);
		fflush(stdout);
#endif

		ret = pobject_fill_jobject_property(jenv, j, p, &p_ll->st);
		if (ret != POBJECT_SUCCESS) {
			(*jenv)->DeleteLocalRef(jenv, jcls);
			return ret;
		}
	}

	(*jenv)->DeleteLocalRef(jenv, jcls);

	return POBJECT_SUCCESS;
}

/** TODO Converte a estrutura "st_pobject" em um objeto Java. */
int
pobject_p2j(JNIEnv *jenv, struct st_pobject *p, jobject *j)
{
	int ret;

	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	memset(j, 0, sizeof(jobject));
	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p->clazz != NULL, POBJECT_ERROR_EINVAL);

	if (pobject_error(p) != POBJECT_SUCCESS)
		return POBJECT_ERROR_EINVAL;

	*j = pobject_new_jobject(jenv, p->clazz);
	if (*j == NULL)
		return POBJECT_ERROR_NOJCLASS;

	ret = pobject_fill_jobject(jenv, *j, p);
	if (ret != POBJECT_SUCCESS) {
		(*jenv)->DeleteLocalRef(jenv, *j);
		*j = NULL;
		return ret;
	}

	return POBJECT_SUCCESS;
}

static char *
pobject_get_jclass_name(JNIEnv *jenv, jclass jcls, jobject strObj)
{
	jmethodID midGetClass;
	jobject clsObj;
	jclass jCls;
	jmethodID midGetFields;
	jstring _name;

	const char *str;
	char *ret;

	PJNI_ASSERT_RETURN(jenv != NULL, NULL);
	PJNI_ASSERT_RETURN(jcls != NULL, NULL);

/*	strObj = (*jenv)->AllocObject(jenv, jcls);
	PJNI_ASSERT_RETURN(strObj != NULL, NULL);*/

	midGetClass = (*jenv)->GetMethodID(jenv, jcls, "getClass", "()Ljava/lang/Class;");
	PJNI_ASSERT_RETURN(midGetClass != NULL, NULL);

	clsObj = (*jenv)->CallObjectMethod(jenv, strObj, midGetClass);
/*	(*jenv)->DeleteLocalRef(jenv, strObj);*/
	PJNI_ASSERT_RETURN(clsObj != NULL, NULL);

	jCls = (*jenv)->GetObjectClass(jenv, clsObj);
	PJNI_ASSERT_RETURN(jCls != NULL, NULL);

	midGetFields = (*jenv)->GetMethodID(jenv, jCls, "getName", "()Ljava/lang/String;");
	(*jenv)->DeleteLocalRef(jenv, jCls);
	PJNI_ASSERT_RETURN(midGetFields != NULL, NULL);

	_name = (jstring)(*jenv)->CallObjectMethod(jenv, clsObj, midGetFields);
	(*jenv)->DeleteLocalRef(jenv, clsObj);
	PJNI_ASSERT_RETURN(_name != NULL, NULL);

	str = (*jenv)->GetStringUTFChars(jenv, _name, 0);
	PJNI_ASSERT_RETURN(str != NULL, NULL);

	ret = strdup(str);

	(*jenv)->ReleaseStringUTFChars(jenv, _name, str);

	(*jenv)->DeleteLocalRef(jenv, _name);

	return ret;
}

static const char *
pobject_method_str_get_param(const char *str)
{
	const char *p;

	PJNI_ASSERT_RETURN(str != NULL, NULL);

	p = strchr(str, (int) '(');
	PJNI_ASSERT_RETURN(p != NULL, NULL);
	PJNI_ASSERT_RETURN(p != str, NULL);
	p++;

	return p;
}

static const char *
pobject_method_str_get_name(const char *str)
{
	const char *pparam, *pname, *p;

	PJNI_ASSERT_RETURN(str != NULL, NULL);

	pparam = pobject_method_str_get_param(str);
	PJNI_ASSERT_RETURN(pparam != NULL, NULL);

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
pobject_method_str_get_type(const char *str)
{
	PJNI_ASSERT_RETURN(str != NULL, NULL);

	if (strlen(str) > 7)
		if (strncmp(str, "public ", 7) == 0)
			return str + 7;
	return NULL;
}

static int
pobject_str_get_type(const char *str)
{
	int len;

	PJNI_ASSERT_RETURN(str != NULL, POBJECT_ERROR_EINVAL);

	if (strchr(str, ' ') != NULL)
		len = (int) (strchr(str, ' ') - str);
	else
		len = strlen(str);

#define POBJECT_STRING_TYPE_BOOLEAN "boolean"
#define POBJECT_STRING_TYPE_BOOLEAN_LENGTH 7
#define POBJECT_STRING_TYPE_BYTE "byte"
#define POBJECT_STRING_TYPE_BYTE_LENGTH 4
#define POBJECT_STRING_TYPE_CHAR "char"
#define POBJECT_STRING_TYPE_CHAR_LENGTH 4
#define POBJECT_STRING_TYPE_SHORT "short"
#define POBJECT_STRING_TYPE_SHORT_LENGTH 5
#define POBJECT_STRING_TYPE_INT "int"
#define POBJECT_STRING_TYPE_INT_LENGTH 3
#define POBJECT_STRING_TYPE_LONG "long"
#define POBJECT_STRING_TYPE_LONG_LENGTH 4
#define POBJECT_STRING_TYPE_FLOAT "float"
#define POBJECT_STRING_TYPE_FLOAT_LENGTH 5
#define POBJECT_STRING_TYPE_DOUBLE "double"
#define POBJECT_STRING_TYPE_DOUBLE_LENGTH 6
#define POBJECT_STRING_TYPE_VOID "void"
#define POBJECT_STRING_TYPE_VOID_LENGTH 4
#define POBJECT_STRING_TYPE_STRING "java.lang.String"
#define POBJECT_STRING_TYPE_STRING_LENGTH 16

#define ISTYPE(x) (strncmp(str, POBJECT_STRING_TYPE_##x, len < POBJECT_STRING_TYPE_##x##_LENGTH ? len : POBJECT_STRING_TYPE_##x##_LENGTH) == 0)

	if (ISTYPE(BOOLEAN))
		return POBJECT_TYPE_BOOLEAN;
	else if (ISTYPE(BYTE))
		return POBJECT_TYPE_BYTE;
	else if (ISTYPE(CHAR))
		return POBJECT_TYPE_CHAR;
	else if (ISTYPE(SHORT))
		return POBJECT_TYPE_SHORT;
	else if (ISTYPE(INT))
		return POBJECT_TYPE_INT;
	else if (ISTYPE(LONG))
		return POBJECT_TYPE_LONG;
	else if (ISTYPE(FLOAT))
		return POBJECT_TYPE_FLOAT;
	else if (ISTYPE(DOUBLE))
		return POBJECT_TYPE_DOUBLE;
	else if (ISTYPE(VOID))
		return POBJECT_TYPE_VOID;
	else if (ISTYPE(STRING))
		return POBJECT_TYPE_STRING;

	return POBJECT_ERROR_EINVAL;
}

static const char *
pobject_java_basename(const char *str)
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
pobject_java_convert_str(char *str, char o, char d)
{
	char *p;

	PJNI_ASSERT_RETURN_VOID(str != NULL);

	for (p = strchr(str, (int) o); p != NULL; p = strchr(p, (int) o))
		*p = d;
}

static struct st_method_ll *
pobject_method_str_to_m(const char *str)
{
	const char *pparam, *pname, *ptype;
	char *temp;
	struct st_method_ll *m;

	PJNI_ASSERT_RETURN(str != NULL, NULL);

	pparam = pobject_method_str_get_param(str);
	PJNI_ASSERT_RETURN(pparam != NULL, NULL);

	if (pparam[0] != ')') {
		POBJECT_WARN_ENOSYS();
		return NULL;
	}

	m = malloc(sizeof(struct st_method_ll));
	PJNI_ASSERT_RETURN(m != NULL, NULL);
	memset(m, 0, sizeof(struct st_method_ll));

	pname = pobject_method_str_get_name(str);
	PJNI_ASSERT_RETURN(pname != NULL, NULL);

	pname = pobject_java_basename(pname);

	ptype = pobject_method_str_get_type(str);
	PJNI_ASSERT_RETURN(ptype != NULL, NULL);

	m->sign = malloc(strlen(str) + 1); /* FIXME */
	PJNI_ASSERT_RETURN(m->sign != NULL, NULL);
	*m->sign = '\000';

	strcat(m->sign, "(");
	strcat(m->sign, ")");

#define CATTYPE(t, m, cat) case POBJECT_TYPE_##t: strcat(m->sign, cat); m->rettype = POBJECT_TYPE_##t; break
	switch (pobject_str_get_type(ptype)) {
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
			m->rettype = POBJECT_TYPE_JCLASS;
			break;
	}

	/* FIXME */
	temp = strdup(m->sign);
	if (temp != NULL) {
		free(m->sign);
		m->sign = temp;
	}

	m->name = malloc(pparam - 1 - pname + 1); /* FIXME */
	PJNI_ASSERT_RETURN(m->name != NULL, NULL);
	*m->name = '\000';
	strncat(m->name, pname, pparam - 1 - pname);

	return m;
}

static bool
pobject_method_str_is_from_java(const char *str)
{
	const char *pname;

	PJNI_ASSERT_RETURN(str != NULL, FALSE);

	pname = pobject_method_str_get_name(str);
	PJNI_ASSERT_RETURN(pname != NULL, FALSE);

	if ((strncmp(pname, "java.", 5) == 0) || (strncmp(pname, "javax.", 6) == 0))
		return TRUE;

	return FALSE;
}

static bool
pobject_method_str_void_param(const char *str)
{
	const char *pparam;

	PJNI_ASSERT_RETURN(str != NULL, FALSE);

	pparam = pobject_method_str_get_param(str);
	PJNI_ASSERT_RETURN(pparam != NULL, FALSE);

	if (pparam[0] == ')')
		return TRUE;
	return FALSE;
}

static bool
pobject_method_str_void_return(const char *str)
{
	const char *ptype;

	PJNI_ASSERT_RETURN(str != NULL, FALSE);

	ptype = pobject_method_str_get_type(str);
	PJNI_ASSERT_RETURN(ptype != NULL, FALSE);

	if (strncmp(ptype, "void ", 5) == 0)
		return TRUE;
	return FALSE;
}

static int
pobject_get_method_sign_list(JNIEnv *jenv, jclass cls, struct st_method_ll **mll)
{
	jobject strObj;
	jmethodID midGetClass;
	jobject clsObj;
	jclass jCls;
	jmethodID midGetFields;
	jobjectArray jobjArray;
	jsize len;
	jsize i;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(cls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(mll != NULL, POBJECT_ERROR_EINVAL);

	*mll = NULL;

#define LAME_ASSERT(x) do {						\
	if (!(x)) {							\
		fprintf(stderr, "%s:%d: LAME_ASSERT\n",			\
		    __FILE__, __LINE__);				\
		return POBJECT_ERROR_EINVAL;				\
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

		if (!pobject_method_str_is_from_java(str) &&
		    !pobject_method_str_void_return(str) &&
		    pobject_method_str_void_param(str)) {
			m = pobject_method_str_to_m(str);
			if (m != NULL) {
				pobject_java_convert_str(m->sign, '.', '/');
				m->jmid = (*jenv)->GetMethodID(jenv, cls, m->name, m->sign);
				/* printf("%s: %s%s (%#010x) (%s)\n", __FUNCTION__, m->name, m->sign, (unsigned int) m->jmid, str); */
				if ((m->jmid == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
					fprintf(stderr, "Could not get method id at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
					if ((*jenv)->ExceptionCheck(jenv)) {
						(*jenv)->ExceptionDescribe(jenv);
						(*jenv)->ExceptionClear(jenv);
					}
					fflush(stderr);

					return POBJECT_ERROR_EINVAL;
				}

				llappend((void **) mll, (void *) m);
			}
		}

		fflush(stdout);
		(*jenv)->ReleaseStringUTFChars(jenv, _name, str);

		(*jenv)->DeleteLocalRef(jenv, _name);
	}

	(*jenv)->DeleteLocalRef(jenv, jobjArray);

	return POBJECT_SUCCESS;
}

static int
pobject_fetch_property_boolean(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	bool val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (bool) (*jenv)->CallBooleanMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_BOOL(val);

	return ret;
}

static int
pobject_fetch_property_byte(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	char val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (char) (*jenv)->CallByteMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_BYTE(val);

	return ret;
}

static int
pobject_fetch_property_char(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	char val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (char) (*jenv)->CallCharMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_CHAR(val);

	return ret;
}

static int
pobject_fetch_property_short(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	short val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (short) (*jenv)->CallShortMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_SHORT(val);

	return ret;
}

static int
pobject_fetch_property_int(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	int val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (int) (*jenv)->CallIntMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	
	DEBUG_STR(m->name);
	DEBUG_INT(val);

	return ret;
}

static int
pobject_fetch_property_long(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	long val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (long) (*jenv)->CallLongMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_LONG(val);

	return ret;
}

static int
pobject_fetch_property_float(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	float val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (float) (*jenv)->CallFloatMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_DOUBLE(val);

	return ret;
}

static int
pobject_fetch_property_double(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	double val;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	val = (double) (*jenv)->CallDoubleMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);
		return POBJECT_ERROR_EXCEPTION;
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) &val);
	DEBUG_DOUBLE(val);

	return ret;
}

static int
pobject_fetch_property_string(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	jstring jstr;
	const char *str;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	jstr = (jstring)(*jenv)->CallObjectMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);

		if (jstr != NULL)
			(*jenv)->DeleteLocalRef(jenv, jstr);

		return POBJECT_ERROR_EXCEPTION;
	}

	DEBUG_STR(m->name);

	if (jstr == NULL) {
		DEBUG_STR("Deu STR = NULL");
		str = NULL;	
	} else {
		str = (*jenv)->GetStringUTFChars(jenv, jstr, 0);
		if (str == NULL) {
			(*jenv)->DeleteLocalRef(jenv, jstr);
			PJNI_ASSERT_RETURN(str != NULL, POBJECT_ERROR_EINVAL);
		}
	}

	ret = pobject_set_property(p, m->name, m->rettype, (void *) str);
	DEBUG_STR(str);

	if (jstr != NULL) {
		(*jenv)->ReleaseStringUTFChars(jenv, jstr, str);
		/*free(str);*/
	}

	/* Está certo rodar o DeleteLocalRef() com ponteiro NULL, NÃO
	 * APAGUE! Se isto não for feito, acontece um JNI leak. */
	(*jenv)->DeleteLocalRef(jenv, jstr);
	
	return ret;
}

static int
pobject_fetch_property_pobject(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	int ret;
	struct st_pobject *pnew;
		jobject jnew;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	jnew = (*jenv)->CallObjectMethod(jenv, j, m->jmid);
	if ((*jenv)->ExceptionCheck(jenv)) {
		(*jenv)->ExceptionDescribe(jenv);
		(*jenv)->ExceptionClear(jenv);
		fflush(stderr);

		if (jnew != NULL);
			(*jenv)->DeleteLocalRef(jenv, jnew);

		return POBJECT_ERROR_EXCEPTION;
	}

	if (jnew == NULL) {
		DEBUG_STR("Deu POBJECT = NULL");
		pnew = NULL;
		ret = pobject_set_property(p, m->name, POBJECT_TYPE_POBJECT, (void *) pnew);
		return ret;
	}

	pnew = malloc(sizeof(struct st_pobject));
	if (pnew == NULL) {
		(*jenv)->DeleteLocalRef(jenv, jnew);
		PJNI_ASSERT_RETURN(pnew != NULL, POBJECT_ERROR_ENOMEM);
	}

	ret = pobject_j2p(jenv, jnew, pnew);
	(*jenv)->DeleteLocalRef(jenv, jnew);
	if (ret == POBJECT_SUCCESS)
		ret = pobject_set_property(p, m->name, POBJECT_TYPE_POBJECT, (void *) pnew);
	else
		pobject_free(pnew);
	free(pnew); /* This pointer has already been used. Free it. */

	return ret;
}

static int
pobject_fetch_property(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *m)
{
	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(m != NULL, POBJECT_ERROR_EINVAL);

	switch (m->rettype) {
		case POBJECT_TYPE_BOOLEAN:
			return pobject_fetch_property_boolean(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_BYTE:
			return pobject_fetch_property_byte(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_CHAR:
			return pobject_fetch_property_char(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_SHORT:
			return pobject_fetch_property_short(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_INT:
			return pobject_fetch_property_int(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_LONG:
			return pobject_fetch_property_long(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_FLOAT:
			return pobject_fetch_property_float(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_DOUBLE:
			return pobject_fetch_property_double(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_STRING:
			return pobject_fetch_property_string(jenv, jcls, j, p, m);
			break;
		case POBJECT_TYPE_JCLASS:
			return pobject_fetch_property_pobject(jenv, jcls, j, p, m);
			break;
		default:
			break;
	}

	POBJECT_WARN_ENOSYS();
	return POBJECT_ERROR_ENOSYS;
}

/*
 * Preenche o "p"(estrutura C) utilizando o "j" como fonte, baseado na lista "mll".
 */
static int
pobject_fetch_property_list(JNIEnv *jenv, jclass jcls, jobject j, struct st_pobject *p, const struct st_method_ll *mll)
{
	int ret;

	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(jcls != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(mll != NULL, POBJECT_ERROR_EINVAL);

	for (; mll != NULL; mll = (struct st_method_ll *) mll->ll.next) {
		ret = pobject_fetch_property(jenv, jcls, j, p, mll);
		if (ret != POBJECT_SUCCESS) {
			return ret;
		}
	}

	return POBJECT_SUCCESS;
}

/** TODO Converte o objeto Java (j) em uma estrutura "st_pobject"(p). */
int
pobject_j2p(JNIEnv *jenv, jobject j, struct st_pobject *p)
{
	jclass jcls;
	char *str;
	int ret;
	struct st_method_ll *mll;


	PJNI_ASSERT_RETURN(jenv != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(j != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);

	memset(p, 0, sizeof(struct st_pobject));

	/*
	 * Pega classe java que foi passada
	 */
	jcls = (*jenv)->GetObjectClass(jenv, j);
	if ((jcls == NULL) || (*jenv)->ExceptionCheck(jenv)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Could not get object class at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		if (jcls != NULL)
			(*jenv)->DeleteLocalRef(jenv, jcls);

		return POBJECT_ERROR_EINVAL;
	}

	/*
	 * Extrai a assinatura da classe java
	 */
	str = pobject_get_jclass_name(jenv, jcls, j);
	if (str == NULL) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Could not get class name at \"%s:%d\".\n", __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);

		return POBJECT_ERROR_EINVAL;
	}

	/* 
	 * Inicia a estrutura com a assinatura da classe.
	 */
	ret = pobject_init(p, str);
	free(str);

	if (ret != POBJECT_SUCCESS) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Error %s while initializing pobject structure at \"%s:%d\".\n", pobject_get_error_name(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);
		
		pobject_free(p);
		return ret;
	}

	/*
	 * Lista em mll os metodos sem parametros e que nao retornam void
	 * da classe jcls (os getters).
	 */
	ret = pobject_get_method_sign_list(jenv, jcls, &mll);
	if ((ret != POBJECT_SUCCESS) || (mll == NULL)) {
#ifdef POBJECT_DEBUG_ERROR
		if (mll == NULL)
			fprintf(stderr, "Found no methods from jobject at \"%s:%d\".\n", __FILE__, __LINE__);
		else
			fprintf(stderr, "Error %s while getting method list from jobject at \"%s:%d\".\n", pobject_get_error_name(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

		(*jenv)->DeleteLocalRef(jenv, jcls);
		pobject_free(p);
		pobject_free_method_ll(&mll);

		return ret;
	}

	/*
	 * Preenche o "p"(estrutura C) utilizando o "j" como fonte, baseado na lista "mll".
	 */
	ret = pobject_fetch_property_list(jenv, jcls, j, p, mll);
/*	pobject_free_method_ll(&mll);*/
	(*jenv)->DeleteLocalRef(jenv, jcls);

	if ((ret != POBJECT_SUCCESS) || (mll == NULL)) {
#ifdef POBJECT_DEBUG_ERROR
		fprintf(stderr, "Error %s while getting method list from jobject at \"%s:%d\".\n", pobject_get_error_name(ret), __FILE__, __LINE__);
#endif
		if ((*jenv)->ExceptionCheck(jenv)) {
			(*jenv)->ExceptionDescribe(jenv);
			(*jenv)->ExceptionClear(jenv);
		}
		fflush(stderr);

	pobject_free_method_ll(&mll);
		pobject_free(p);

		return ret;
	}
	
pobject_free_method_ll(&mll);

	return POBJECT_SUCCESS;
}

/** TODO Lê a propriedade de uma estrutura "st_pobject". 
 *  passa soh o ponteiro 
 */
int
pobject_get_property(struct st_pobject *p, char *getter, enum e_pobject_type data_type, void **data)
{
	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(getter != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(data != NULL, POBJECT_ERROR_EINVAL);

	POBJECT_WARN_ENOSYS();
	return POBJECT_ERROR_ENOSYS;
}

/* TODO le dados da estrutura "st_pobject" e copia em "data"
 * duplica os dados (aloca memoria) */
int
pobject_get_property_copy(struct st_pobject *p, char *getter, enum e_pobject_type data_type, void **buf)
{
	struct st_pobject_property_ll *reg_atual;
	size_t data_size;

	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(getter != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(buf != NULL, POBJECT_ERROR_EINVAL);

DEBUG_STR("copy");
DEBUG_STR(getter);
DEBUG_STR(p->clazz);
	
	/*  varre lista e compara "getter" */
	for (reg_atual = p->properties;
	    reg_atual != NULL;
	    reg_atual = (struct st_pobject_property_ll *)reg_atual->ll.next) {
		if (strncmp(reg_atual->st.method_name, getter, 255) == 0) 
			break;
	}

	if (reg_atual == NULL)
		return POBJECT_ERROR_NOTFOUND;

	/* para casos de string ou objeto nulo */
	if (data_type == POBJECT_TYPE_STRING || data_type == POBJECT_TYPE_POBJECT) {
		if (reg_atual->st.data == NULL) {
			*buf = NULL;
			return POBJECT_SUCCESS;			
		}
	}

	if (data_type == POBJECT_TYPE_STRING) {
		data_size = strlen((char *) reg_atual->st.data) + 1;
	} else {
		data_size = pobject_get_type_size(data_type);
		PJNI_ASSERT_RETURN(data_size != POBJECT_ERROR_EINVAL, POBJECT_ERROR_EINVAL);
	}

	PJNI_ASSERT_RETURN(data_size > 0, POBJECT_ERROR_EINVAL);
	*buf = malloc(data_size);
	if (*buf == NULL) {
		p->error = POBJECT_ERROR_ENOMEM;
		return POBJECT_ERROR_ENOMEM;
	}

	memcpy(*buf, reg_atual->st.data, data_size);

	if (data_type == POBJECT_TYPE_STRING) {
		((char *)*buf)[data_size-1] = '\000';
DEBUG_STR((char *)*buf);
	} else
DEBUG_PTR(*buf);
	 
	return POBJECT_SUCCESS;			
}

/* 
 * grava no buffer passado(buf) de tamanho "buf_size", e sem alocar nada, o 
 * dado da estrutura "st_pobject", retornado pelo "getter"
 */ 
int
pobject_get_property_buf(struct st_pobject *p, char *getter, enum e_pobject_type data_type, void *buf, size_t buf_size)
{
	struct st_pobject_property_ll *reg_atual;

	PJNI_ASSERT_RETURN(p != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(getter != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(buf != NULL, POBJECT_ERROR_EINVAL);
	PJNI_ASSERT_RETURN(buf_size > 0, POBJECT_ERROR_EINVAL);

DEBUG_STR("buf");
DEBUG_STR(getter);
DEBUG_STR(p->clazz);

	if (data_type != POBJECT_TYPE_STRING) {
		PJNI_ASSERT_RETURN(pobject_get_type_size(data_type) != POBJECT_ERROR_EINVAL, POBJECT_ERROR_EINVAL);
		PJNI_ASSERT_RETURN(pobject_get_type_size(data_type) <= buf_size, POBJECT_ERROR_EINVAL);
		buf_size = pobject_get_type_size(data_type);
	}

	/*  varre lista e compara "getter" */
	for (reg_atual = p->properties;
	    reg_atual != NULL;
	    reg_atual = (struct st_pobject_property_ll *)reg_atual->ll.next) {
		if (strncmp(reg_atual->st.method_name, getter, 255) == 0) 
			break;
	}

	if (reg_atual == NULL)
		return POBJECT_ERROR_NOTFOUND;

	/* para casos de string ou objeto nulo */
	if (data_type == POBJECT_TYPE_STRING || data_type == POBJECT_TYPE_POBJECT) {
		if (reg_atual->st.data == NULL) {
			/* buf = NULL;*/
			memset (buf, 0, buf_size);
			return POBJECT_SUCCESS;			
		}
	}
	
	if (data_type == POBJECT_TYPE_STRING) {
		/* ajusta o buf */
		if ((strlen((char *) reg_atual->st.data) + 1) < buf_size)
			buf_size = strlen((char *) reg_atual->st.data) + 1;
	}

	memcpy(buf, reg_atual->st.data, buf_size);
	if (data_type == POBJECT_TYPE_STRING) {
		((char *)buf)[buf_size-1] = '\000';
DEBUG_STR((char *)buf);
	} else
DEBUG_INT(*((int *)buf));
	 
	return POBJECT_SUCCESS;			
}

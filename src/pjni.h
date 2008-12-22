#if !defined(_PJNI_H_)
#define _PJNI_H_

#include <errno.h> 
#include <jni.h>

#include "llist.h"

enum e_pobject_type {
	POBJECT_TYPE_STRING = 1,
	POBJECT_TYPE_INT = 2,
	POBJECT_TYPE_UINT = 3,
	POBJECT_TYPE_DOUBLE = 4,
	POBJECT_TYPE_FLOAT = 5,
	POBJECT_TYPE_LONG = 6,
	POBJECT_TYPE_ULONG = 7,
	POBJECT_TYPE_BOOLEAN = 8,
	POBJECT_TYPE_BYTE = 9,
	POBJECT_TYPE_CHAR = 10,
	POBJECT_TYPE_SHORT = 11,
	POBJECT_TYPE_VOID = 12,
	POBJECT_TYPE_POBJECT = 13,
	POBJECT_TYPE_JCLASS = 14
};

enum e_pobject_error {
	POBJECT_SUCCESS = 0,
	POBJECT_ERROR_INTERNAL = -1,		/* Erro interno. */
	POBJECT_ERROR_ENOMEM = -2,		/* Falta de memoria. */
	POBJECT_ERROR_EINVAL = -3,		/* Argumento inv�lido. */
	POBJECT_ERROR_EDEADLK = -4,		/* Deadlock. */
	POBJECT_ERROR_ENOSYS = -5,		/* Funcionalidade n�o implementada. */
	POBJECT_ERROR_NOJCLASS = -6,		/* Classe inexistente. */
	POBJECT_ERROR_NOTFOUND = -7,		/* N�o encontrado. */
	POBJECT_ERROR_EXCEPTION = -8		/* Exception recebida. */
};

/**
 * Estrutura de dados utilizada na cria��o de objetos Java.
 *
 * Esta estrutura n�o deve ser acessada diretamente, mas sim utilizando as
 * APIs pobject (pobject_init(), pobject_append(), pobject_send()).
 */
struct st_pobject {
	/** Classe Java do objeto. */
	char *clazz;

	/** Lista ligada de propriedades do objeto. */
	struct st_pobject_property_ll *properties;

	/**
	 * Indicador de erro. Caso esta vari�vel seja diferente de
	 * "POBJECT_SUCCESS", nenhuma API aceitar� esta estrutura como
	 * par�metro e todas retornar�o um valor de erro, ou seja, ela dever�
	 * ser liberada imediatamente com a fun��o pobject_free().
	 */
	 int error;
};

struct st_pobject_property {
	/**
	 * Indicador de dado alocado dinamicamente. Caso seja JNI_TRUE, a
	 * fun��o pobject_free() ir� rodar o free() no ponteiro data ao
	 * liberar esta estrutura.
	 */
	jboolean freeme;

	/** M�todo "setter" do objeto Java � ser chamado. */
	char *method_name;
#if 0
	/** Campo do objeto Java � ser configurado. */
	char *field; /* TODO implementar */
#endif

	/** Tipo do dado contido no n�. */
	enum e_pobject_type data_type;
	/** Dado do n�. */
	void *data;
};

struct st_pobject_property_ll {
	struct st_llist ll;
	struct st_pobject_property st;
};

/**
 * Inicializa a biblioteca PJNI.
 * @param version A vers�o da biblioteca PJNI.
 */
jboolean pjni_init(unsigned int version);

/**
 * Emite uma exception.
 * @param clazz A classe da exception.
 */ 
void pjni_throw(char *clazz);

/** Retorna os nomes dos tipos. */
const char *pobject_get_type_name(enum e_pobject_type t);
/** Retorna os nomes dos erros. */
const char *pobject_get_error_name(enum e_pobject_error t);

/**
 * Inicializa uma estrutura "st_pobject".
 *
 * Esta fun��o deve ser utilizada antes de configurar as propriedades do objeto.
 *
 * @param o Ponteiro para a estrutura "st_pobject".
 * @param clazz Nome da classe � ser criada.
 *
 * @return Um valor do enumerador "e_pobject_error".
 */
int pobject_init(struct st_pobject *o, char *clazz);

/**
 * Configura a propriedade de uma estrutura "st_pobject".
 *
 * @param o Ponteiro para a estrutura "st_pobject".
 * @param setter Nome da propriedade � ser configurada.
 * @param data_type Enumerador do tipo de dado da propriedade � ser configurada.
 * @param data Ponteiro para o dado da configura��o. TODO comentar sobre string
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
int pobject_set_property(struct st_pobject *p, const char *setter, enum e_pobject_type data_type, const void *orig_data);

/**
 * Pega a propriedade de uma estrutura "st_pobject".
 *
 * @param o Ponteiro para a estrutura "st_pobject".
 * @param getter Nome da propriedade � ser configurada.
 * @param data_type Enumerador do tipo de dado da propriedade � ser configurada.
 * @param data Ponteiro para o dado da configura��o. TODO comentar sobre string
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
/* passa soh o ponteiro */
int pobject_get_property(struct st_pobject *o, char *getter, enum e_pobject_type data_type, void **data);
/* duplica os dados (aloca memoria) */
int pobject_get_property_copy(struct st_pobject *o, char *getter, enum e_pobject_type data_type, void **buf);
/* grava no buffer passado */
int pobject_get_property_buf(struct st_pobject *o, char *getter, enum e_pobject_type data_type, void *data, size_t data_size);

/**
 * Converte a estrutura "st_pobject" em um objeto Java (jobject) e o envia � uma fun��o est�tica de uma classe Java.
 *
 * @param o Ponteiro para a estrutura "st_pobject".
 * @param clazz Nome da classe receptora.
 * @param method Nome do m�todo receptor.
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
int pobject_send(JNIEnv *jenv, struct st_pobject *p, const char *clazz, const char *method);

/**
 * Libera (desaloca) a estrutura de dados "st_pobject".
 *
 * @param o Ponteiro para a estrutura "st_pobject".
 */
void pobject_free(struct st_pobject *o);

/**
 * Verifica o estado de erro ocorrido na estrutura "st_pobject".
 *
 * @return Um valor do enumerador "e_pobject_error".
 */
int pobject_error(struct st_pobject *o);

/**
 * Converte uma estrutura "st_pobject" em uma "jobject".
 *
 * @return Um valor do enumerador "e_pobject_error".
 */
int pobject_p2j(JNIEnv *jenv, struct st_pobject *p, jobject *j);

/**
 * Converte uma estrutura "jobject" em uma "st_pobject".
 *
 * @return Um valor do enumerador "e_pobject_error".
 */
int pobject_j2p(JNIEnv *jenv, jobject j, struct st_pobject *p);

/**
 * Inicia a thread principal da aplica��o C.
 */
int start_main_thread(void);

#endif /* !defined(_PJNI_H_) */

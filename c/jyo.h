/*
 * Copyright 2005-2014 Fernando Silveira <fsilveira@gmail.com>
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

#if !defined(_JYO_H_)
#define _JYO_H_

#include <jni.h>

#include "jnyikes.h"
#include "llist.h"

enum e_jyo_type {
	JYO_TSTRING	=  1,
	JYO_TINT	=  2,
	JYO_TUINT	=  3,
	JYO_TDOUBLE	=  4,
	JYO_TFLOAT	=  5,
	JYO_TLONG	=  6,
	JYO_TULONG	=  7,
	JYO_TBOOLEAN	=  8,
	JYO_TBYTE	=  9,
	JYO_TCHAR	= 10,
	JYO_TSHORT	= 11,
	JYO_TVOID	= 12,
	JYO_TJYO	= 13,
	JYO_TJCLASS	= 14,
};

/**
 * XXX
 *
 * Estrutura de dados utilizada na criação de objetos Java.
 *
 * Esta estrutura não deve ser acessada diretamente, mas sim utilizando as
 * APIs jyo (jyo_init(), jyo_append(), jyo_send()).
 */
struct st_jyo {
	/** XXX Classe Java do objeto. */
	char *clazz;

	/** XXX Lista ligada de propriedades do objeto. */
	struct st_jyo_property_ll *properties;

	/**
	 * XXX Indicador de erro. Caso esta variável seja diferente de
	 * "JY_ESUCCESS", nenhuma API aceitará esta estrutura como
	 * parâmetro e todas retornarão um valor de erro, ou seja, ela deverá
	 * ser liberada imediatamente com a função jyo_free().
	 */
	enum e_jy_err error;
};

struct st_jyo_property {
	/**
	 * XXX Indicador de dado alocado dinamicamente. Caso seja JNI_TRUE, a
	 * função jyo_free() irá rodar o free() no ponteiro data ao
	 * liberar esta estrutura.
	 */
	jboolean freeme;

	/** XXX Método "setter" do objeto Java à ser chamado. */
	char *method_name;
#if 0
	/** XXX Campo do objeto Java à ser configurado. */
	char *field; /* XXX TODO implementar */
#endif

	/** XXX Tipo do dado contido no nó. */
	enum e_jyo_type data_type;
	/** XXX Dado do nó. */
	void *data;
};

struct st_jyo_property_ll {
	struct st_llist ll;
	struct st_jyo_property st;
};

/**
 * XXX Inicializa a biblioteca jnyikes.
 * @param version A versão da biblioteca jnyikes.
 */
jboolean jnyikes_init(unsigned int version);

/**
 * XXX Emite uma exception.
 * @param clazz A classe da exception.
 */
void jnyikes_throw(char *clazz);

/** XXX Retorna os nomes dos tipos. */
const char *jyo_get_type_name(enum e_jyo_type t);

/**
 * XXX Inicializa uma estrutura "st_jyo".
 *
 * Esta função deve ser utilizada antes de configurar as propriedades do objeto.
 *
 * @param o Ponteiro para a estrutura "st_jyo".
 * @param clazz Nome da classe à ser criada.
 *
 * @return Um valor do enumerador "e_jy_err".
 */
int jyo_init(struct st_jyo *o, char *clazz);

/**
 * XXX Configura a propriedade de uma estrutura "st_jyo".
 *
 * @param o Ponteiro para a estrutura "st_jyo".
 * @param setter Nome da propriedade à ser configurada.
 * @param data_type Enumerador do tipo de dado da propriedade à ser configurada.
 * @param data Ponteiro para o dado da configuração. TODO comentar sobre string
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
int jyo_set_property(struct st_jyo *p, const char *setter, enum e_jyo_type data_type, const void *orig_data);

/**
 * XXX Pega a propriedade de uma estrutura "st_jyo".
 *
 * @param o Ponteiro para a estrutura "st_jyo".
 * @param getter Nome da propriedade à ser configurada.
 * @param data_type Enumerador do tipo de dado da propriedade à ser configurada.
 * @param data Ponteiro para o dado da configuração. TODO comentar sobre string
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
/* XXX passa soh o ponteiro */
int jyo_get_property(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void **data);
/* XXX duplica os dados (aloca memoria) */
int jyo_get_property_copy(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void **buf);
/* XXX grava no buffer passado */
int jyo_get_property_buf(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void *data, size_t data_size);

/**
 * XXX Converte a estrutura "st_jyo" em um objeto Java (jobject) e o envia à uma função estática de uma classe Java.
 *
 * @param o Ponteiro para a estrutura "st_jyo".
 * @param clazz Nome da classe receptora.
 * @param method Nome do método receptor.
 *
 * @return -1 em caso de erro e 0 para sucesso.
 */
int jyo_send(JNIEnv *jenv, struct st_jyo *p, const char *clazz, const char *method);

/**
 * XXX Libera (desaloca) a estrutura de dados "st_jyo".
 *
 * @param o Ponteiro para a estrutura "st_jyo".
 */
void jyo_free(struct st_jyo *o);

/**
 * XXX Verifica o estado de erro ocorrido na estrutura "st_jyo".
 *
 * @return Um valor do enumerador "e_jy_err".
 */
int jyo_error(struct st_jyo *o);

/**
 * XXX Converte uma estrutura "st_jyo" em uma "jobject".
 *
 * @return Um valor do enumerador "e_jy_err".
 */
int jyo_p2j(JNIEnv *jenv, struct st_jyo *p, jobject *j);

/**
 * XXX Converte uma estrutura "jobject" em uma "st_jyo".
 *
 * @return Um valor do enumerador "e_jy_err".
 */
int jyo_j2p(JNIEnv *jenv, jobject j, struct st_jyo *p);

#endif /* !defined(_JYO_H_) */

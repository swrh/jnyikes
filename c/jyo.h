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
 * Data structure used in the creation of Java objects.
 *
 * This structure should be used using the jyo APIs (jyo_init(), jyo_append(),
 * jyo_send()) and not directly.
 */
struct st_jyo {
	/** The Java class. */
	char *clazz;

	/** The object properties contained in a linked list. */
	struct st_jyo_property_ll *properties;

	/**
	 * Error indicator. If this variable is different than "JY_ESUCCESS",
	 * no API will ever accept this structure as a parameter and all will
	 * return an error value/state. In this case it should be freed
	 * immediately using the jyo_free() function.
	 */
	enum e_jy_err error;
};

struct st_jyo_property {
	/**
	 * Dynamically allocated data flag. If this variable is JNI_TRUE, the
	 * jyo_free() function will free(3) this structure.
	 */
	jboolean freeme;

	/** The setter method name of the Java object to be called. */
	char *method_name;
#if 0
	/** The Java object field to be configured. */
	char *field; /* XXX TODO: implement. */
#endif

	/** The type of the data contained in this node. */
	enum e_jyo_type data_type;
	/** The data of this node. */
	void *data;
};

struct st_jyo_property_ll {
	struct st_llist ll;
	struct st_jyo_property st;
};

/**
 * Initialize the jnyikes library.
 * @param version The version which the application expects.
 */
jboolean jnyikes_init(unsigned int version);

/**
 * Throws/emits an exception.
 * @param clazz The exception class.
 */
void jnyikes_throw(char *clazz);

/**
 * Returns the string description of a type.
 */
const char *jyo_get_type_name(enum e_jyo_type t);

/**
 * Initializes a "st_jyo" struct.
 *
 * This functions should be used before setting up the object properties.
 *
 * @param o The pointer to the "st_jyo" struct.
 * @param clazz The name of the class to be created.
 *
 * @return A "e_jy_err" error code.
 */
int jyo_init(struct st_jyo *o, char *clazz);

/**
 * Sets up the property of an "st_jyo" struct.
 *
 * @param o The pointer to the "st_jyo" struct.
 * @param setter The name of the property to be set.
 * @param data_type The data type enum of the data to be set.
 * @param data The pointer to the data to be set.
 *
 * @return -1 in case of error and 0 for success.
 */
int jyo_set_property(struct st_jyo *p, const char *setter, enum e_jyo_type data_type, const void *orig_data);

/**
 * Gets the pointer of the data of a "st_jyo" struct returned by "getter".
 *
 * @param o The pointer to the "st_jyo" struct.
 * @param getter Name of the property to be fetched.
 * @param data_type The data type enum of the data.
 * @param data The variable where the pointer will be returned.
 *
 * @return -1 in case of error and 0 for success.
 */
int jyo_get_property(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void **data);

/**
 * Duplicates the data of a "st_jyo" struct returned by "getter".
 *
 * @param o The pointer to the "st_jyo" struct.
 * @param getter Name of the property to be fetched.
 * @param data_type The data type enum of the data to be read.
 * @param buf The pointer where the data will be duplicated.
 *
 * @return -1 in case of error and 0 for success.
 */
int jyo_get_property_copy(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void **buf);

/**
 * Reads the data of a "st_jyo" struct returned by "getter" into a passed buffer.
 *
 * @param o The pointer to the "st_jyo" struct.
 * @param getter Name of the property to be fetched.
 * @param data_type The data type enum of the data to be read.
 * @param data The pointer of the buffer to store the data.
 * @param data_size The size of the "data" buffer.
 *
 * @return -1 in case of error and 0 for success.
 */
int jyo_get_property_buf(struct st_jyo *o, char *getter, enum e_jyo_type data_type, void *data, size_t data_size);

/**
 * Converts the "st_jyo" struct to a Java object (jobject) and send it to a
 * Java static method of a defined class.
 *
 * @param o Pointer to the "st_jyo" struct.
 * @param clazz Name of the receiving class.
 * @param method Name of the receiving method.
 *
 * @return -1 in case of error and 0 for success.
 */
int jyo_send(JNIEnv *jenv, struct st_jyo *p, const char *clazz, const char *method);

/**
 * Free the "st_jyo" struct.
 *
 * @param o Pointer to the "st_jyo" struct.
 */
void jyo_free(struct st_jyo *o);

/**
 * Gets the error state ocurred with the "st_jyo" struct.
 *
 * @return The "e_jy_err" error enumerator.
 */
int jyo_error(struct st_jyo *o);

/**
 * Converts an "st_jyo" struct into a "jobject".
 *
 * @return The "e_jy_err" error enumerator.
 */
int jyo_p2j(JNIEnv *jenv, struct st_jyo *p, jobject *j);

/**
 * Converts a "jobject" into an "st_jyo" struct.
 *
 * @return The "e_jy_err" error enumerator.
 */
int jyo_j2p(JNIEnv *jenv, jobject j, struct st_jyo *p);

#endif /* !defined(_JYO_H_) */

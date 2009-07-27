/*
 * Copyright 2005, 2009 Fernando Silveira <fsilveira@gmail.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <jni.h>

#include "pjni.h"

#include "interface.h"

#define L_PTHREAD_INITIALIZER ((pthread_t)0)

static JavaVM *g_jvm = NULL;
static pthread_t g_thread = L_PTHREAD_INITIALIZER;
static int g_thread_running = 0;
static pthread_mutex_t g_thread_mutex = PTHREAD_MUTEX_INITIALIZER;

struct thrparam_st {
	JavaVM *jvm;
	int (*thrfn)(JavaVM *, JNIEnv *);
};

/**
 * Stores JVM data pointer.
 *
 * This function is called automatically by the JVM when it is loaded.
 */
jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
#ifdef DEBUG
	printf("%s(%p, %p);\n", __func__, (void *)vm, reserved);
	fflush(stdout);
#endif

	g_jvm = vm;

	return JNI_VERSION_1_2;
}

/**
 * Unload JVM data pointer.
 *
 * This function is called automatically by the JVM when it is unloaded.
 */
void
JNI_OnUnload(JavaVM *vm, void *reserved)
{
#ifdef DEBUG
	printf("%s(%p, %p);\n", __func__, (void *) vm, reserved);
	fflush(stdout);
#endif

	g_jvm = NULL;
}

/**
 * Wrapper thread function - loads JVM configurations and runs the thread
 * function.
 *
 * This function shall be called by start_thread(). It will properly start a
 * separated thread and run the main function passed as argument.
 */
static void *
l_thread(void *param)
{
	int ret;
	struct thrparam_st *thrp;
	JavaVMAttachArgs thr_args;
	JNIEnv *jenv;
	JavaVM *jvm;

#ifdef DEBUG
	printf("%s(%p);\n", __func__, param);
	fflush(stdout);
#endif

	thrp = (struct thrparam_st *)param;

	/* Load JVM data pointer. */
	jvm = thrp->jvm;
	if (jvm == NULL) {
		fprintf(stderr, "%s(%p); Received an invalid JVM pointer.\n", __func__, param);
		fflush(stderr);

		/* Reduce our global thread counter. */
		(void)pthread_mutex_lock(&g_thread_mutex);
		g_thread_running--;
		(void)pthread_mutex_unlock(&g_thread_mutex);

		return NULL;
	}

	/* Conecta a thread atual à máquina virtual Java. */
	memset(&thr_args, 0, sizeof(JavaVMAttachArgs));
	thr_args.version = JNI_VERSION_1_2;
	thr_args.name = "CCorte";
	if ((*jvm)->AttachCurrentThread(jvm, (void *)&jenv, &thr_args) < 0) {
		fprintf(stderr, "%s(%p); Error while attaching current thread to the JVM.\n", __func__, param);
		fflush(stderr);

		/* Reduce our global thread counter. */
		(void)pthread_mutex_lock(&g_thread_mutex);
		g_thread_running--;
		(void)pthread_mutex_unlock(&g_thread_mutex);

		return NULL;
	}

	/* Start thread function. */
	ret = thrp->thrfn(jvm, jenv);

	/* Reduce our global thread counter. */
	(void)pthread_mutex_lock(&g_thread_mutex);
	g_thread_running--;
	(void)pthread_mutex_unlock(&g_thread_mutex);

	/* Descarrega as configurações do Java. */
	if ((*jvm)->DetachCurrentThread(jvm) < 0) {
		fprintf(stderr, "%s(%p); Error while dettaching current thread from the JVM.\n", __func__, param);
		fflush(stderr);
	}

	return (void *)ret;
}

/**
 * Starts a thread in a separated function.
 *
 * @param thrfn The function pointer to be executed in a separated thread.
 */
int
start_thread(int (*thrfn)(JavaVM *, JNIEnv *))
{
	pthread_attr_t attr;
	struct thrparam_st *param;
	int eno;
	void *ret;

	/* Lock our global variables mutex. */
	eno = pthread_mutex_lock(&g_thread_mutex);
	if (eno != 0) {
		errno = eno;
		return -1;
	}

	/* Check if there already is a thread running. */
	if (g_thread_running > 0) {
		(void)pthread_mutex_unlock(&g_thread_mutex);
		/* A thread is already running. */
		errno = EADDRINUSE;
		return -1;
	}
	/* No thread is currently running. */

	/* If there already was a thread running, join it. */
	if (g_thread != L_PTHREAD_INITIALIZER)
		(void)pthread_join(g_thread, &ret);
	g_thread = L_PTHREAD_INITIALIZER;

	/* Parameters to be passed to the application function. */
	param = malloc(sizeof(struct thrparam_st));
	if (param == NULL) {
		(void)pthread_mutex_unlock(&g_thread_mutex);
		return -1;
	}
	memset(param, 0, sizeof(struct thrparam_st));
	param->jvm = g_jvm;
	param->thrfn = thrfn;

	/* Start the thread. */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	eno = pthread_create(&g_thread, &attr, l_thread, (void *)param);
	if (eno != 0) {
		g_thread = L_PTHREAD_INITIALIZER;
		(void)pthread_mutex_unlock(&g_thread_mutex);
		free(param);
		/* Properly set the errno variable. */
		errno = eno;
		return -1;
	}

	/* Ok, now we have a thread running. */
	g_thread_running++;

	(void)pthread_mutex_unlock(&g_thread_mutex);

	return 0;
}

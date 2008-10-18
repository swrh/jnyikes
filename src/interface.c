#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <jni.h>

#include "pjni.h"

#include "interface.h"
#include "app.h"

static JavaVM *g_jvm = NULL;
static pthread_t g_main_thread;

/**
 * Guarda as informações sobre a máquina virtual Java.
 *
 * Função chamada pela JVM no carregamento da biblioteca.
 */
jint
JNI_OnLoad(JavaVM *vm, void *reserved)
{
#ifdef DEBUG
	printf("%s(%p, %p);\n", __FUNCTION__, (void *) vm, reserved);
	fflush(stdout);
#endif

	g_jvm = vm;

	return JNI_VERSION_1_2;
}

/**
 * Descarrega as informações da biblioteca.
 *
 * Função chamada pela JVM no descarregamento da biblioteca.
 */
void
JNI_OnUnload(JavaVM *vm, void *reserved)
{
#ifdef DEBUG
	printf("%s(%p, %p);\n", __FUNCTION__, (void *) vm, reserved);
	fflush(stdout);
#endif

	g_jvm = NULL;
}

/**
 * Thread wrapper para a função principal da aplicação C.
 *
 * Carrega as configurações da máquina virtual Java e executa a função C
 * app_main(). Ao invés de chamar esta função diretamente, a função
 * start_main_thread() deve ser executada.
 */
static void *
main_thread(void *param)
{
	JavaVMAttachArgs thr_args;
	JNIEnv *jenv;
	JavaVM *jvm;

#ifdef DEBUG
	printf("%s(%p);\n", __FUNCTION__, param);
	fflush(stdout);
#endif

	printf("Carregando thread C principal...\n");
	fflush(stdout);

	/* Carrega as configurações do Java. */
	jvm = (JavaVM *) param;
	if (jvm == NULL) {
		fprintf(stderr, "Ponteiro invalido para maquina virtual Java.\n");
		fflush(stderr);
		return NULL;
	}

	/* Conecta a thread atual à máquina virtual Java. */
	memset(&thr_args, 0, sizeof(JavaVMAttachArgs));
	thr_args.version = JNI_VERSION_1_2;
	thr_args.name = "CCorte";
	if ((*jvm)->AttachCurrentThread(jvm, (void *) &jenv, &thr_args) < 0) {
		fprintf(stderr, "Erro conectando a thread principal `a maquina virtual Java.\n");
		fflush(stderr);
		return NULL;
	}

	/*(void)pthread_mutex_lock(&g_main_thread_mutex);
	g_main_thread_running++;
	(void)pthread_mutex_unlock(&g_main_thread_mutex);*/

	printf("Thread C principal carregada. Continuando com a execucao da aplicacao C...\n");
	fflush(stdout);

	/* Rodar aplicação. */
	app_main(jvm, jenv);

	/*(void)pthread_mutex_lock(&g_main_thread_mutex);
	g_main_thread_running--;
	(void)pthread_mutex_unlock(&g_main_thread_mutex);*/

	printf("Aplicacao C finalizada. Fechando thread principal...\n");
	fflush(stdout);

	/* Descarrega as configurações do Java. */
	if ((*jvm)->DetachCurrentThread(jvm) < 0) {
		fprintf(stderr, "Erro desconectando a thread principal da maquina virtual Java.\n");
		fflush(stderr);
	}

	return NULL;
}

/**
 * Inicia a thread da aplicação C.
 */
int
start_main_thread(void)
{
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (pthread_create(&g_main_thread, &attr, main_thread, (void *) g_jvm) != 0)
		return -1;
	/* TODO esperar a main thread enviar um sinal de ok */

	return 0;
}

/*
	pobject_append(&o, g_type_float, "pi", &f);
	pobject_send(&o, "com/provectus/vicunha/corte/jni/objects/PEvento");
*/

/*
int
send_string(char *str)
{
	jmethodID mid;
	jclass cls;
	JNIEnv *env;
	jstring jstr;

	env = g_jnienv;

//	printf("%s(%p, \"%s\");\n", __FUNCTION__, env, str);
//	fflush(stdout);

	if (g_cback_clazz == NULL)
//	if ((g_cback_clazz == NULL) || (g_cback_method == NULL) || (g_cback_sign == NULL))
		return -1;

//	cls = (*env)->GetObjectClass(env, obj);
	cls = (*env)->FindClass(env, g_cback_clazz);
	if ((*env)->ExceptionOccurred(env)) {
//		printf("(*env)->FindClass();\n(*env)->ExceptionOccurred(env) = JNI_TRUE;\n");
//		fflush(stdout);
		(*env)->ExceptionDescribe(env);
		fflush(stderr);
		(*env)->ExceptionClear(env);
		return -1;
	}


//	mid = (*env)->GetMethodID(env, cls, "prvEntraEvento", "(Ljava/lang/String;)V");
//	mid = (*env)->GetMethodID(env, cls, "prvEntraEvento", "()V");
//	mid = (*env)->GetStaticMethodID(env, cls, "prvEntraEvento", "()V");
	mid = (*env)->GetStaticMethodID(env, cls, "prvEntraMensagem", "(Ljava/lang/String;)V");

//	printf("mid = %p;\n", mid);
	fflush(stdout);

	if ((*env)->ExceptionOccurred(env)) {
//		printf("(*env)->ExceptionOccurred(env) = JNI_TRUE;\n");
//		fflush(stdout);
		(*env)->ExceptionDescribe(env);
		fflush(stderr);
		(*env)->ExceptionClear(env);
		return -1;
	}

	if (mid == 0)
		return -1;

	jstr = (*env)->NewStringUTF(env, str);
	if ((*env)->ExceptionOccurred(env)) {
		(*env)->ExceptionDescribe(env);
		fflush(stderr);
		(*env)->ExceptionClear(env);
		if (jstr != NULL)
			(*env)->ReleaseStringUTFChars(env, jstr, str);
		return -1;
	} else if (jstr == NULL) {
		return -1;
	}

//	(*env)->CallObjectMethod(env, obj, mid);
	(*env)->CallStaticVoidMethod(env, cls, mid, jstr);
	if (jstr != NULL)
		(*env)->ReleaseStringUTFChars(env, jstr, str);

	return 0;
}
*/

/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class ai_madara_knowledge_Any */

#ifndef _Included_ai_madara_knowledge_Any
#define _Included_ai_madara_knowledge_Any
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     ai_madara_knowledge_Any
 * Method:    jni_construct
 * Signature: (Ljava/lang/String;[J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ai_madara_knowledge_Any_jni_1construct(
    JNIEnv*, jclass, jstring, jlongArray);

/*
 * Class:     ai_madara_knowledge_Any
 * Method:    jni_emplace_capnp
 * Signature: (Ljava/lang/String;[B[J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ai_madara_knowledge_Any_jni_1emplace_1capnp(
    JNIEnv*, jclass, jstring, jbyteArray, jlongArray);

/*
 * Class:     ai_madara_knowledge_Any
 * Method:    jni_free
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_ai_madara_knowledge_Any_jni_1free(
    JNIEnv*, jclass, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif

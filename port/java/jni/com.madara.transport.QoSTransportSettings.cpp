/*********************************************************************
 * Usage of this software requires acceptance of the SMASH-CMU License,
 * which can be found at the following URL:
 *
 * https://code.google.com/p/smash-cmu/wiki/License
 *********************************************************************/

#include "com.madara.transport.QoSTransportSettings.h"
#include "madara/transport/QoS_Transport_Settings.h"
#include "madara/filters/java/Java_Buffer_Filter.h"

#include <iostream>

// define useful shorthands
namespace engine = Madara::Knowledge_Engine;
namespace transport = Madara::Transport;
namespace filters = Madara::Filters;
typedef Madara::Knowledge_Record  Knowledge_Record;
typedef Knowledge_Record::Integer Integer;
typedef transport::QoS_Transport_Settings QoS_Transport_Settings;
typedef filters::Java_Buffer_Filter  Java_Buffer_Filter;
typedef filters::Buffer_Filter Buffer_Filter;

/*
 * Class:   com_madara_transport_QoSTransportSettings
 * Method:  jni_QoSTransportSettings
 * Signature: ()J
 */
jlong JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1QoSTransportSettings__
  (JNIEnv *, jobject)
{
  return (jlong) new QoS_Transport_Settings ();
}

/*
 * Class:   com_madara_transport_QoSTransportSettings
 * Method:  jni_QoSTransportSettings
 * Signature: (J)J
 */
jlong JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1QoSTransportSettings__J
  (JNIEnv *, jobject, jlong old)
{
  jlong result (0);
  QoS_Transport_Settings * source = (QoS_Transport_Settings *) old;

  if (source)
  {
    result = (jlong) new QoS_Transport_Settings (*source);
  }

  return result;
}

/*
 * Class:   com_madara_transport_QoSTransportSettings
 * Method:  jni_freeQoSTransportSettings
 * Signature: (J)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1freeQoSTransportSettings
  (JNIEnv * env, jclass cls, jlong cptr)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  delete settings;
}


/*
* Class:     com_madara_transport_QoSTransportSettings
* Method:    jni_addBufferFilter
* Signature: (JJ)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addBufferFilter
(JNIEnv *, jobject, jlong cptr, jlong filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;
  Buffer_Filter * buffer_filter = (Buffer_Filter *)filter;

  if (settings)
  {
    settings->add_filter (buffer_filter);
  }
}

/*
* Class:     com_madara_transport_QoSTransportSettings
* Method:    jni_addBufferFilterObj
* Signature: (JLcom/madara/filters/BufferFilter;)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addBufferFilterObj
(JNIEnv *, jobject, jlong cptr, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;
  Java_Buffer_Filter * buffer_filter = new Java_Buffer_Filter (filter);

  if (settings)
  {
    settings->add_filter (buffer_filter);
  }
}

/*
* Class:     com_madara_transport_QoSTransportSettings
* Method:    jni_clearBufferFilters
* Signature: (J)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1clearBufferFilters
(JNIEnv *, jobject, jlong cptr)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->clear_buffer_filters ();
  }
}

/*
* Class:     com_madara_transport_QoSTransportSettings
* Method:    jni_getNumberOfBufferFilters
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1getNumberOfBufferFilters
(JNIEnv *, jobject, jlong cptr)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;
  jint result (0);

  if (settings)
  {
    result = (jint)settings->get_number_of_buffer_filters ();
  }

  return result;
}


void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1saveQoS
(JNIEnv * env, jobject, jlong cptr, jstring filename)
{
  if (cptr && filename)
  {
    const char * nativeFilename = env->GetStringUTFChars (filename, 0);
    QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

    settings->save (nativeFilename);

    env->ReleaseStringUTFChars (filename, nativeFilename);
  }
}

void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1loadQoS
(JNIEnv * env, jobject, jlong cptr, jstring filename)
{
  if (cptr && filename)
  {
    const char * nativeFilename = env->GetStringUTFChars (filename, 0);
    QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

    settings->load (nativeFilename);

    env->ReleaseStringUTFChars (filename, nativeFilename);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addRebroadcastFilter
 * Signature: (JILcom/madara/transport/filters/RecordFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addRebroadcastFilter__JILcom_madara_transport_filters_RecordFilter_2
  (JNIEnv *, jobject, jlong cptr, jint type, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_rebroadcast_filter ( (uint32_t)type, filter);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addRebroadcastFilter
 * Signature: (JLcom/madara/transport/filters/AggregateFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addRebroadcastFilter__JLcom_madara_transport_filters_AggregateFilter_2
  (JNIEnv *, jobject, jlong cptr, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_rebroadcast_filter (filter);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addSendFilter
 * Signature: (JILcom/madara/transport/filters/RecordFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addSendFilter__JILcom_madara_transport_filters_RecordFilter_2
  (JNIEnv *, jobject, jlong cptr, jint type, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_send_filter ( (uint32_t)type, filter);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addSendFilter
 * Signature: (JLcom/madara/transport/filters/AggregateFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addSendFilter__JLcom_madara_transport_filters_AggregateFilter_2
  (JNIEnv *, jobject, jlong cptr, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_send_filter (filter);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addReceiveFilter
 * Signature: (JILcom/madara/transport/filters/RecordFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addReceiveFilter__JILcom_madara_transport_filters_RecordFilter_2
  (JNIEnv *, jobject, jlong cptr, jint type, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_receive_filter ( (uint32_t)type, filter);
  }
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addReceiveFilter
 * Signature: (JLcom/madara/transport/filters/AggregateFilter;)V
 */
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addReceiveFilter__JLcom_madara_transport_filters_AggregateFilter_2
  (JNIEnv *, jobject, jlong cptr, jobject filter)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->add_receive_filter (filter);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_setRebroadcastTtl
* Signature: (JI)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1setRebroadcastTtl
  (JNIEnv * env, jobject obj, jlong cptr, jint ttl)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->set_rebroadcast_ttl ( (unsigned char)ttl);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_getRebroadcastTtl
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1getRebroadcastTtl
  (JNIEnv * env, jobject obj, jlong cptr)
{
  jint result (0);
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    result = (jint) settings->get_rebroadcast_ttl ();
  }

  return result;
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_enableParticipantTtl
* Signature: (JI)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1enableParticipantTtl
  (JNIEnv * env, jobject obj, jlong cptr, jint ttl)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->enable_participant_ttl ( (unsigned char)ttl);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_getParticpantTtl
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1gettParticpantTtl (JNIEnv * env, jobject obj, jlong cptr)
{
  jint result (0);
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    result = (jint) settings->get_participant_ttl ();
  }

  return result;
}


/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_setSendBandwidthLimit
* Signature: (JI)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1setSendBandwidthLimit (JNIEnv * env, jobject obj, jlong cptr, jint limit)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->set_send_bandwidth_limit (limit);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_getSendBandwidthLimit
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1getSendBandwidthLimit (JNIEnv * env, jobject obj, jlong cptr)
{
  jint result (0);
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    result = (jint) settings->get_send_bandwidth_limit ();
  }

  return result;
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_setTotalBandwidthLimit
* Signature: (JI)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1setTotalBandwidthLimit
  (JNIEnv * env, jobject obj, jlong cptr, jint limit)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->set_total_bandwidth_limit (limit);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_getTotalBandwidthLimit
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1getTotalBandwidthLimit
  (JNIEnv * env, jobject obj, jlong cptr)
{
  jint result (0);
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    result = (jint) settings->get_total_bandwidth_limit ();
  }

  return result;
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_setDeadline
* Signature: (JI)V
*/
void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1setDeadline
  (JNIEnv * env, jobject obj, jlong cptr, jint limit)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->set_deadline (limit);
  }
}

/*
* Class:   com_madara_transport_QoSTransportSettings
* Method:  jni_getDeadline
* Signature: (J)I
*/
jint JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1getDeadline
  (JNIEnv * env, jobject obj, jlong cptr)
{
  jint result (0);
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    result = (jint) settings->get_deadline ();
  }

  return result;
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addTrustedPeer
 * Signature: (JLjava/lang/String;)V
 */
 void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addTrustedPeer
  (JNIEnv * env, jobject, jlong cptr, jstring jhost)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;
  const char * host = env->GetStringUTFChars (jhost, 0);

  if (settings)
  {
    settings->add_trusted_peer (host);
  }

  env->ReleaseStringUTFChars (jhost, host);
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_addBannedPeer
 * Signature: (JLjava/lang/String;)V
 */
 void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1addBannedPeer
  (JNIEnv * env, jobject, jlong cptr, jstring jhost)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;
  const char *host = env->GetStringUTFChars (jhost, 0);

  if (settings)
  {
    settings->add_banned_peer (host);
  }

  env->ReleaseStringUTFChars (jhost, host);
}

/*
 * Class:     com_madara_transport_QoSTransportSettings
 * Method:    jni_updateDropRate
 * Signature: (JDII)V
 */
 void JNICALL Java_com_madara_transport_QoSTransportSettings_jni_1updateDropRate
  (JNIEnv *, jobject, jlong cptr,
   jdouble drop_rate, jint drop_type, jint burstamount)
{
  QoS_Transport_Settings * settings = (QoS_Transport_Settings *)cptr;

  if (settings)
  {
    settings->update_drop_rate (drop_rate, (int)drop_type,
      (uint64_t)burstamount);
  }
}

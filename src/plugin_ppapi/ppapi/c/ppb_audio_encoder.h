/* Copyright 2015 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* From ppb_audio_encoder.idl modified Mon Sep  7 10:17:53 2015. */

#ifndef PPAPI_C_PPB_AUDIO_ENCODER_H_
#define PPAPI_C_PPB_AUDIO_ENCODER_H_

#include "ppapi/c/pp_array_output.h"
#include "ppapi/c/pp_bool.h"
#include "ppapi/c/pp_codecs.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/pp_instance.h"
#include "ppapi/c/pp_macros.h"
#include "ppapi/c/pp_resource.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/ppb_audio_buffer.h"

#define PPB_AUDIOENCODER_INTERFACE_0_1 "PPB_AudioEncoder;0.1" /* dev */
/**
 * @file
 * This file defines the <code>PPB_AudioEncoder</code> interface.
 */


/**
 * @addtogroup Interfaces
 * @{
 */
/**
 * Audio encoder interface.
 *
 * Typical usage:
 * - Call Create() to create a new audio encoder resource.
 * - Call GetSupportedProfiles() to determine which codecs and profiles are
 *   available.
 * - Call Initialize() to initialize the encoder for a supported profile.
 * - Call GetBuffer() to get an empty buffer and fill it in, or get an audio
 *   buffer from another resource, e.g. <code>PPB_MediaStreamAudioTrack</code>.
 * - Call Encode() to push the audio buffer to the encoder. If an external
 *   buffer is pushed, wait for completion to recycle the buffer.
 * - Call GetBitstreamBuffer() continuously (waiting for each previous call to
 *   complete) to pull encoded buffers from the encoder.
 * - Call RecycleBitstreamBuffer() after consuming the data in the bitstream
 *   buffer.
 * - To destroy the encoder, the plugin should release all of its references to
 *   it. Any pending callbacks will abort before the encoder is destroyed.
 *
 * Available audio codecs vary by platform.
 * All: opus.
 */
struct PPB_AudioEncoder_0_1 { /* dev */
  /**
   * Creates a new audio encoder resource.
   *
   * @param[in] instance A <code>PP_Instance</code> identifying the instance
   * with the audio encoder.
   *
   * @return A <code>PP_Resource</code> corresponding to an audio encoder if
   * successful or 0 otherwise.
   */
  PP_Resource (*Create)(PP_Instance instance);
  /**
   * Determines if the given resource is an audio encoder.
   *
   * @param[in] resource A <code>PP_Resource</code> identifying a resource.
   *
   * @return <code>PP_TRUE</code> if the resource is a
   * <code>PPB_AudioEncoder</code>, <code>PP_FALSE</code> if the resource is
   * invalid or some other type.
   */
  PP_Bool (*IsAudioEncoder)(PP_Resource resource);
  /**
   * Gets an array of supported audio encoder profiles.
   * These can be used to choose a profile before calling Initialize().
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[in] output A <code>PP_ArrayOutput</code> to receive the supported
   * <code>PP_AudioProfileDescription</code> structs.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return If >= 0, the number of supported profiles returned, otherwise an
   * error code from <code>pp_errors.h</code>.
   */
  int32_t (*GetSupportedProfiles)(PP_Resource audio_encoder,
                                  struct PP_ArrayOutput output,
                                  struct PP_CompletionCallback callback);
  /**
   * Initializes an audio encoder resource. The plugin should call Initialize()
   * successfully before calling any of the functions below.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[in] channels The number of audio channels to encode.
   * @param[in] input_sampling_rate The sampling rate of the input audio buffer.
   * @param[in] input_sample_size The sample size of the input audio buffer.
   * @param[in] output_profile A <code>PP_AudioProfile</code> specifying the
   * codec profile of the encoded output stream.
   * @param[in] initial_bitrate The initial bitrate for the encoder.
   * @param[in] acceleration A <code>PP_HardwareAcceleration</code> specifying
   * whether to use a hardware accelerated or a software implementation.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns PP_ERROR_NOTSUPPORTED if audio encoding is not available, or the
   * requested codec profile is not supported.
   */
  int32_t (*Initialize)(PP_Resource audio_encoder,
                        uint32_t channels,
                        PP_AudioBuffer_SampleRate input_sample_rate,
                        PP_AudioBuffer_SampleSize input_sample_size,
                        PP_AudioProfile output_profile,
                        uint32_t initial_bitrate,
                        PP_HardwareAcceleration acceleration,
                        struct PP_CompletionCallback callback);
  /**
   * Gets the number of audio samples per channel that audio buffers must
   * contain in order to be processed by the encoder. This will be the number of
   * samples per channels contained in buffers returned by GetBuffer().
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @return An int32_t containing the number of samples required, or an error
   * code from <code>pp_errors.h</code>.
   * Returns PP_ERROR_FAILED if Initialize() has not successfully completed.
   */
  int32_t (*GetNumberOfSamples)(PP_Resource audio_encoder);
  /**
   * Gets a blank audio buffer (with metadata given by the Initialize()
   * call) which can be filled with audio data and passed to the encoder.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[out] audio_buffer A blank <code>PPB_AudioBuffer</code> resource.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns PP_ERROR_FAILED if Initialize() has not successfully completed.
   */
  int32_t (*GetBuffer)(PP_Resource audio_encoder,
                       PP_Resource* audio_buffer,
                       struct PP_CompletionCallback callback);
  /**
   * Encodes an audio buffer.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[in] audio_buffer The <code>PPB_AudioBuffer</code> to be encoded.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion. Plugins that pass <code>PPB_AudioBuffer</code> resources owned
   * by other resources should wait for completion before reusing them.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns PP_ERROR_FAILED if Initialize() has not successfully completed.
   */
  int32_t (*Encode)(PP_Resource audio_encoder,
                    PP_Resource audio_buffer,
                    struct PP_CompletionCallback callback);
  /**
   * Gets the next encoded bitstream buffer from the encoder.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[out] bitstream_buffer A <code>PP_BitstreamBuffer</code> containing
   * encoded audio data.
   * @param[in] callback A <code>PP_CompletionCallback</code> to be called upon
   * completion. The plugin can call GetBitstreamBuffer from the callback in
   * order to continuously "pull" bitstream buffers from the encoder.
   *
   * @return An int32_t containing an error code from <code>pp_errors.h</code>.
   * Returns PP_ERROR_FAILED if Initialize() has not successfully completed.
   * Returns PP_ERROR_INPROGRESS if a prior call to GetBitstreamBuffer() has
   * not completed.
   */
  int32_t (*GetBitstreamBuffer)(
      PP_Resource audio_encoder,
      struct PP_AudioBitstreamBuffer* bitstream_buffer,
      struct PP_CompletionCallback callback);
  /**
   * Recycles a bitstream buffer back to the encoder.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[in] bitstream_buffer A <code>PP_BitstreamBuffer</code> that is no
   * longer needed by the plugin.
   */
  void (*RecycleBitstreamBuffer)(
      PP_Resource audio_encoder,
      const struct PP_AudioBitstreamBuffer* bitstream_buffer);
  /**
   * Requests a change to the encoding bitrate. This is only a request,
   * fulfilled on a best-effort basis.
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   * @param[in] bitrate The requested new bitrate, in bits per second.
   */
  void (*RequestBitrateChange)(PP_Resource audio_encoder, uint32_t bitrate);
  /**
   * Closes the audio encoder, and cancels any pending encodes. Any pending
   * callbacks will still run, reporting <code>PP_ERROR_ABORTED</code> . It is
   * not valid to call any encoder functions after a call to this method.
   * <strong>Note:</strong> Destroying the audio encoder closes it implicitly,
   * so you are not required to call Close().
   *
   * @param[in] audio_encoder A <code>PP_Resource</code> identifying the audio
   * encoder.
   */
  void (*Close)(PP_Resource audio_encoder);
};
/**
 * @}
 */

#endif  /* PPAPI_C_PPB_AUDIO_ENCODER_H_ */


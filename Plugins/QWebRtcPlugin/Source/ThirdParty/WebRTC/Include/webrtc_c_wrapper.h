#ifndef WEBRTC_C_WRAPPER_H_
#define WEBRTC_C_WRAPPER_H_

#include <stdint.h>

#if defined(_MSC_VER)
    #if defined(WEBRTC_WRAPPER_EXPORT)
        #define WEBRTC_WRAPPER_API __declspec(dllexport)
    #else
        #define WEBRTC_WRAPPER_API __declspec(dllimport)
    #endif
#else
    #define WEBRTC_WRAPPER_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Callback typings
typedef void (*WebRTC_OnDataChannelMessageCallback)(void* user_data, const char* text);
typedef void (*WebRTC_OnDataChannelStateChangeCallback)(void* user_data, int state);

// Initialize / Cleanup WebRTC SSL
WEBRTC_WRAPPER_API int WebRTC_InitializeSSL();
WEBRTC_WRAPPER_API void WebRTC_CleanupSSL();

// WebRTC Thread management
WEBRTC_WRAPPER_API void* WebRTC_CreateThread(const char* name);
WEBRTC_WRAPPER_API void WebRTC_DestroyThread(void* thread_ptr);

// PeerConnectionFactory management
WEBRTC_WRAPPER_API void* WebRTC_CreatePeerConnectionFactory(
    void* network_thread,
    void* worker_thread,
    void* signaling_thread
);
WEBRTC_WRAPPER_API void WebRTC_ReleasePeerConnectionFactory(void* factory_ptr);

// PeerConnection management
WEBRTC_WRAPPER_API void* WebRTC_CreatePeerConnection(void* factory_ptr);
WEBRTC_WRAPPER_API void WebRTC_ReleasePeerConnection(void* pc_ctx_ptr);

// DataChannel management
WEBRTC_WRAPPER_API void* WebRTC_CreateDataChannel(void* pc_ctx_ptr, const char* label);
WEBRTC_WRAPPER_API void WebRTC_ReleaseDataChannel(void* channel_ctx_ptr);
WEBRTC_WRAPPER_API int WebRTC_SendDataChannelText(void* channel_ctx_ptr, const char* text);
WEBRTC_WRAPPER_API int WebRTC_GetDataChannelState(void* channel_ctx_ptr);
WEBRTC_WRAPPER_API void WebRTC_RegisterDataChannelCallbacks(
    void* channel_ctx_ptr,
    void* user_data,
    WebRTC_OnDataChannelMessageCallback on_message,
    WebRTC_OnDataChannelStateChangeCallback on_state_change
);

// Video Frame Ingestion
WEBRTC_WRAPPER_API void* WebRTC_CreateVideoSource(void* factory_ptr);
WEBRTC_WRAPPER_API void* WebRTC_CreateVideoTrack(void* factory_ptr, void* source_ptr, const char* label);
WEBRTC_WRAPPER_API void WebRTC_PushVideoFrameYUV(
    void* source_ptr,
    const uint8_t* y_data,
    const uint8_t* u_data,
    const uint8_t* v_data,
    int width,
    int height,
    int stride_y,
    int stride_u,
    int stride_v,
    int64_t timestamp_us
);

// Audio Frame Ingestion
WEBRTC_WRAPPER_API void* WebRTC_CreateAudioSource(void* factory_ptr);
WEBRTC_WRAPPER_API void* WebRTC_CreateAudioTrack(void* factory_ptr, void* source_ptr, const char* label);
WEBRTC_WRAPPER_API void WebRTC_PushAudioFrame(
    void* source_ptr,
    const int16_t* audio_data,
    int sample_rate,
    int num_channels,
    int num_samples
);

// Add tracks to PeerConnection
WEBRTC_WRAPPER_API void* WebRTC_AddTrack(void* pc_ctx_ptr, void* track_ptr);
WEBRTC_WRAPPER_API void WebRTC_RemoveTrack(void* pc_ctx_ptr, void* sender_ptr);
WEBRTC_WRAPPER_API void WebRTC_ReleaseRefCounted(void* ptr);

// SDP and ICE candidate negotiation
typedef void (*WebRTC_OnLocalSdpCreatedCallback)(void* user_data, const char* type, const char* sdp);
typedef void (*WebRTC_OnIceCandidateGeneratedCallback)(void* user_data, const char* sdp_mid, int sdp_mline_index, const char* candidate);

WEBRTC_WRAPPER_API void WebRTC_RegisterSignalingCallbacks(
    void* pc_ctx_ptr,
    void* user_data,
    WebRTC_OnLocalSdpCreatedCallback on_local_sdp,
    WebRTC_OnIceCandidateGeneratedCallback on_ice_candidate
);

WEBRTC_WRAPPER_API void WebRTC_SetRemoteDescription(void* pc_ctx_ptr, const char* type, const char* sdp);
WEBRTC_WRAPPER_API void WebRTC_AddIceCandidate(void* pc_ctx_ptr, const char* sdp_mid, int sdp_mline_index, const char* candidate);
WEBRTC_WRAPPER_API void WebRTC_CreateOffer(void* pc_ctx_ptr);
WEBRTC_WRAPPER_API void WebRTC_CreateAnswer(void* pc_ctx_ptr);

#ifdef __cplusplus
}
#endif

#endif // WEBRTC_C_WRAPPER_H_

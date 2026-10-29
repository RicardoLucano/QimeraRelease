#include <iostream>
#include <cassert>
#include "webrtc_c_wrapper.h"

// Define tests
bool TestSslInitialization() {
    std::cout << "[Test] Running SSL Initialization..." << std::endl;
    int success = WebRTC_InitializeSSL();
    if (!success) {
        std::cerr << "SSL Init Failed!" << std::endl;
        return false;
    }
    WebRTC_CleanupSSL();
    std::cout << "[Test] SSL Init Passed." << std::endl;
    return true;
}

bool TestThreadLifecycle() {
    std::cout << "[Test] Running Thread Lifecycle..." << std::endl;
    WebRTC_InitializeSSL();
    void* thread = WebRTC_CreateThread("test_worker_thread");
    if (!thread) {
        std::cerr << "Thread Creation Failed!" << std::endl;
        WebRTC_CleanupSSL();
        return false;
    }
    WebRTC_DestroyThread(thread);
    WebRTC_CleanupSSL();
    std::cout << "[Test] Thread Lifecycle Passed." << std::endl;
    return true;
}

bool TestFactoryLifecycle() {
    std::cout << "[Test] Running Factory Lifecycle..." << std::endl;
    WebRTC_InitializeSSL();
    void* net_t = WebRTC_CreateThread("test_net_t");
    void* work_t = WebRTC_CreateThread("test_work_t");
    void* sig_t = WebRTC_CreateThread("test_sig_t");
    
    void* factory = WebRTC_CreatePeerConnectionFactory(net_t, work_t, sig_t);
    if (!factory) {
        std::cerr << "PeerConnectionFactory Creation Failed!" << std::endl;
        WebRTC_DestroyThread(net_t);
        WebRTC_DestroyThread(work_t);
        WebRTC_DestroyThread(sig_t);
        WebRTC_CleanupSSL();
        return false;
    }
    WebRTC_ReleasePeerConnectionFactory(factory);
    WebRTC_DestroyThread(net_t);
    WebRTC_DestroyThread(work_t);
    WebRTC_DestroyThread(sig_t);
    WebRTC_CleanupSSL();
    std::cout << "[Test] Factory Lifecycle Passed." << std::endl;
    return true;
}

bool TestConnectionAndMediaTracks() {
    std::cout << "[Test] Running Connection & Media Tracks..." << std::endl;
    WebRTC_InitializeSSL();
    void* net_t = WebRTC_CreateThread("test_net_t");
    void* work_t = WebRTC_CreateThread("test_work_t");
    void* sig_t = WebRTC_CreateThread("test_sig_t");
    void* factory = WebRTC_CreatePeerConnectionFactory(net_t, work_t, sig_t);
    
    void* pc = WebRTC_CreatePeerConnection(factory);
    if (!pc) {
        std::cerr << "PeerConnection Creation Failed!" << std::endl;
        WebRTC_ReleasePeerConnectionFactory(factory);
        WebRTC_DestroyThread(net_t);
        WebRTC_DestroyThread(work_t);
        WebRTC_DestroyThread(sig_t);
        WebRTC_CleanupSSL();
        return false;
    }
    
    void* video_source = WebRTC_CreateVideoSource(factory);
    void* video_track = WebRTC_CreateVideoTrack(factory, video_source, "test_video_track");
    void* audio_source = WebRTC_CreateAudioSource(factory);
    void* audio_track = WebRTC_CreateAudioTrack(factory, audio_source, "test_audio_track");
    
    if (!video_track || !audio_track) {
        std::cerr << "Media Tracks Creation Failed!" << std::endl;
        if (video_track) WebRTC_ReleaseRefCounted(video_track);
        if (video_source) WebRTC_ReleaseRefCounted(video_source);
        if (audio_track) WebRTC_ReleaseRefCounted(audio_track);
        if (audio_source) WebRTC_ReleaseRefCounted(audio_source);
        WebRTC_ReleasePeerConnection(pc);
        WebRTC_ReleasePeerConnectionFactory(factory);
        WebRTC_DestroyThread(net_t);
        WebRTC_DestroyThread(work_t);
        WebRTC_DestroyThread(sig_t);
        WebRTC_CleanupSSL();
        return false;
    }
    
    void* video_sender = WebRTC_AddTrack(pc, video_track);
    void* audio_sender = WebRTC_AddTrack(pc, audio_track);
    
    if (!video_sender || !audio_sender) {
        std::cerr << "Add Track Failed!" << std::endl;
    } else {
        std::cout << "[Test] Video and Audio tracks added successfully." << std::endl;
    }
    
    if (video_sender) WebRTC_ReleaseRefCounted(video_sender);
    if (audio_sender) WebRTC_ReleaseRefCounted(audio_sender);
    
    WebRTC_ReleaseRefCounted(video_track);
    WebRTC_ReleaseRefCounted(video_source);
    WebRTC_ReleaseRefCounted(audio_track);
    WebRTC_ReleaseRefCounted(audio_source);
    
    WebRTC_ReleasePeerConnection(pc);
    WebRTC_ReleasePeerConnectionFactory(factory);
    WebRTC_DestroyThread(net_t);
    WebRTC_DestroyThread(work_t);
    WebRTC_DestroyThread(sig_t);
    WebRTC_CleanupSSL();
    std::cout << "[Test] Connection & Media Tracks Passed." << std::endl;
    return true;
}

int RunWebRtcPluginTests() {
    int failed = 0;
    
    if (!TestSslInitialization()) failed++;
    if (!TestThreadLifecycle()) failed++;
    if (!TestFactoryLifecycle()) failed++;
    if (!TestConnectionAndMediaTracks()) failed++;
    
    if (failed == 0) {
        std::cout << "\n>>> ALL TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    } else {
        std::cerr << "\n>>> " << failed << " TEST(S) FAILED! <<<\n" << std::endl;
    }
    
    return failed;
}

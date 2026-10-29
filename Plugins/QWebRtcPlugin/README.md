# QWebRtcPlugin for Unreal Engine 5.6

`QWebRtcPlugin` is a high-performance Unreal Engine plugin that integrates native WebRTC capabilities into Unreal Engine. It exposes connection, data-channel messaging, and real-time audio/video ingestion (streaming) directly to Blueprints.

The plugin utilizes a lightweight C-wrapper (`webrtc_c_wrapper.dll`) around the native WebRTC library, and connects to a WebSocket-based signaling server to coordinate connections.

---

## Features

* **Real-time Video Streaming**: Ingest YUV420p video frames into the WebRTC pipeline via `PushVideoFrameYUV`.
* **Real-time Audio Streaming**: Ingest 16-bit PCM audio samples via `PushAudioFrame`.
* **Bi-directional Data Channels**: Send and receive text messages over WebRTC Data Channels.
* **WebSocket Signaling**: Built-in signaling client logic that handshakes automatically using standard SDP Offer/Answer and ICE candidates.
* **Blueprint Friendly**: Fully exposed properties, events, and functions.
* **YUV Texture Helper**: Static library functions to extract and convert Unreal `UTexture` (Render Targets or 2D textures) into YUV420p frames on the fly.

---

## Architecture Overview

The plugin's core class is `AQWebRtcActor`. Here is how the signaling and negotiation flow works when connecting to a remote client (e.g., a React frontend):

```
+------------------+             +------------------+             +-------------------+
|  QWebRtcActor    |             | Signaling Server |             |   WebRTC Client   |
| (Unreal Engine)  |             |   (WebSockets)   |             | (React / Browser) |
+--------+---------+             +--------+---------+             +---------+---------+
         |                                |                                 |
         |  Connect()                     |                                 |
         |------------------------------->|                                 |
         |  SignalingConnected Event      |                                 |
         |<-------------------------------|                                 |
         |                                |                                 |
         |                                |          Client Joins / Offers  |
         |                                |<--------------------------------|
         |  SDP Offer                     |                                 |
         |<-------------------------------|                                 |
         |                                |                                 |
         |  SetRemoteDescription()        |                                 |
         |  CreateAnswer()                |                                 |
         |                                |                                 |
         |  SDP Answer                    |                                 |
         |------------------------------->|                                 |
         |                                |  SDP Answer                     |
         |                                |-------------------------------->|
         |                                |                                 |
         |  Local ICE Candidate           |                                 |
         |------------------------------->|                                 |
         |                                |  Local ICE Candidate            |
         |                                |-------------------------------->|
         |                                |                                 |
         |                                |  Remote ICE Candidate           |
         |                                |<--------------------------------|
         |  Remote ICE Candidate          |                                 |
         |<-------------------------------|                                 |
         |                                |                                 |
         +==================================================================+
         |                 Direct Peer-to-Peer WebRTC Session               |
         |             (Audio / Video / Data Channel Established)            |
         +==================================================================+
```

---

## Core Components

### 1. `AQWebRtcActor`
Place or spawn this Actor in your level to manage the WebRTC session lifecycle.

#### Properties
* **`SignalingServerUrl`** (FString, BlueprintReadWrite): The URL of your WebSocket signaling server (e.g., `ws://localhost:3000`).
* **`ChannelLabel`** (FString, BlueprintReadWrite): The label of the RTC Data Channel (default: `qimera-chat`).
* **`bAutoConnect`** (bool, BlueprintReadWrite): If true, starts the connection sequence automatically on `BeginPlay`.

#### Functions
* **`Connect()`** -> `bool`: Initializes WebRTC and opens the WebSocket connection.
* **`Disconnect()`**: Closes active WebRTC peer connections, data channels, and the WebSocket signaling socket.
* **`SendTextMessage(Message)`** -> `bool`: Sends a text string through the WebRTC data channel.
* **`PushVideoFrameYUV(YData, UData, VData, Width, Height)`**: Pushes a raw YUV420p video frame to the WebRTC video track.
* **`PushAudioFrame(PCMData, SampleRate, NumChannels, NumSamples)`**: Pushes PCM16 audio samples to the WebRTC audio track.

#### Events (Delegates)
* **`OnInitialized(bSuccess)`**: Fired when the WebRTC local subsystem is initialized.
* **`OnConnectionStateChanged(NewState)`**: Fired on state changes (`SignalingConnected`, `Open`, `Closed`, `ConnectionError`, etc.).
* **`OnMessageReceived(Message)`**: Fired when a text message is received via the WebRTC data channel.

---

### 2. `UQWebRtcBlueprintFunctionLibrary`
A utility helper library that handles texture pixel readback and converts it to YUV420p format.

#### Functions
* **`ExtractYUVFromTexture(Texture, OutYData, OutUData, OutVData, OutWidth, OutHeight)`** -> `bool`:
  * Takes a `UTexture*` (works with `UTextureRenderTarget2D` or CPU-readable `UTexture2D`).
  * Extracts the raw pixels, performs chroma subsampling, and returns the three planar arrays `YData`, `UData`, `VData` along with the dimensions.
  * Dimensions must be **positive even integers** (e.g. 1280x720, 1920x1080) for YUV420p subsampling.

---

## Blueprint Integration Guide

Follow these steps to capture your Unreal Engine viewport or camera and stream it over WebRTC:

### Step 1: Add QWebRtcActor to the Level
1. Drag the **QWebRtcActor** from the Place Actors panel or your Content Browser into your level.
2. In the Details panel:
   * Set **Signaling Server Url** to your server (e.g. `ws://localhost:3000`).
   * Set **Channel Label** to match your frontend channel name.
   * Enable **Auto Connect** (or call `Connect` manually in your Blueprint).

### Step 2: Set Up Scene Capture 2D (Video Source)
1. Add a **Scene Capture 2D** actor to the level. Place it where you want the stream camera to be.
2. Create a new **Render Target** in your Content Browser:
   * Right-click -> **Materials & Textures** -> **Render Target**. Name it `RT_WebRTCStream`.
   * Open `RT_WebRTCStream`, set the size to even numbers (e.g. **Width: 1280, Height: 720**).
   * Ensure the Address X and Address Y are set to Clamp.
3. Select your **Scene Capture 2D** actor:
   * In the Details panel, under **Scene Capture**, set the **Texture Target** to `RT_WebRTCStream`.
   * Set **Capture Source** to `Final Color (LDR) in RGB` (this matches standard FColor format).

### Step 3: Capture and Stream Frames (Tick Loop)
1. Open your Level Blueprint (or create a custom Actor Blueprint).
2. Get a reference to your **QWebRtcActor** and your `RT_WebRTCStream` render target.
3. Create a **Tick** event (or set up a recurring timer, e.g. 30 FPS).
4. Implement the following node chain:
   * Call `Extract YUV From Texture` passing your `RT_WebRTCStream` as the `Texture` input.
   * Pull the return value into a **Branch**.
   * If **True**, call `Push Video Frame YUV` on your **QWebRtcActor** reference, passing:
     * `YData` -> `Out YData`
     * `UData` -> `Out UData`
     * `VData` -> `Out VData`
     * `Width` -> `Out Width`
     * `Height` -> `Out Height`

```
[ Event Tick ]
      |
      v
[ Extract YUV From Texture ] (Texture: RT_WebRTCStream)
      |
      +---> (Return Value: True) ---> [ Push Video Frame YUV ] (Target: QWebRtcActor)
```

---

## Optimization & Performance Guidelines

> [!WARNING]
> Reading back pixels from the GPU to the CPU (via `ReadPixels`) is a blocking operation on the Render Thread. Doing this at high resolutions (e.g., 1080p or 4K) or high framerates (60 FPS) on the game thread will introduce significant frame-rate drops.

### Tips for Optimal Performance:
1. **Limit Resolution**: Keep the Render Target resolution to `1280x720` or `960x540`. This offers a good balance between video quality and readback speed.
2. **Limit Frame Rate**: Do not capture on every `Tick` if the game is running at 60+ FPS. Instead, use a Blueprint **Timer** to trigger the capture and stream at a locked rate (e.g., **24 FPS** or **30 FPS**).
3. **Ensure Even Dimensions**: YUV 4:2:0 subsampling splits chrominance blocks into 2x2 pixels. If your render target size is odd (e.g. 1001x701), the subsampling helper will return false.

---

## Troubleshooting

### WebRTC Subsystem Fails to Initialize
* Make sure `webrtc_c_wrapper.dll` is present in the `Plugins/QWebRtcPlugin/Source/ThirdParty/WebRTC/Lib/win64/` directory.
* Verify your firewall is not blocking signaling port connections.

### Blueprint Nodes are Missing
* If you just created the helper code, you must compile the project.
* In the Unreal Editor, press **Ctrl+Alt+F11** to trigger a **Live Coding** compile. The new helper nodes (`Extract YUV From Texture`) will instantly appear in the Blueprint graph once compilation succeeds.

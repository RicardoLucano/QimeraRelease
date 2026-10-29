# AGENTS.md — QimeraCompose5 AI Coding Guide

## Project Overview
Unreal Engine **5.6** Virtual Production project (`QimeraCompose5.uproject`). All production logic lives in **`Plugins/QimeraCompose/`**, not in `Source/QimeraCompose5/` (that module is a near-empty shell). The editor extension lives in `QimeraComposeEditor` (PostEngineInit phase).

## Build
- **Solution**: `QimeraCompose5.sln` (VS2022)
- **Engine version**: UE 5.6 (`"EngineAssociation": "5.6"`)
- **CLI build**: `UnrealBuildTool.exe QimeraCompose5Editor Win64 Development`
- All modules use `PCHUsageMode.UseExplicitOrSharedPCHs`
- `INTERNAL_EXPERIMENTAL=1` is defined globally in `QimeraCompose.Build.cs`
- Firebase SDK static `.lib` files are Windows-only (VS2019 MD x64 Release), linked from `Plugins/QimeraCompose/Source/ThirdParty/firebase_cpp_sdk/libs/`. `libcrypto.lib` / `libssl.lib` are **Editor-only** due to size constraints.

## Architecture: The Subsystem Graph
All domain logic is split into UE Subsystems. `UQimeraSubsystem` (EngineSubsystem) is the root orchestrator.

| Subsystem | Base | Responsibility |
|---|---|---|
| `UQimeraSubsystem` | EngineSubsystem | Root: composition mode, view mode, PIE lifecycle, load orchestration |
| `USceneSubsystem` | GameInstanceSubsystem | Named scene CRUD + active/preview scene switching |
| `UMainCanvasSubsystem` | GameInstanceSubsystem | UMG canvas overlay creation and child widget attachment |
| `UQimeraComposureData` | GameInstanceSubsystem | Composure compositing element references |
| `UQimeraTrackingSubsystem` | EngineSubsystem | Camera tracking, LiveLink, talent tracker, lens/distortion |
| `UQimeraMediaInputSubsystem` | EngineSubsystem | SDI/media source management (`UQimeraMediaInputDataObject` wrappers) |
| `UQimeraOutputSubsystem` | EngineSubsystem | `UMediaCapture` lifecycle with auto-restart on stop/error |
| `UQimeraDMXSubsystem` | EngineSubsystem | DMX light data with named presets |
| `UQimeraOSCSubsystem` | GameInstanceSubsystem | OSC server/client + StreamDeck Companion protocol |
| `UQimeraTriggerSubsystem` | EngineSubsystem | Trigger objects → actor actions via registry ID |
| `UQimeraSyncSubsystem` | EngineSubsystem | Frame sync / genlock via Timed Data Monitor |
| `UQimeraLivelinkSubsystem` | EngineSubsystem | LiveLink preset apply/save/load |
| `UQimeraActorRegistrySubsystem` | GameInstanceSubsystem | Global `FName → AActor*` registry |
| `UQimeraSaveLoadSubsystem` | EngineSubsystem | In-memory `TMap<FString, USaveGame*>` cache + optional disc load |
| `UQimeraCommandExecutor` | EngineSubsystem | Console variable asset presets |
| `UQimeraLauncherDataController` | EngineSubsystem | IPC bridge to `QimeraDataSync` (launcher ↔ runtime) |
| `UQimeraAugmentedRealitySubsystem` | GameInstanceSubsystem + FTickableGameObject | AR viewport overlay |
| `UQimeraCameraSubsystem` | EngineSubsystem + IQimeraLoader | Tracked/trackless camera management, active camera switching (`Camera/QimeraCameraSubsystem.h`) |
| `UQimeraMediaPlayerSubsystem` | GameInstanceSubsystem | `UMediaPlayer` registration & ownership tracking — prevents double-use (`Media/QimeraMediaPlayerSubsystem.h`) |
| `UQimeraTimedDataSubsystem` | GameInstanceSubsystem | Templated timed-data buffers (`float`, `int32`, `FVector`, `FTransform`) for frame-sync pipelines (`Sync/QimeraTimedDataSubsystem.h`) |
| `UViewTargetSetter` | GameInstanceSubsystem + FTickableGameObject | Binds active camera to player viewport & first ICVFX component on nDisplay root (`Camera/ViewTargetSetter.h`) |
| `UQimeraControllersSubsystem` | WorldSubsystem | Spawns runtime controller actors (trigger, composition, transition) on world init |

## The IQimeraLoader Boot Protocol
Every subsystem that must complete before the app is "ready" implements `IQimeraLoader` (`Plugins/QimeraCompose/Source/QimeraCompose/Public/QimeraLoader.h`).

**Required methods** (all `BlueprintNativeEvent`): `GetLoaderId()`, `IsRequired()`, `IsDone()`, `HasFailed()`, `BeginLoad()`, `BindLoaderEvents()`, `UnbindLoaderEvents()`

`UQimeraSubsystem::DiscoverAndBindLoaders()` auto-discovers all `IQimeraLoader` implementors, calls `BindLoaderEvents()`, then fires `BeginLoad()`. When all required loaders report done, `QimeraLoaded` broadcasts. When adding a new subsystem that blocks startup, implement this interface — do **not** chain logic off `PIEStarted`.

**Current loader implementors:**

| Subsystem | LoaderId |
|---|---|
| `UQimeraCameraSubsystem` | `"Camera.Init"` |
| `UQimeraTrackingSubsystem` | `"Trakcing.Init"` |
| `USceneSubsystem` | `"Scene.Init"` |
| `UMainCanvasSubsystem` | `"MainCanvas.Init"` |
| `UQimeraComposureData` | `"Camera.Init"` |

**Loader state pattern** — every implementor carries the same boilerplate fields:
```cpp
UPROPERTY(EditAnywhere, Category="Qimera|Loader") FName LoaderId = TEXT("MyLoader.Init");
UPROPERTY(EditAnywhere, Category="Qimera|Loader") bool bRequired = true;
UPROPERTY(VisibleAnywhere) bool bStarted, bDone, bFailed;
UPROPERTY(VisibleAnywhere) FString FailReason;
void MarkDone();          // sets bDone, broadcasts OnQimeraLoaderDone
void MarkFailed(const FString& Reason); // sets bFailed, broadcasts OnQimeraLoaderFailed
```

## Composition Modes
`ECompositionType` (set on `UQimeraSubsystem`): `VirtualProduction`, `AugmentedReality`, `Library`, `Node`.  
`EViewMode`: `UserInterface`, `QimeraEditor`.  
Save/load of the active type is handled via `UQimeraSubsystemSave`.

## Persistent Actor Pattern
`AQimeraPersistentActorBase` → `AQimeraShadowCatcher` / `AQimeraReflectionCatcher` / `AQimeraAmbientLightCaster`

- Identity key: `PersistentId` (FName set in editor)
- Each has a **sidecar** `USaveGame` (e.g. `UQimeraShadowCatcherSaveGame`) stored under that ID
- All use per-actor `UMaterialInstanceDynamic` (MID) with a parameter snapshot that survives material swaps
- Override `OnAfterApply(const FQimeraPersistentActorData&)` to react after load restores data
- `bUseMID = true` is the default — set false only for shared materials

## Actor Identity & Trigger System
1. Add `UQimeraIdentifiableComponent` to any actor needing a stable ID. Choose `IDGenerationMethod` (Manual / AutoSuffix / GUID).
2. On `BeginPlay` the component calls `UQimeraActorRegistrySubsystem::RegisterActorByID()`.
3. `UTriggerObject` references actors by their `FName` ID in `TriggerBaseData.ObjectNameID`, resolving lazily through the registry. Use `UTriggerObject::Initialize()` immediately after creation or deserialization.
4. `UTriggerObjectVisual` (`Triggers/TriggerObjectVisual.h`) — extends `UTriggerObject` with `FTriggerVisualData` and lazy `UQimeraFeedbackComponent` resolution. Call `InitializeVisual()` after `Initialize()`.
5. `AQimeraTriggerPlayer` (`Triggers/QimeraTriggerPlayer.h`) — actor that plays a named sequence of triggers with per-trigger delays. Carries `UQimeraFeedbackComponent` + `UQimeraIdentifiableComponent`. Call `StartPlayback(TriggerNames, TriggerDelays)`.
6. `ITransitionPlayerInterface` — Blueprintable interface with a single method `PlayTransitionWithName(const FString&)` used by `USceneSubsystem` during scene transitions.

## Feedback System
`UQimeraFeedbackComponent` (`QimeraFeedbackComponent.h`) is a `BlueprintSpawnableComponent` that tracks common playback/UI state:
- **Config flags**: `bSupportsActiveness`, `bSupportsPageIndex`, `bSupportsPlayerState`
- **State**: `bIsObjectActive`, `PageIndex`, `PlayerState` (`EQimeraFeedbackPlayerState`: Stopped/Playing/Paused), `RemainingPlayTime`
- **Events**: `OnActivenessChanged`, `OnPageIndexChanged`, `OnPlayerStateChanged`, `OnRemainingTimeFinished`

`IQimeraFeedbackProvider` (`QimeraFeedbackProvider.h`) — Blueprintable interface; implement `GetQimeraFeedbackComponent()` on any actor that owns a feedback component. `AQimeraTriggerPlayer` is the reference implementor.

## Tracking & Cameras
- `ITrackedCameraInterface` (`Tracking/TrackedCameraInterface.h`) — Blueprintable interface providing `GetTrackedCameraInfo()`, `ProcessTrackingData()`, `SetTransform()`, `SetFocus()`, `SetAperture()`, `SetFocalLength()`, `SetNodalOffset()`
- `AQimeraTrackedCamera` extends `ACineCameraActor` + `ITrackedCameraInterface` — always carries a `ULensComponent` and `ULensFile`
- `AQimeraFreeTrackedCamera` extends `ACineCameraActor` + `ITrackedCameraInterface` — camera without physical tracking; moves freely
- `AQimeraTracklessCamera` (`Camera/QimeraTracklessCamera.h`) — bare `ACineCameraActor` subclass (no tracking interface, ClassGroup=Qimera)
- `AQimeraDistortionController` — base: calls `UCameraCalibrationSubsystem` to render distortion/undistortion maps to render targets
  - `AQimeraVirtualProductionDistortionController` — VP specialization: updates distortion for CG composure elements
  - `AQimeraARDistortionController` — AR specialization: applies distortion directly to a given `ACineCameraActor`
- `UQimeraICVFXCameraComponent` (`NDisplay/QimeraICVFXCameraComponent.h`) — extends `UDisplayClusterICVFXCameraComponent` with `SetActiveCamera()` for runtime camera binding
- `AViewportRenderer` — actor that captures the game viewport to a render target with `FOpenColorIOColorConversionSettings` color transforms
- LiveLink preset path (hardcoded reference): `/QimeraCompose/QimeraTracking/LiveLinkPresets/FIZPreset.FIZPreset`

## Inter-Process Communication (QimeraDataSync)
`Plugins/QimeraDataSync/Source/QimeraDataSync/Public/`
- `UQimeraDataSyncSubsystem` (EngineSubsystem) supports two transports: `TcpSocketTransport` or `CopyFileTransport`
- Messages are `FQDS_Message` structs serialized to JSON with type tag (`UpdateData`, `DeleteData`, `Event`) and string payload
- `UQimeraLauncherDataController` bridges this to the QimeraSubsystem using `DataControllerTag = "LauncherDataController"`

## Content Asset Paths (hardcoded in C++)
| Purpose | Path |
|---|---|
| Launcher Settings UI | `/QimeraCompose/QimeraLauncher/QimeraLauncherSettingsUI.QimeraLauncherSettingsUI` |
| FIZ LiveLink Preset | `/QimeraCompose/QimeraTracking/LiveLinkPresets/FIZPreset.FIZPreset` |
| Library assets base | `Saved/SaveGames/AssetsBase/` (installed via `QimeraLibraryAssetsBP` plugin README) |
| Default CVar Preset | `/QimeraCompose/QimeraComposure/ConsoleVariablePreset/QimeraDefaultPreset.QimeraDefaultPreset` |

## Conventions
- **UFUNCTION/UPROPERTY categories**: always `"Qimera|<Domain>"` (e.g. `"Qimera|Loader"`, `"Qimera|ICVFX"`, `"Qimera|Registry"`)
- **Log categories**: declared with `DECLARE_LOG_CATEGORY_EXTERN` in headers, defined in `.cpp`; named after class (e.g. `LogQimeraMediaInputSubsystem`)
- **Save/Load**: use `UQimeraSaveLoadSubsystem::GetSaveGame()` / `SetSaveGame()` for transient in-memory storage; use `UGameplayStatics::SaveGameToSlot` for disc persistence
- **Editor-guarded code**: wrap editor-only API with `#if WITH_EDITOR` (see `QimeraCommandExecutor.h` for example)
- Delegates follow: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_<N>Params(FOn<Event>, ...)` — never raw C++ delegates for Blueprint-exposed events

## Key External Integrations
| Integration | Plugin / Module |
|---|---|
| SDI I/O (AJA, Blackmagic) | `AjaMedia`, `BlackmagicMedia` (Win64 only) |
| nDisplay / LED wall | `DisplayCluster`, `AQimeraDisplayClusterRootActor` extends `ADisplayClusterRootActor` |
| NVIDIA DLSS/NIS/Reflex | `DLSS`, `NIS`, `Streamline*` plugins |
| Color management | OpenColorIO + ACES configs in `Plugins/QimeraCompose/Resources/OpenColorIO-Configs-master/` |
| Firebase backend | C++ SDK, Windows-only static libs in `ThirdParty/firebase_cpp_sdk/` |
| StreamDeck | OSC via `UQimeraOSCSubsystem` (Companion protocol) |
| Depth/body tracking | `ZEDLiveLink` plugin, `TalentTracker` plugin (`ATalentTrackerBillboardSkeletonActor`) |
| PTZ camera control | `QimeraVisca` plugin (VISCA protocol, modules: `QimeraVisca` + `ViscaCore`) |
| Networking (UDP/WS/HTTP) | `ENCOAPI` plugin |
| Gaussian splats | `LumaAIPlugin` |
| Multi-User editing | `MultiUserClient`, `ConcertClient`, `ConcertSyncClient` modules + `Switchboard` plugin |


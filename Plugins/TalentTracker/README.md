# Talent Tracker (Plugin)

Runtime plugin that visualizes LiveLink animation subjects (ZEDLiveLink compatible) as an efficient billboard skeleton proxy.

## What’s implemented (v0.1)
- `ATalentTrackerBillboardSkeletonActor`: drop into a level, set `SubjectName` (e.g. `"0 ZED"`), and it will draw joint billboards using an `UInstancedStaticMeshComponent`.
- Pooling: instances are allocated once when static data is discovered; on each frame we only update instance transforms.
- Root-space transform: `RootSpaceTransform` can be set to offset/rotate incoming skeleton data before placing it in the world.

## Notes
- For true camera-facing billboards, assign an appropriate material that rotates the plane in the vertex shader to face the camera.
- The actor currently renders joints only. Bones (cylinders/strips between joints) can be added next.

```

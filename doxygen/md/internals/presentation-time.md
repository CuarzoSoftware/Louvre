## Presentation Time Implementation

1. The client creates one or more `RPresentationFeedback` resources for an `LSurface` (stored in `LSurfacePrivate::pending`).
2. On surface commit, the pending state is applied (`LSurfacePrivate::applyCommit`):
   1. If there are still presentation feedbacks from the previous commit, they are discarded (since that frame was never shown).
   2. Pending presentation feedbacks are moved to the current state.
3. When `LSurface::requestNextFrame(true)` is called inside `LOutput::paintGL()`, the surface is marked as presented on that output. The current `paintEventId` is saved, and feedbacks are moved to `LOutputPrivate::waitingPresentationFeedback`.
4. After `paintGL()`, `LOutputPrivate::handleUnpresentedSurfaces()` sends early discard events to surfaces on that output (LSurface::outputs()) that didn’t request a frame.
5. The backend always notifies a `CZPresentationEvent` (page flip) some time after each `paintGL()`, which may indicate either a presentation or a discard (`LOutputPrivate::backendPresented/Discarded`).
6. The `CZPresentationEvent` is queued in `LOutputPrivate::presentationEventQueue` for thread-safe delivery.
7. Later `LCompositor::dispatch()` calls `LCompositorPrivate::dispatchPresentationTimeEvents()`, which drains the queues of all outputs.
8. Each output processes its events in `LOutput::presentationEvent()`. Only if the `paintEventId` matches, the presented or discarded event is sent to the client.

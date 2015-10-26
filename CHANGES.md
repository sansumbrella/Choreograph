# Choreograph

A simple, modern C++ animation and timing library.  

## Unreleased (v0.4.0)
`Timeline` is now a `TimelineItem`: has a concept of where it is in time.
Removed `MotionGroup` since it is no longer needed.
Changed default `Time` type to be alias to double.
Added `splice()` method to Sequence.

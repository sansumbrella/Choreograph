# CobWeb

Simple 2D scene graph for UI development.
Dynamically allocated.
Organizes elements in space
Handles user interaction event hierarchy

Node
 - a point in space that can hold other elements
Buttons
 - easily-extended base
 - Image button
Toggles
 - swaps between two images
Text
 - body of text displayed using TextureFont
Scroll Pane (with knowledge of visible region passed on to children)

For Now:
Unoptimized, just calls draw() on each item in turn.
Challenge for batching is handling text rendering alongside basic triangles.
Would like to merge a TextureFont's texture into a larger spritesheet to enable that.
Would limit size/number of characters in a gui font (which is probably fine).

Goals:
UI elements add themselves to a spritesheet shared by the hierarchy.
- individual textures get added to spritesheet
- can also grab textures from existing sheet by name
Single draw call to render everything
(find way to add TextureFont to shared spritesheet)
(Look at TextureFont::getGlyphPlacements)

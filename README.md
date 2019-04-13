# Qt3D Experiments

Here are few projects that make use of Qt3D framework - I have ended up creating them while working on rendering techniques for QGIS 3D.

Qt3D framework is a really nice addition to Qt ecosystem, but the documentation is still quite sparse and sometimes even wrong. There are not that many code examples floating around that use Qt 3D to demonstrate various 3D rendering techniques, so this is my little contribution towards that.

I have used QML as much as possible to keep the code concise. C++ was only used for bits where QML does not have the necessary bindings (yet?).


# Billboards

Demonstrates billboards rendering technique - quads with constant screen size that are always facing the camera. This uses geometry shader to generate quads from points.

![](qt3d-billboards.png)

# Edge Detection

Edge detection is done as a post-processing pass of scene rendering. In the first stage we generate depth texture and normal vectors texture which are then combined in the post-processing pass using Sobel filter.

![](qt3d-edge-detection.png)

# Lines

Rendering of lines in 3D space with constant screen space thickness. Supports flat and miter joins.

![](qt3d-lines.png)

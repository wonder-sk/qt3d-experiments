import QtQuick 2.1 as QQ2
import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.10
import Qt3D.Extras 2.10

Entity {

    components: [
        InputSettings {},
        RenderSettings {
            activeFrameGraph: frameGraph
        }
    ]

    // framegraph:
    // 1. render objects that should get highlighted
    //    - using ordinary material, with writes to stencil buffers enabled
    //      (sets value 1 for pixels where we rendered something)
    // 2. render any other objects
    //    - using ordinary material (no stencil tests or writes)
    // 3. render the highlight (only rendering pixels around the objects from the first step)
    //    - use scaled-up geometry of highilghted object + single color material
    //    - no depth test + stencil tests enabled (stencil value must not be 1)

    RenderSurfaceSelector {
        id: frameGraph
        Viewport {
            normalizedRect: Qt.rect(0,0,1,1)
            CameraSelector {
                camera: camera
                LayerFilter {
                    layers: [layerNormalHighlight]
                    RenderStateSet {
                        renderStates: [
                            DepthTest { depthFunction: DepthTest.Less },
                            StencilMask { frontOutputMask: 0xff },
                            StencilOperation { front.allTestsPassOperation: StencilOperationArguments.Replace },
                            StencilTest {
                                front.stencilFunction: StencilTestArguments.Always
                                front.referenceValue: 1
                                front.comparisonMask: 0xff
                            }
                        ]
                        ClearBuffers {
                            buffers: ClearBuffers.ColorDepthStencilBuffer
                        }
                    }
                }
                LayerFilter {
                    layers: [layerNormal]
                    RenderStateSet {
                        renderStates: [
                            DepthTest { depthFunction: DepthTest.Less }
                        ]
                    }
                }
                LayerFilter {
                    layers: [layerHighlight]
                    RenderStateSet {
                        renderStates: [
                            StencilTest {
                                front.stencilFunction: StencilTestArguments.NotEqual
                                front.referenceValue: 1
                                front.comparisonMask: 0xff
                            }
                        ]
                    }
                }
            }
        }
    }

    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        aspectRatio: _window.width / _window.height
        nearPlane: 0.1
        farPlane: 100.0
        position: Qt.vector3d(-5.0, 5.0, 20.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
    }

    FirstPersonCameraController { camera: camera }

    Layer { id: layerNormal }             // stuff to render ordinarily
    Layer { id: layerNormalHighlight }    // stuff to render ordinarily, should be highlighted
    Layer { id: layerHighlight }          // higlight stuff (scaled up)

    Entity {
        PlaneMesh {
            id: pm
            width: 20
            height: 20
        }
        PhongMaterial {
            id: pmm
            ambient: Qt.rgba(0.0,0.0,0.7,1)
        }
        components: [ pm, pmm, layerNormal ]
    }

    Entity {
        PhongMaterial {
            id: redMat
            ambient: "red"
        }
        SphereMesh {
            id: sphereMesh
        }
        Transform {
            id: sphereTransform
            translation: Qt.vector3d(0,3,0)
        }

        components: [ redMat, sphereMesh, sphereTransform, layerNormalHighlight ]
    }

    // highlighted upscaled version of the sphere
    Entity {
        Transform {
            id: sphereTransformBig
            translation: sphereTransform.translation
            scale: 1.05
        }

        components: [ singleColorMaterial, sphereMesh, sphereTransformBig, layerHighlight ]
    }

    // material to draw everything with a single color - for the highlight
    Material {
        id: singleColorMaterial
        effect: Effect {
            techniques: Technique {
                graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
                renderPasses: [
                    RenderPass {
                        shaderProgram: ShaderProgram {
                            id: sp
                            vertexShaderCode: "
#version 420 core
in vec3 vertexPosition;
uniform mat4 modelViewProjection;
void main()  { gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 ); }"

                            fragmentShaderCode: "
#version 420 core
out vec4 fragColor;
void main() { fragColor = vec4(1.0,0.5,0.5,1.0); }"

                            onLogChanged: {
                                console.warn("status", sp.status)
                                console.log(sp.log)
                            }
                        }
                    }
                ]
            }
        }
    }

    Entity {
        PhongMaterial {
            id: grayMat
            ambient: "gray"
        }
        CuboidMesh {
            id: cubeMesh
        }
        Transform {
            id: cubeTransform
            translation: Qt.vector3d(1,2,4)
            scale: 2
        }

        components: [ grayMat, cubeMesh, cubeTransform, layerNormal ]
    }

}

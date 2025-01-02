import QtQuick 2.1 as QQ2
import Qt3D.Core 2.10
import Qt3D.Render 2.10
import Qt3D.Input 2.10
import Qt3D.Extras 2.10

Entity {

    property bool msaa: true

    components: [
        rendSettings,
        inputSettings
    ]

    InputSettings { id: inputSettings }

    KeyboardDevice { id: keyboardDevice }

    KeyboardHandler {
        focus: true
        sourceDevice: keyboardDevice
        onSpacePressed: {
            parent.msaa = !parent.msaa
            console.log("msaa:" + parent.msaa)
        }
    }

    RenderSettings {
        id: rendSettings
        activeFrameGraph: parent.msaa ? msaaFrameGraph : basicFrameGraph
    }

    // framegraph used when rendering without anti-aliasing - simple forward renderer
    RenderSurfaceSelector {
        id: basicFrameGraph
        Viewport {
            normalizedRect: Qt.rect(0,0,1,1)
            CameraSelector {
                camera: camera
                ClearBuffers {
                    buffers: ClearBuffers.ColorDepthBuffer
                }
            }
        }
    }

    // framegraph used when rendering with anti-aliasing
    // - first the usual forward renderer, but rendering to a render target
    //   with multisample textures, and with MSAA render state enabled
    // - then resolve the multisample framebuffer by doing blit
    RenderSurfaceSelector {
        id: msaaFrameGraph
        RenderTargetSelector {
            target: msaaFramebuffer
            RenderStateSet {
                renderStates: [
                    CullFace { mode: CullFace.Back },
                    DepthTest { depthFunction: DepthTest.Less },
                    MultiSampleAntiAliasing {}
                ]
                Viewport {
                    normalizedRect: Qt.rect(0,0,1,1)
                    CameraSelector {
                        camera: camera
                        ClearBuffers {
                            buffers: ClearBuffers.ColorDepthBuffer
                        }
                    }
                }
            }
        }
        BlitFramebuffer {
            source: msaaFramebuffer
            sourceRect: Qt.rect(0, 0, _window.width * _dpr, _window.height * _dpr)
            destinationRect: sourceRect
            NoDraw {}
        }
    }

    RenderTarget {
        id: msaaFramebuffer
        attachments: [
            RenderTargetOutput {
                attachmentPoint : RenderTargetOutput.Color0
                texture: Texture2DMultisample {
                    width : _window.width * _dpr
                    height : _window.height * _dpr
                    format : Texture.RGBA8_UNorm
                }
            },
            RenderTargetOutput {
                attachmentPoint : RenderTargetOutput.Depth
                texture: Texture2DMultisample {
                    width : _window.width * _dpr
                    height : _window.height * _dpr
                    format: Texture.DepthFormat
                }
            }
        ]
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
        components: [ pm, pmm ]
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

        components: [ redMat, sphereMesh, sphereTransform ]
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

        components: [ grayMat, cubeMesh, cubeTransform ]
    }

}

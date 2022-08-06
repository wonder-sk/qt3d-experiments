import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

Entity {

        components: [
            rendSettings,
            inputSettings
        ]

        InputSettings { id: inputSettings }

        RenderSettings {
            id: rendSettings
            activeFrameGraph: RenderSurfaceSelector {
                Viewport {
                    normalizedRect: Qt.rect(0,0,1,1)

                    // "ordinary" pass to render scene with shading to a color+depth texture
                    CameraSelector {
                        camera: camera
                        ClearBuffers {
                            buffers: ClearBuffers.ColorDepthBuffer
                            clearColor: Qt.rgba(0.3,0.3,0.3,1)
                            LayerFilter {
                                layers: [layerSsao, layerFinal]
                                filterMode: LayerFilter.DiscardAnyMatchingLayers
                                RenderTargetSelector {
                                    target: RenderTarget {
                                        attachments: [
                                            RenderTargetOutput { attachmentPoint : RenderTargetOutput.Color0; texture: colorTexture },
                                            RenderTargetOutput { attachmentPoint : RenderTargetOutput.Depth; texture: depthTexture }
                                        ]
                                    }
                                }
                            }
                        }
                    }

                    // SSAO pass - using depth to produce SSAO texture
                    CameraSelector {
                        camera: ortoCamera
                        RenderStateSet {
                            // disable depth tests (no need to clear buffers)
                            renderStates: [ DepthTest { depthFunction: DepthTest.Always } ]
                            LayerFilter {
                                layers: [layerSsao]
                                RenderTargetSelector {
                                    target: RenderTarget {
                                        attachments: [
                                            RenderTargetOutput { attachmentPoint : RenderTargetOutput.Color0; texture: ssaoTexture }
                                        ]
                                    }
                                }
                            }
                        }
                    }

                    // final pass - combine color texture and SSAO to the final result
                    CameraSelector {
                        camera: ortoCamera
                        RenderStateSet {
                            // disable depth tests (no need to clear buffers)
                            renderStates: [ DepthTest { depthFunction: DepthTest.Always } ]
                            LayerFilter {
                                layers: [layerFinal]
                            }
                        }
                    }

                }
            }

        }

        Texture2D {
            id : colorTexture
            width : _window.width
            height : _window.height
            format : Texture.RGB16F
            generateMipMaps : false
            magnificationFilter : Texture.Linear
            minificationFilter : Texture.Linear
            wrapMode {
                x: WrapMode.ClampToEdge
                y: WrapMode.ClampToEdge
            }
        }

        Texture2D {
            id : depthTexture
            width : _window.width
            height : _window.height

            format: Texture.DepthFormat

            generateMipMaps : false
            magnificationFilter : Texture.Linear
            minificationFilter : Texture.Linear
            wrapMode {
                x: WrapMode.ClampToEdge
                y: WrapMode.ClampToEdge
            }
        }

        Texture2D {
            id : ssaoTexture
            width : _window.width
            height : _window.height
            format : Texture.R16F
            generateMipMaps : false
            magnificationFilter : Texture.Linear
            minificationFilter : Texture.Linear
            wrapMode {
                x: WrapMode.ClampToEdge
                y: WrapMode.ClampToEdge
            }
        }

        Camera {
            id: ortoCamera
            projectionType: CameraLens.OrthographicProjection
            aspectRatio: _window.width / _window.height
            nearPlane: 1
            farPlane: 100.0
            position: Qt.vector3d(0.0, 10.0, 0.0)
            viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
            upVector: Qt.vector3d(0.0, 1.0, 0.0)
        }



    Camera {
        id: camera
        projectionType: CameraLens.PerspectiveProjection
        fieldOfView: 45
        aspectRatio: _window.width / _window.height
        nearPlane: 0.1
        farPlane: 1000.0
        position: Qt.vector3d(0.0, 10.0, 20.0)
        viewCenter: Qt.vector3d(0.0, 0.0, 0.0)
        upVector: Qt.vector3d(0.0, 1.0, 0.0)
    }

    FirstPersonCameraController { camera: camera }

    Layer { id: layerSsao }  // quad used for SSAO
    Layer { id: layerFinal }  // quad used for post-processing

    MyScene {
        id: sceneRoot
    }

    /////
    /////
    /////


    Entity {
        PlaneMesh {
            id: ssaoQuadMesh
            width: 1
            height: 1
        }
        Transform {
            id: ssaoQuadTransform
            translation: Qt.vector3d(0, 2, 0)
        }
        SsaoMaterial {
            id: ssaoMaterial

            textureColor: colorTexture
            textureDepth: depthTexture
            cameraZNear: camera.nearPlane
            cameraZFar: camera.farPlane
            cameraFov: camera.fieldOfView
            cameraAr: camera.aspectRatio
            cameraProjMatrix: camera.projectionMatrix
        }

        components: [ ssaoQuadMesh, ssaoMaterial, ssaoQuadTransform, layerSsao ]
    }

    /////

    Entity {
        PlaneMesh {
            id: finalQuadMesh
            width: 1
            height: 1
        }
        Transform {
            id: finalQuadTransform
            translation: Qt.vector3d(0, 2, 0)
        }
        FinalMaterial {
            id: finalMaterial

            textureColor: colorTexture
            textureSsao: ssaoTexture
        }

        components: [ finalQuadMesh, finalMaterial, finalQuadTransform, layerFinal ]
    }


}

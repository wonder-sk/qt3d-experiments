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
                //id: surfaceSelector
                Viewport {
                    normalizedRect: Qt.rect(0,0,1,1)
                    CameraSelector {
                        camera: camera

                        // "ordinary" pass to render scene with shading to a color+depth texture
                        ClearBuffers {
                            buffers: ClearBuffers.ColorDepthBuffer
                            clearColor: Qt.rgba(0.3,0.3,0.3,1)
                            LayerFilter {
                                layers: [layerQuad]
                                filterMode: LayerFilter.DiscardAnyMatchingLayers
                                RenderTargetSelector {
                                    target: rt
                                }
                            }
                        }

                        // second pass - renders a quad using input from color + depth buffer
                        CameraSelector {
                            camera: ortoCamera
                            LayerFilter {
                                layers: [layerQuad]
                                    ClearBuffers {
                                        buffers: ClearBuffers.ColorDepthBuffer
                                        //clearColor: Qt.rgba(0.8,0.0,0.0,1)
                                    }
                            }
                        }
                    }
                }
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

    PreprocessRenderTarget {
        id: rt

        w: _window.width
        h: _window.height
    }


    Layer { id: layerQuad }  // quad used for post-processing

    MyScene {
        id: sceneRoot
    }

    /////
    /////
    /////


    Entity {
        PlaneMesh {
            id: ortoMesh
            width: 1
            height: 1
        }
        Transform {
            id: ortoTr
            translation: Qt.vector3d(0, 2, 0)
        }
        // this material uses normal/depth textures and builds edges
        SsaoMaterial {
            id: ssaoMaterial

            textureColor: rt.textureColor
            textureDepth: rt.textureDepth
            cameraZNear: camera.nearPlane
            cameraZFar: camera.farPlane
            cameraFov: camera.fieldOfView
            cameraAr: camera.aspectRatio
            cameraProjMatrix: camera.projectionMatrix
        }

        components: [ ortoMesh, ssaoMaterial, ortoTr, layerQuad ]
    }


}

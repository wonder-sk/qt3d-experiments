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
                CameraSelector {
                    camera: camera
                    ClearBuffers {
                        buffers: ClearBuffers.ColorDepthBuffer
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
        position: Qt.vector3d(0.0, 20.0, 15.0)
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
        /*
        PhongMaterial {
            id: pmm
            ambient: Qt.rgba(0.0,0.0,0.7,1)
        }*/

        Material {
            id: pmm

            parameters: [
                Parameter { name: "tex0"; value: txt }
            ]

            Texture2D {
                id : txt
                generateMipMaps : false
                magnificationFilter : Texture.Linear
                minificationFilter : Texture.Linear
                textureImages: [
                    TextureImage {
                        source: "qrc:/arrow.png"
                    }
                ]
            }

            effect: Effect {
                techniques: Technique {
                    graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
                    renderPasses: [
                        RenderPass {
                            shaderProgram: ShaderProgram {
                                vertexShaderCode: loadSource("qrc:/shaders/arrows.vert")
                                fragmentShaderCode: loadSource("qrc:/shaders/arrows.frag")
                            }
                        }
                    ]
                }
            }
        }

        components: [ pm, pmm ]
    }

}

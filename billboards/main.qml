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
        position: Qt.vector3d(0.0, 10.0, 20.0)
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
        GeometryRenderer {
            id: gr
            primitiveType: GeometryRenderer.Points
            vertexCount: _bbg.count
            geometry: _bbg
        }

        Transform {
            id: trr
            translation: Qt.vector3d(0,1.5,0)
        }

        Material {
            id: grm

            parameters: [
                Parameter { name: "tex0"; value: txt },
                Parameter { name: "WIN_SCALE"; value: Qt.size(_window.width,_window.height) },
                Parameter { name: "BB_SIZE"; value: Qt.size(100, 100) }
            ]

            Texture2D {
                id : txt
                generateMipMaps : false
                magnificationFilter : Texture.Linear
                minificationFilter : Texture.Linear
                textureImages: [
                    TextureImage {
                        source: "qrc:/success-kid.png"
                    }
                ]
            }

            effect: Effect {
                techniques: Technique {
                    graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
                    renderPasses: [
                        RenderPass {
                            shaderProgram: ShaderProgram {
                                vertexShaderCode: loadSource("qrc:/shaders/billboards.vert")
                                geometryShaderCode: loadSource("qrc:/shaders/billboards.geom")
                                fragmentShaderCode: loadSource("qrc:/shaders/billboards.frag")
                            }
                        }
                    ]
                }
            }
        }

        components:  [ gr, grm, trr ]
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
            translation: Qt.vector3d(0,5,0)
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
            translation: Qt.vector3d(2,2,5)
            scale: 2
        }

        components: [ grayMat, cubeMesh, cubeTransform ]
    }

}

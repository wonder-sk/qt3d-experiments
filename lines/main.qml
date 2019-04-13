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

        // debugging of line clipping
        //position: Qt.vector3d(1.3532, 2.14093, 4.8184)
        //viewCenter: Qt.vector3d(23.6686, 0.746005, 4.53359)
        //upVector: Qt.vector3d(0.0623777, 0.998053, -0.000796974)

        function dumpCameraSetup() {
            console.log("position " + position)
            console.log("viewCenter" + viewCenter)
            console.log("upVector" + upVector)
        }

        function viewProjMatrix() {
            var viewMat = Qt.matrix4x4();
            viewMat.lookAt(camera.position, camera.viewCenter, camera.upVector)
            var viewProjMat = camera.projectionMatrix.times( viewMat )
            return viewProjMat;
        }

        function projectPoint(pt) {  // expecting vector3d
            return viewProjMatrix().times(Qt.vector4d(pt.x, pt.y, pt.z, 1))
        }

        function perspectiveDivide(pt) {  // expecting vector4d
            return Qt.vector3d(pt.x/pt.w, pt.y/pt.w, pt.z/pt.w)
        }

        function regCode(pt) {
            console.log( (pt.w+pt.x<0) + "-" + (pt.w-pt.x<0) + "-" + (pt.w+pt.y<0) + "-" + (pt.w-pt.y<0) + "-" + (pt.w+pt.z<0) + "-" + (pt.w-pt.z<0) )
        }

        function regCodeX(pt) {
            return (pt.w+pt.x<0) << 0 |
                   (pt.w-pt.x<0) << 1 |
                   (pt.w+pt.y<0) << 2 |
                   (pt.w-pt.y<0) << 3 |
                   (pt.w+pt.z<0) << 4 |
                    (pt.w-pt.z<0) << 5
        }

        function tstLineClipping() {

            //dumpCameraSetup()

            // when trying c++ version of our material
            //_lineMaterial.setCameraParameters(camera.position, camera.viewVector, camera.nearPlane)
            //_lineMaterial.setViewportSize(Qt.size(_window.width, _window.height))

            // this code does calculation of intersection between line and camera's near plane
            // ... used in geometry shader later to fix lines with points "behind" the camera
            var pt1in = Qt.vector3d(1,1,4)
            var pt2in = Qt.vector3d(3,1,4)
            //var pt1in = Qt.vector3d(-5,5,5)
            //var pt2in = Qt.vector3d(5,5,-5)
            var lineDir = pt2in.minus(pt1in)

            var pt1 = projectPoint(pt1in)
            var pt2 = projectPoint(pt2in)
            var pt1x = perspectiveDivide(pt1)
            var pt2x = perspectiveDivide(pt2)
            //console.log("projected " + pt1 +  " -- " + pt2)
            console.log("pers " + pt1x + " -- " + pt2x )

            regCode(pt1)
            regCode(pt2)
            var c1 = regCodeX(pt1)
            var c2 = regCodeX(pt2)
            if (!(c1|c2))
                console.log("accept")
            else if (c1&c2)
                console.log("reject")
            else
                console.log("clip!")

            if ((c1 & 16) || (c2 & 16))
                console.log("clip near!")

            var u = (-pt1.z - pt1.w) / ((pt2.z-pt1.z) + (pt2.w - pt1.w));
            var px = pt1.plus(pt2.minus(pt1).times(u))
            //var px = Qt.vector4d( pt1.x + u*(pt2.x-pt1.x), pt1.y + u*(pt2.y-pt1.y), pt1.z + u*(pt2.z-pt1.z), pt1.w + u*(pt2.w-pt1.w) );
            var pxx = perspectiveDivide(px);
            console.log("int point " + pxx  + "  u " + u)

            var nearPlaneNormal = camera.viewVector
            var nearPlanePoint = camera.position.plus(nearPlaneNormal.times(camera.nearPlane))
            var d = (nearPlaneNormal.dotProduct(nearPlanePoint.minus(pt1in)) / lineDir.dotProduct(nearPlaneNormal))
            var intPt = pt1in.plus(lineDir.times(d))

            console.log("int_p1" + "---" +  "    " + d + "    " + intPt)

            //var lineDir2 = pt1in.minus(pt2in)
            //var d2 = (nearPlaneNormal.dotProduct(nearPlanePoint.minus(pt2in)) / lineDir2.dotProduct(nearPlaneNormal))
            //var intPt2 = pt2in.plus(lineDir2.times(d))

            //console.log("int_p2" + "---" +  "    " + d2 + "    " + intPt2)

        }

        onPositionChanged: tstLineClipping()
        onViewCenterChanged: tstLineClipping()
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
            primitiveType: GeometryRenderer.LineStripAdjacency
            primitiveRestartEnabled: true
            restartIndexValue: 0
            //primitiveType: GeometryRenderer.Lines
            vertexCount: _lmg.count // _posData.count
            geometry: _lmg

// for some unknown reason this simply does not work
// (anyway it's a pain that one can't put data into QBuffer from JS)
//
//            geometry: Geometry {
//                attributes: [
//                    Attribute {
//                        name: Attribute.defaultPositionAttributeName()
//                        attributeType: Attribute.VertexAttribute
//                        vertexBaseType:  Attribute.Float
//                        vertexSize: 3
//                        byteOffset: 0
//                        //byteStride: 4*3
//                        buffer: _posData.buffer
//                        count: _posData.count
//                    }
//                ]
//            }
        }


        Material {
            id: grm

            parameters: [
                Parameter { name: "THICKNESS"; value: 10 },
                Parameter { name: "MITER_LIMIT"; value: -1 }, // 0.75 },
                Parameter { name: "WIN_SCALE"; value: Qt.size(_window.width,_window.height) },
                Parameter { name: "tex0"; value: txt },
                Parameter { name: "lineColor"; value: Qt.rgba(0,1,0,1) },
                Parameter { name: "useTex"; value: false },

                Parameter { name: "camNearPlanePoint"; value: camera.position.plus(camera.viewVector.times(camera.nearPlane)) },
                Parameter { name: "camNearPlaneNormal"; value: camera.viewVector }
            ]

            Texture2D {
                id : txt
                generateMipMaps : false
                magnificationFilter : Texture.Linear
                minificationFilter : Texture.Linear
                textureImages: [
                    TextureImage {
                        source: "qrc:/line-texture.png"
                    }
                ]
            }

            effect: Effect {
                techniques: Technique {
                    graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
                    renderPasses: [
                        RenderPass {

                            // enable alpha blending
                            renderStates: [
                                BlendEquation {
                                    blendFunction: BlendEquation.Add
                                },
                                BlendEquationArguments {
                                    sourceRgb: BlendEquationArguments.SourceAlpha
                                    destinationRgb: BlendEquationArguments.OneMinusSourceAlpha
                                }
                            ]

                            shaderProgram: ShaderProgram {
                                vertexShaderCode: loadSource("qrc:/shaders/lines.vert")
                                geometryShaderCode: loadSource("qrc:/shaders/lines.geom")
                                fragmentShaderCode: loadSource("qrc:/shaders/lines.frag")
                            }
                        }
                    ]
                }
            }
        }

        components:  [ gr, grm ]
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
        //enabled: false
    }

}

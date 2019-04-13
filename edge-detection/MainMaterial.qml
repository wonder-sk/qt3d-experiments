import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0

// material that renders normals
Material {
    id: normalMaterial

    property alias ambient: paramKA.value

    parameters: [
        Parameter { id: paramKA; name: "ka"; value: Qt.rgba(0.05, 0.05, 0.05, 1.0) },
        Parameter { name: "kd"; value: Qt.rgba(0.7, 0.7, 0.7, 1.0) },
        Parameter { name: "ks"; value: Qt.rgba(0.01, 0.01, 0.01, 1.0) },
        Parameter { name: "shininess"; value: 150. }
    ]

    effect: Effect {
        techniques: Technique {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
            renderPasses: [
                RenderPass {
                    filterKeys: [ FilterKey { name: "name"; value: "prepass" } ]
                    shaderProgram: ShaderProgram {
                        vertexShaderCode: "
#version 140

in vec3 vertexPosition;
in vec3 vertexNormal;
uniform mat4 modelViewProjection;

out vec3 norm;

void main()
{
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
    norm = vertexNormal;
}"
                        fragmentShaderCode: "
#version 140

in vec3 norm;
out vec4 normColor;

void main()
{
    normColor = vec4(norm, 1);
}
"
                    }
                },
                RenderPass {
                    filterKeys: [ FilterKey { name: "name"; value: "forward" } ]
                    shaderProgram: ShaderProgram {
                        vertexShaderCode: loadSource("qrc:/phong.vert")
                        fragmentShaderCode: loadSource("qrc:/phong.frag")
                    }
                }
            ]
        }
    }

}

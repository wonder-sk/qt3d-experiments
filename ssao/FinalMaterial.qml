import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0


Material {

    property Texture2D textureColor
    property Texture2D textureSsao

    parameters: [
        Parameter { name: "col"; value: textureColor },
        Parameter { name: "ssao"; value: textureSsao }
    ]
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

void main()
{
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}"


                        fragmentShaderCode: "
#version 420 core

uniform sampler2D col;
uniform sampler2D ssao;

out vec4 fragColor;

void main()
{
    float ssao = texelFetch(ssao, ivec2(gl_FragCoord), 0 ).r;
    fragColor = vec4(texelFetch(col, ivec2(gl_FragCoord), 0 ).rgb * ssao, 1.0);
}
"

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

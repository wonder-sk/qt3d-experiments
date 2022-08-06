import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0


Material {

    property Texture2D textureSsao

    parameters: [
        Parameter { name: "ssaoInput"; value: textureSsao }
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
#version 330 core

out float FragColor;

uniform sampler2D ssaoInput;

void main()
{
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            result += texelFetch(ssaoInput, ivec2(gl_FragCoord) + ivec2(x,y), 0).r;
        }
    }
    FragColor = result / (4.0 * 4.0);
}  "

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

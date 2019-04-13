import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0


Material {

    property Texture2D textureColor
    property Texture2D textureNormal
    property Texture2D textureDepth
    property real cameraZNear
    property real cameraZFar
    property color edgeColor: "black"

    parameters: [
        Parameter { name: "nor"; value: textureNormal },
        Parameter { name: "dep"; value: textureDepth },
        Parameter { name: "zNear"; value: cameraZNear },
        Parameter { name: "zFar"; value: cameraZFar },
        Parameter { name: "edgeColor"; value: edgeColor }
    ]
    effect: Effect {
        techniques: Technique {
            graphicsApiFilter { api: GraphicsApiFilter.OpenGL; profile: GraphicsApiFilter.CoreProfile; majorVersion: 3; minorVersion: 1 }
            renderPasses: [
                RenderPass {
                    shaderProgram: ShaderProgram {
                        id: sp
                        vertexShaderCode: "
#version 140

in vec3 vertexPosition;

uniform mat4 modelViewProjection;

void main()
{
    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}"
                        fragmentShaderCode: "
#version 140

// name must be equal to parameter of the Material
uniform sampler2D nor;
uniform sampler2D dep;

//in vec4 gl_FragCoord;  //  in window space (not normalized coords)
out vec4 fragColor;

uniform float zNear;
uniform float zFar;
uniform vec4 edgeColor;

// result suitable for assigning to gl_FragDepth
float depthSample(float linearDepth)
{
    float nonLinearDepth = (zFar + zNear - 2.0 * zNear * zFar / linearDepth) / (zFar - zNear);
    nonLinearDepth = (nonLinearDepth + 1.0) / 2.0;
    return nonLinearDepth;
}

float depthSampleToDepth(float depthSample)
{
   depthSample = 2.0 * depthSample - 1.0;
   float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
   return zLinear;
}

float get_info_depth(sampler2D s)
{
    ivec2 baseCoord = ivec2(gl_FragCoord);
    float s00 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(-1,-1), 0).r);
    float s01 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(-1,0), 0).r);
    float s02 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(-1,1), 0).r);
    float s10 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(0,-1), 0).r);
    float s12 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(0,1), 0).r);
    float s20 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(1,-1), 0).r);
    float s21 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(1,0), 0).r);
    float s22 = depthSampleToDepth(texelFetch(s, baseCoord + ivec2(1,1), 0).r);

    float gx = -1*s00 + 1*s02 + -2*s10 + 2*s12 + (-1)*s20 + 1*s22;
    float gy = -1*s00 + 1*s20 + -2*s01 + 2*s21 + (-1)*s02 + 1*s22;
    float g = sqrt(pow(gx, 2.0)+pow(gy, 2.0));
    return g;
}

float get_info_norm(sampler2D s)
{
    ivec2 baseCoord = ivec2(gl_FragCoord);
    vec3 s00 = texelFetch(s, baseCoord + ivec2(-1,-1), 0).rgb;
    vec3 s01 = texelFetch(s, baseCoord + ivec2(-1,0), 0).rgb;
    vec3 s02 = texelFetch(s, baseCoord + ivec2(-1,1), 0).rgb;
    vec3 s10 = texelFetch(s, baseCoord + ivec2(0,-1), 0).rgb;
    vec3 s12 = texelFetch(s, baseCoord + ivec2(0,1), 0).rgb;
    vec3 s20 = texelFetch(s, baseCoord + ivec2(1,-1), 0).rgb;
    vec3 s21 = texelFetch(s, baseCoord + ivec2(1,0), 0).rgb;
    vec3 s22 = texelFetch(s, baseCoord + ivec2(1,1), 0).rgb;

    vec3 gx = -1*s00 + 1*s02 + -2*s10 + 2*s12 + (-1)*s20 + 1*s22;
    vec3 gy = -1*s00 + 1*s20 + -2*s01 + 2*s21 + (-1)*s02 + 1*s22;
    float g = sqrt(pow(length(gx), 2.0)+pow(length(gy), 2.0));
    return g;
}

void main()
{
    // for testing to display raw data
    //fragColor = vec4(texture(nor, texCoord).rgb, 1.0);
    //fragColor = vec4(texture(dep, texCoord).rrr, 1.0);

    // apply Sobel filter on both normals and depths and figure out strength of edges by combining those
    float edge_depth = get_info_depth(dep) / 2;
    float edge_norm = get_info_norm(nor) / 4;
    float edge_strength = edge_depth + edge_norm;
    if (edge_strength < 0.5)
        discard;
    fragColor = edgeColor;

    // use the same depth as was used by the original rendering
    // (this makes sure that if a part of our post-processed geometry is behind
    // an object that's not being post-processed, that part will be ignored
    // and no edges will be drawn)
    gl_FragDepth = texelFetch(dep, ivec2(gl_FragCoord), 0 ).r;
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

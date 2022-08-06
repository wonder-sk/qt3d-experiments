import QtQuick 2.1 as QQ2
import Qt3D.Core 2.0
import Qt3D.Render 2.0
import Qt3D.Input 2.0
import Qt3D.Extras 2.0


Material {

    property Texture2D textureColor
    property Texture2D textureDepth
    property real cameraZNear
    property real cameraZFar
    property real cameraFov
    property real cameraAr
    property matrix4x4 cameraProjMatrix

    // create a list of randomly generated 3D vectors inside a unit sphere for sampling neighbors
    function generate_kernel() {
        var lst = []
        var kernelSize = 64;
        for (var i = 0; i < kernelSize; ++i) {
            var scale = i / kernelSize
            scale = 0.1 + 0.9 * scale * scale
            var vct = Qt.vector3d(Math.random()*2-1, Math.random()*2-1, Math.random()*2-1).normalized()
            vct = vct.times(scale)
            lst.push(vct)
        }
        return lst
    }

    // 4x4 array of random rotation vectors
    function generate_random_texture() {
        var lst = []
        for (var i = 0; i < 256; ++i) {
            var vct = Qt.vector3d(Math.random() /**2-1*/, Math.random() /**2-1*/, 0.0)
            lst.push(vct)
        }
        return lst;
    }

    parameters: [
        Parameter { name: "col"; value: textureColor },
        Parameter { name: "dep"; value: textureDepth },
        Parameter { name: "zNear"; value: cameraZNear },
        Parameter { name: "zFar"; value: cameraZFar },
        Parameter { name: "uKernelOffsets[0]"; value: generate_kernel() },
        Parameter { name: "uNoiseTexture[0]"; value: generate_random_texture() },
        Parameter { name: "origProjMatrix"; value: cameraProjMatrix },
        Parameter { name: "uTanHalfFov"; value: Math.tan( cameraFov/2 * Math.PI/180) },
        Parameter { name: "uAspectRatio"; value: cameraAr }
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

//	view frustum parameters:
uniform float uTanHalfFov;
uniform float uAspectRatio;

noperspective out vec3 vViewRay; // ray to far plane

void main()
{
    // math of the view ray is covered here: https://ogldev.org/www/tutorial46/tutorial46.html
    //
    // our quad is in X-Z plane.
    // and its coordinates are in range [-0.5,0.5] that's why we multiply by 2
    vViewRay = vec3(
        -vertexPosition.x * uTanHalfFov * uAspectRatio * 2,
        vertexPosition.z * uTanHalfFov * 2,
        1.0 // since we'll be multiplying by linear depth, leave z as 1
    );

    gl_Position = modelViewProjection * vec4( vertexPosition, 1.0 );
}"


                        fragmentShaderCode: "
#version 420 core

// name must be equal to parameter of the Material
uniform sampler2D col;
uniform sampler2D dep;

uniform float zNear;
uniform float zFar;

uniform vec3 uKernelOffsets[64];  // unit sphere with random vectors in it
uniform vec3 uNoiseTexture[256];  //
uniform mat4 origProjMatrix;      // perspective projection matrix used for forward rendering

noperspective in vec3 vViewRay;   // ray to far plane

out vec4 fragColor;


// converts depth from depth texture (in range [0,1]) to 'linear' depth based on camera settings
float depthSampleToDepth(float depthSample)
{
   depthSample = 2.0 * depthSample - 1.0;
   float zLinear = 2.0 * zNear * zFar / (zFar + zNear - depthSample * (zFar - zNear));
   return zLinear;
}


vec3 rotate_x(vec3 vct, float angle)
{
  return vec3(vct.x*cos(angle)-vct.y*sin(angle), vct.x*sin(angle)+vct.y*cos(angle), vct.z);
}
vec3 rotate_y(vec3 vct, float angle)
{
  return vec3(vct.x*cos(angle)+vct.z*sin(angle), vct.y, -vct.x*sin(angle)+vct.z*cos(angle));
}


// based on the code from John Chapman
float ssao(vec3 originPos, float radius, vec3 noise)
{
    float occlusion = 0.0;
    for (int i = 0; i < 64; ++i)
    {
        //	get sample position:
        vec3 samplePos = rotate_y(rotate_x(uKernelOffsets[i], noise.x), noise.y);
        samplePos = samplePos + originPos;

        //	project sample position:
        vec4 offset = origProjMatrix * vec4(samplePos, 1.0);
        offset.xy /= offset.w;               // only need xy   (range [-1,1])
        offset.xy = offset.xy * 0.5 + 0.5;   // scale/bias to texcoords  (range [0,1])

        //	get sample depth:
        float sampleDepth = texture(dep, offset.xy).r;
        sampleDepth = depthSampleToDepth(sampleDepth);

        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(originPos.z - sampleDepth));
        occlusion += rangeCheck * step(sampleDepth, samplePos.z);
    }

    return 1.0 - (occlusion / float(64));
}


void main()
{
    // for debugging
    //fragColor = vec4(texelFetch(col, ivec2(gl_FragCoord), 0 ).rgb, 1.0);
    //float shade = exp(-edlFactor(ivec2(gl_FragCoord)) * 4000);
    //fragColor = vec4(fragColor.rgb * shade, fragColor.a);
    //fragColor = vec4(vViewRay.x, vViewRay.y, 0.0,1.0);

    // calculate view-space 3D coordinates of this pixel
    vec2 texelSize = 1.0 / vec2(textureSize(dep, 0));
    vec2 screenTexCoords = gl_FragCoord.xy * texelSize;
    float originDepth = depthSampleToDepth(texture(dep, screenTexCoords).r);
    vec3 originPos = vViewRay * originDepth;

    vec4 originColor = vec4(texture(col, screenTexCoords).rgb, 1.0);

    int a_idx = int(gl_FragCoord.x) % 16 + 16 * (int(gl_FragCoord.y) % 16);
    vec3 noise = uNoiseTexture[a_idx] * 2*3.14;

    float ssao_res = ssao(originPos, 0.5, noise);
    //fragColor = originColor * pow(ssao_res, 1.0);

    // more debugging
    //fragColor = vec4(vec3(originDepth/100),1.0);
    //fragColor = vec4(originPos.x, originPos.y, 0.0, 1.0);
    fragColor = vec4(pow(ssao_res, 1.0));
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

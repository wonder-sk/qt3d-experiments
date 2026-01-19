#ifndef SPLATRENDERER_H
#define SPLATRENDERER_H

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DCore/QBuffer>
#include <Qt3DCore/QGeometry>
#include <Qt3DCore/QAttribute>

#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QTechnique>
#include <Qt3DRender/QRenderPass>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QBlendEquation>
#include <Qt3DRender/QBlendEquationArguments>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QNoDepthMask>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QFilterKey>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QTextureImage>
#include <Qt3DRender/QTextureImageDataGenerator>
#include <Qt3DRender/QAbstractTexture>
#include <Qt3DRender/QTextureWrapMode>

#include <QVector>
#include <QTimer>
#include <QImage>

#include "gaussiansplat.h"

// Texture data generator for splat data
class SplatTextureDataGenerator : public Qt3DRender::QTextureImageDataGenerator {
public:
    SplatTextureDataGenerator(const QByteArray& data, int width, int height, int index)
        : m_data(data), m_width(width), m_height(height), m_index(index) {}

    Qt3DRender::QTextureImageDataPtr operator()() override {
        Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
        dataPtr->setWidth(m_width);
        dataPtr->setHeight(m_height);
        dataPtr->setDepth(1);
        dataPtr->setFaces(1);
        dataPtr->setLayers(1);
        dataPtr->setMipLevels(1);
        dataPtr->setFormat(QOpenGLTexture::RGBA32F);
        dataPtr->setPixelFormat(QOpenGLTexture::RGBA);
        dataPtr->setPixelType(QOpenGLTexture::Float32);
        // blockSize = 1 for uncompressed data (byte alignment)
        dataPtr->setData(m_data, 1, false);
        qDebug() << "SplatTextureDataGenerator: Generated texture" << m_index
                 << "size:" << m_width << "x" << m_height
                 << "data bytes:" << m_data.size();
        return dataPtr;
    }

    bool operator==(const Qt3DRender::QTextureImageDataGenerator& other) const override {
        const SplatTextureDataGenerator* otherGen = dynamic_cast<const SplatTextureDataGenerator*>(&other);
        return otherGen && otherGen->m_index == m_index && otherGen->m_data == m_data;
    }

    QT3D_FUNCTOR(SplatTextureDataGenerator)

private:
    QByteArray m_data;
    int m_width;
    int m_height;
    int m_index;
};

// Custom texture image that uses our data generator
class SplatTextureImage : public Qt3DRender::QAbstractTextureImage {
    Q_OBJECT
public:
    explicit SplatTextureImage(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QAbstractTextureImage(parent) {}

    void setDataGenerator(const QSharedPointer<Qt3DRender::QTextureImageDataGenerator>& gen) {
        m_generator = gen;
        notifyDataGeneratorChanged();
    }

protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override {
        return m_generator;
    }

private:
    QSharedPointer<Qt3DRender::QTextureImageDataGenerator> m_generator;
};

// Simple quad geometry for instanced rendering
class QuadGeometry : public Qt3DCore::QGeometry {
    Q_OBJECT
public:
    explicit QuadGeometry(Qt3DCore::QNode *parent = nullptr)
        : Qt3DCore::QGeometry(parent)
        , m_vertexBuffer(new Qt3DCore::QBuffer(this))
        , m_indexBuffer(new Qt3DCore::QBuffer(this))
        , m_positionAttribute(new Qt3DCore::QAttribute(this))
        , m_indexAttribute(new Qt3DCore::QAttribute(this))
    {
        setupGeometry();
    }

private:
    void setupGeometry() {
        // Single quad with corners at [-1,-1] to [1,1]
        // These are the quad offsets used for billboarding
        const float vertices[] = {
            // position (x, y) - z is always 0 for the base quad
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f
        };

        const quint16 indices[] = {
            0, 1, 2,
            0, 2, 3
        };

        QByteArray vertexData(reinterpret_cast<const char*>(vertices), sizeof(vertices));
        QByteArray indexData(reinterpret_cast<const char*>(indices), sizeof(indices));

        m_vertexBuffer->setData(vertexData);
        m_indexBuffer->setData(indexData);

        // Position attribute (2D quad offset)
        m_positionAttribute->setName(Qt3DCore::QAttribute::defaultPositionAttributeName());
        m_positionAttribute->setVertexBaseType(Qt3DCore::QAttribute::Float);
        m_positionAttribute->setVertexSize(2);
        m_positionAttribute->setAttributeType(Qt3DCore::QAttribute::VertexAttribute);
        m_positionAttribute->setBuffer(m_vertexBuffer);
        m_positionAttribute->setByteStride(2 * sizeof(float));
        m_positionAttribute->setByteOffset(0);
        m_positionAttribute->setCount(4);

        // Index attribute
        m_indexAttribute->setVertexBaseType(Qt3DCore::QAttribute::UnsignedShort);
        m_indexAttribute->setAttributeType(Qt3DCore::QAttribute::IndexAttribute);
        m_indexAttribute->setBuffer(m_indexBuffer);
        m_indexAttribute->setCount(6);

        addAttribute(m_positionAttribute);
        addAttribute(m_indexAttribute);
    }

    Qt3DCore::QBuffer *m_vertexBuffer;
    Qt3DCore::QBuffer *m_indexBuffer;
    Qt3DCore::QAttribute *m_positionAttribute;
    Qt3DCore::QAttribute *m_indexAttribute;
};

class SplatMaterial : public Qt3DRender::QMaterial {
    Q_OBJECT
public:
    explicit SplatMaterial(Qt3DCore::QNode *parent = nullptr)
        : Qt3DRender::QMaterial(parent)
        , m_effect(new Qt3DRender::QEffect(this))
        , m_technique(new Qt3DRender::QTechnique(this))
        , m_renderPass(new Qt3DRender::QRenderPass(this))
        , m_shaderProgram(new Qt3DRender::QShaderProgram(this))
        , m_dataTexture1(new Qt3DRender::QTexture2D(this))
        , m_dataTexture2(new Qt3DRender::QTexture2D(this))
        , m_dataTexture3(new Qt3DRender::QTexture2D(this))
        , m_dataTexture4(new Qt3DRender::QTexture2D(this))
        , m_texture1Param(new Qt3DRender::QParameter(QStringLiteral("splatData1"), m_dataTexture1, this))
        , m_texture2Param(new Qt3DRender::QParameter(QStringLiteral("splatData2"), m_dataTexture2, this))
        , m_texture3Param(new Qt3DRender::QParameter(QStringLiteral("splatData3"), m_dataTexture3, this))
        , m_texture4Param(new Qt3DRender::QParameter(QStringLiteral("splatData4"), m_dataTexture4, this))
        , m_textureWidthParam(new Qt3DRender::QParameter(QStringLiteral("textureWidth"), 1, this))
        , m_splatCountParam(new Qt3DRender::QParameter(QStringLiteral("splatCount"), 0, this))
        , m_viewportSizeParam(new Qt3DRender::QParameter(QStringLiteral("viewportSize"), QVector2D(800, 600), this))
        , m_focalLengthParam(new Qt3DRender::QParameter(QStringLiteral("focalLength"), QVector2D(800, 800), this))
    {
        setupTextures();
        setupShaders();
        setupRenderStates();

        // Use OpenGL 4.5 backend for better GLSL compatibility
        m_technique->graphicsApiFilter()->setApi(Qt3DRender::QGraphicsApiFilter::OpenGL);
        m_technique->graphicsApiFilter()->setMajorVersion(4);
        m_technique->graphicsApiFilter()->setMinorVersion(5);
        m_technique->graphicsApiFilter()->setProfile(Qt3DRender::QGraphicsApiFilter::CoreProfile);

        Qt3DRender::QFilterKey *filterKey = new Qt3DRender::QFilterKey(this);
        filterKey->setName(QStringLiteral("renderingStyle"));
        filterKey->setValue(QStringLiteral("forward"));
        m_technique->addFilterKey(filterKey);

        m_technique->addRenderPass(m_renderPass);
        m_effect->addTechnique(m_technique);

        // Add parameters
        m_effect->addParameter(m_texture1Param);
        m_effect->addParameter(m_texture2Param);
        m_effect->addParameter(m_texture3Param);
        m_effect->addParameter(m_texture4Param);
        m_effect->addParameter(m_textureWidthParam);
        m_effect->addParameter(m_splatCountParam);
        m_effect->addParameter(m_viewportSizeParam);
        m_effect->addParameter(m_focalLengthParam);

        setEffect(m_effect);
    }

    void setSplatData(const QVector<GaussianSplat>& splats) {
        if (splats.isEmpty()) return;

        const int splatCount = splats.size();

        // Calculate texture dimensions (power of 2 width, enough height)
        int textureWidth = 1;
        while (textureWidth * textureWidth < splatCount) {
            textureWidth *= 2;
        }
        int textureHeight = (splatCount + textureWidth - 1) / textureWidth;
        textureHeight = qMax(1, textureHeight);

        qDebug() << "Splat texture:" << textureWidth << "x" << textureHeight
                 << "for" << splatCount << "splats";

        // Texture layout with covariance:
        // Texture 1: position.xyz, cov.xx
        // Texture 2: cov.xy, cov.xz, cov.yy, cov.yz
        // Texture 3: cov.zz, opacity, 0, 0
        // Texture 4: color.rgb, 0
        QByteArray data1, data2, data3, data4;
        int texelBytes = textureWidth * textureHeight * 4 * sizeof(float);
        data1.resize(texelBytes);
        data2.resize(texelBytes);
        data3.resize(texelBytes);
        data4.resize(texelBytes);

        float* ptr1 = reinterpret_cast<float*>(data1.data());
        float* ptr2 = reinterpret_cast<float*>(data2.data());
        float* ptr3 = reinterpret_cast<float*>(data3.data());
        float* ptr4 = reinterpret_cast<float*>(data4.data());

        for (int i = 0; i < splatCount; ++i) {
            const GaussianSplat& splat = splats[i];
            int idx = i * 4;

            // Texture 1: position.xyz, cov.xx
            ptr1[idx + 0] = splat.position.x();
            ptr1[idx + 1] = splat.position.y();
            ptr1[idx + 2] = splat.position.z();
            ptr1[idx + 3] = splat.cov3d.data[0];  // xx

            // Texture 2: cov.xy, cov.xz, cov.yy, cov.yz
            ptr2[idx + 0] = splat.cov3d.data[1];  // xy
            ptr2[idx + 1] = splat.cov3d.data[2];  // xz
            ptr2[idx + 2] = splat.cov3d.data[3];  // yy
            ptr2[idx + 3] = splat.cov3d.data[4];  // yz

            // Texture 3: cov.zz, opacity, 0, 0
            ptr3[idx + 0] = splat.cov3d.data[5];  // zz
            ptr3[idx + 1] = splat.opacity;
            ptr3[idx + 2] = 0.0f;
            ptr3[idx + 3] = 0.0f;

            // Texture 4: color.rgb, 0
            ptr4[idx + 0] = splat.color.x();
            ptr4[idx + 1] = splat.color.y();
            ptr4[idx + 2] = splat.color.z();
            ptr4[idx + 3] = 0.0f;
        }

        // Fill remaining texels with zeros
        for (int i = splatCount; i < textureWidth * textureHeight; ++i) {
            int idx = i * 4;
            ptr1[idx + 0] = ptr1[idx + 1] = ptr1[idx + 2] = ptr1[idx + 3] = 0.0f;
            ptr2[idx + 0] = ptr2[idx + 1] = ptr2[idx + 2] = ptr2[idx + 3] = 0.0f;
            ptr3[idx + 0] = ptr3[idx + 1] = ptr3[idx + 2] = ptr3[idx + 3] = 0.0f;
            ptr4[idx + 0] = ptr4[idx + 1] = ptr4[idx + 2] = ptr4[idx + 3] = 0.0f;
        }

        // Create texture images with data generators
        auto gen1 = QSharedPointer<SplatTextureDataGenerator>::create(data1, textureWidth, textureHeight, 1);
        auto gen2 = QSharedPointer<SplatTextureDataGenerator>::create(data2, textureWidth, textureHeight, 2);
        auto gen3 = QSharedPointer<SplatTextureDataGenerator>::create(data3, textureWidth, textureHeight, 3);
        auto gen4 = QSharedPointer<SplatTextureDataGenerator>::create(data4, textureWidth, textureHeight, 4);

        // Remove old texture images and add new ones
        for (auto* tex : {m_dataTexture1, m_dataTexture2, m_dataTexture3, m_dataTexture4}) {
            for (auto* img : tex->textureImages()) {
                tex->removeTextureImage(img);
                delete img;
            }
        }

        SplatTextureImage* img1 = new SplatTextureImage(m_dataTexture1);
        img1->setDataGenerator(gen1);
        m_dataTexture1->addTextureImage(img1);

        SplatTextureImage* img2 = new SplatTextureImage(m_dataTexture2);
        img2->setDataGenerator(gen2);
        m_dataTexture2->addTextureImage(img2);

        SplatTextureImage* img3 = new SplatTextureImage(m_dataTexture3);
        img3->setDataGenerator(gen3);
        m_dataTexture3->addTextureImage(img3);

        SplatTextureImage* img4 = new SplatTextureImage(m_dataTexture4);
        img4->setDataGenerator(gen4);
        m_dataTexture4->addTextureImage(img4);

        // Update parameters
        m_textureWidthParam->setValue(textureWidth);
        m_splatCountParam->setValue(splatCount);
        m_textureWidth = textureWidth;

        qDebug() << "Splat data textures updated (with covariance)";
    }

    void setViewportSize(const QVector2D& size) {
        m_viewportSizeParam->setValue(size);
    }

    void setFocalLength(const QVector2D& focal) {
        m_focalLengthParam->setValue(focal);
    }

private:
    void setupTextures() {
        // Configure all data textures as RGBA32F
        for (auto* tex : {m_dataTexture1, m_dataTexture2, m_dataTexture3, m_dataTexture4}) {
            tex->setFormat(Qt3DRender::QAbstractTexture::RGBA32F);
            tex->setMinificationFilter(Qt3DRender::QAbstractTexture::Nearest);
            tex->setMagnificationFilter(Qt3DRender::QAbstractTexture::Nearest);
            tex->wrapMode()->setX(Qt3DRender::QTextureWrapMode::ClampToEdge);
            tex->wrapMode()->setY(Qt3DRender::QTextureWrapMode::ClampToEdge);
            tex->setGenerateMipMaps(false);
        }
    }

    void setupShaders() {
        // Vertex shader for instanced splat rendering with 3D covariance
        const char* vertexShader = R"(
#version 450 core

layout(location = 0) in vec2 vertexPosition;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragOffset;
layout(location = 2) out float fragOpacity;
layout(location = 3) out vec3 fragCov2d;  // 2D covariance: [a, b, c] for [[a,b],[b,c]]

// Qt3D uniforms
uniform mat4 modelViewProjection;
uniform mat4 modelView;
uniform mat4 projectionMatrix;

// Splat data textures
// Texture 1: position.xyz, cov.xx
// Texture 2: cov.xy, cov.xz, cov.yy, cov.yz
// Texture 3: cov.zz, opacity, 0, 0
// Texture 4: color.rgb, 0
uniform sampler2D splatData1;
uniform sampler2D splatData2;
uniform sampler2D splatData3;
uniform sampler2D splatData4;

// Custom uniforms
uniform int textureWidth;
uniform vec2 viewportSize;
uniform vec2 focalLength;

// Compute 2D covariance from 3D covariance in view space
// Returns [cov2d_00, cov2d_01, cov2d_11] and the radius for the bounding quad
vec4 computeCov2D(vec3 viewPos, mat3 cov3d, vec2 focal, vec2 viewport) {
    float z = viewPos.z;
    float z2 = z * z;

    // Jacobian of perspective projection
    // J = [[fx/z, 0, -fx*x/z^2],
    //      [0, fy/z, -fy*y/z^2]]
    float fx = focal.x;
    float fy = focal.y;

    float J00 = fx / z;
    float J02 = -fx * viewPos.x / z2;
    float J11 = fy / z;
    float J12 = -fy * viewPos.y / z2;

    // Compute J * cov3d * J^T
    // First compute T = J * cov3d (2x3 matrix)
    float t00 = J00 * cov3d[0][0] + J02 * cov3d[2][0];
    float t01 = J00 * cov3d[0][1] + J02 * cov3d[2][1];
    float t02 = J00 * cov3d[0][2] + J02 * cov3d[2][2];
    float t10 = J11 * cov3d[1][0] + J12 * cov3d[2][0];
    float t11 = J11 * cov3d[1][1] + J12 * cov3d[2][1];
    float t12 = J11 * cov3d[1][2] + J12 * cov3d[2][2];

    // Now compute cov2d = T * J^T (2x2 matrix)
    float cov2d_00 = t00 * J00 + t02 * J02;
    float cov2d_01 = t00 * 0.0 + t01 * J11 + t02 * J12;
    float cov2d_11 = t10 * 0.0 + t11 * J11 + t12 * J12;

    // Add small value for numerical stability
    cov2d_00 += 0.3;
    cov2d_11 += 0.3;

    // Compute eigenvalues for bounding radius
    float det = cov2d_00 * cov2d_11 - cov2d_01 * cov2d_01;
    float trace = cov2d_00 + cov2d_11;
    float discriminant = max(trace * trace - 4.0 * det, 0.0);
    float sqrtDisc = sqrt(discriminant);
    float lambda1 = (trace + sqrtDisc) * 0.5;
    float lambda2 = (trace - sqrtDisc) * 0.5;

    // Radius is 3 sigma of the larger eigenvalue (covers 99.7%)
    float radius = 3.0 * sqrt(max(lambda1, lambda2));

    return vec4(cov2d_00, cov2d_01, cov2d_11, radius);
}

void main() {
    int splatIndex = gl_InstanceID;

    // Calculate texture coordinates for this splat
    int texWidth = max(textureWidth, 1);
    int texX = splatIndex % texWidth;
    int texY = splatIndex / texWidth;

    // Fetch splat data from textures
    vec4 data1 = texelFetch(splatData1, ivec2(texX, texY), 0);
    vec4 data2 = texelFetch(splatData2, ivec2(texX, texY), 0);
    vec4 data3 = texelFetch(splatData3, ivec2(texX, texY), 0);
    vec4 data4 = texelFetch(splatData4, ivec2(texX, texY), 0);

    // Unpack data
    vec3 splatPosition = data1.xyz;
    float cov_xx = data1.w;
    float cov_xy = data2.x;
    float cov_xz = data2.y;
    float cov_yy = data2.z;
    float cov_yz = data2.w;
    float cov_zz = data3.x;
    float opacity = data3.y;
    vec3 splatColor = data4.rgb;

    // Build 3D covariance matrix (symmetric)
    mat3 cov3d = mat3(
        cov_xx, cov_xy, cov_xz,
        cov_xy, cov_yy, cov_yz,
        cov_xz, cov_yz, cov_zz
    );

    // Transform position to view space
    vec4 viewPos4 = modelView * vec4(splatPosition, 1.0);
    vec3 viewPos = viewPos4.xyz;

    // Skip splats behind camera
    if (viewPos.z >= -0.1) {
        gl_Position = vec4(0.0, 0.0, -1000.0, 1.0);
        return;
    }

    // Transform covariance to view space: W * cov3d * W^T
    mat3 W = mat3(modelView);
    mat3 viewCov3d = W * cov3d * transpose(W);

    // Compute 2D covariance and bounding radius
    vec2 focal = max(focalLength, vec2(100.0));
    vec2 viewport = max(viewportSize, vec2(100.0));
    vec4 cov2dResult = computeCov2D(viewPos, viewCov3d, focal, viewport);

    vec3 cov2d = cov2dResult.xyz;
    float radius = cov2dResult.w;

    // Project center to clip space
    vec4 clipPos = projectionMatrix * viewPos4;

    // Convert radius from pixels to NDC
    vec2 ndcRadius = radius * 2.0 / viewport;
    ndcRadius = max(ndcRadius, vec2(0.001));

    // Apply billboard offset (scaled by radius)
    clipPos.xy += vertexPosition * ndcRadius * clipPos.w;

    gl_Position = clipPos;

    // Pass to fragment shader
    fragColor = splatColor;
    fragOffset = vertexPosition * radius;  // Offset in pixel space
    fragOpacity = opacity;
    fragCov2d = cov2d;
}
)";

        // Fragment shader with oriented ellipse rendering
        const char* fragmentShader = R"(
#version 450 core

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragOffset;  // Offset from center in pixel space
layout(location = 2) in float fragOpacity;
layout(location = 3) in vec3 fragCov2d;   // 2D covariance [a, b, c]

layout(location = 0) out vec4 color;

void main() {
    // Get 2D covariance matrix components
    float a = fragCov2d.x;  // cov[0][0]
    float b = fragCov2d.y;  // cov[0][1] = cov[1][0]
    float c = fragCov2d.z;  // cov[1][1]

    // Compute inverse of 2D covariance matrix
    float det = a * c - b * b;
    if (det <= 0.0) discard;

    float invDet = 1.0 / det;
    float invA = c * invDet;
    float invB = -b * invDet;
    float invC = a * invDet;

    // Compute Mahalanobis distance: d^2 = offset^T * Sigma^-1 * offset
    vec2 p = fragOffset;
    float d2 = invA * p.x * p.x + 2.0 * invB * p.x * p.y + invC * p.y * p.y;

    // Gaussian: exp(-0.5 * d^2)
    float gaussian = exp(-0.5 * d2);

    // Discard pixels with negligible contribution
    if (gaussian < 0.02) discard;

    // Final alpha
    float finalAlpha = gaussian * fragOpacity;

    color = vec4(fragColor, finalAlpha);
}
)";

        m_shaderProgram->setVertexShaderCode(QByteArray(vertexShader));
        m_shaderProgram->setFragmentShaderCode(QByteArray(fragmentShader));
        m_renderPass->setShaderProgram(m_shaderProgram);

        // Log shader errors
        QObject::connect(m_shaderProgram, &Qt3DRender::QShaderProgram::statusChanged,
            [this](Qt3DRender::QShaderProgram::Status status) {
                qDebug() << "Shader status:" << status;
                if (status == Qt3DRender::QShaderProgram::Error) {
                    qWarning() << "Shader error:" << m_shaderProgram->log();
                }
            });
    }

    void setupRenderStates() {
        // Standard alpha blending
        Qt3DRender::QBlendEquation *blendEquation = new Qt3DRender::QBlendEquation(this);
        blendEquation->setBlendFunction(Qt3DRender::QBlendEquation::Add);
        m_renderPass->addRenderState(blendEquation);

        Qt3DRender::QBlendEquationArguments *blendArgs = new Qt3DRender::QBlendEquationArguments(this);
        blendArgs->setSourceRgb(Qt3DRender::QBlendEquationArguments::SourceAlpha);
        blendArgs->setDestinationRgb(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);
        blendArgs->setSourceAlpha(Qt3DRender::QBlendEquationArguments::One);
        blendArgs->setDestinationAlpha(Qt3DRender::QBlendEquationArguments::OneMinusSourceAlpha);
        m_renderPass->addRenderState(blendArgs);

        // Depth test but no depth write for proper transparency
        Qt3DRender::QDepthTest *depthTest = new Qt3DRender::QDepthTest(this);
        depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);
        m_renderPass->addRenderState(depthTest);

        Qt3DRender::QNoDepthMask *noDepthMask = new Qt3DRender::QNoDepthMask(this);
        m_renderPass->addRenderState(noDepthMask);

        // No face culling for billboards
        Qt3DRender::QCullFace *cullFace = new Qt3DRender::QCullFace(this);
        cullFace->setMode(Qt3DRender::QCullFace::NoCulling);
        m_renderPass->addRenderState(cullFace);
    }

    Qt3DRender::QEffect *m_effect;
    Qt3DRender::QTechnique *m_technique;
    Qt3DRender::QRenderPass *m_renderPass;
    Qt3DRender::QShaderProgram *m_shaderProgram;

    // Data textures (4 textures for position, covariance, opacity, color)
    Qt3DRender::QTexture2D *m_dataTexture1;
    Qt3DRender::QTexture2D *m_dataTexture2;
    Qt3DRender::QTexture2D *m_dataTexture3;
    Qt3DRender::QTexture2D *m_dataTexture4;

    // Parameters
    Qt3DRender::QParameter *m_texture1Param;
    Qt3DRender::QParameter *m_texture2Param;
    Qt3DRender::QParameter *m_texture3Param;
    Qt3DRender::QParameter *m_texture4Param;
    Qt3DRender::QParameter *m_textureWidthParam;
    Qt3DRender::QParameter *m_splatCountParam;
    Qt3DRender::QParameter *m_viewportSizeParam;
    Qt3DRender::QParameter *m_focalLengthParam;

    int m_textureWidth = 1;
};

class SplatRenderer : public Qt3DCore::QEntity {
    Q_OBJECT
public:
    explicit SplatRenderer(Qt3DCore::QNode *parent = nullptr)
        : Qt3DCore::QEntity(parent)
        , m_geometry(new QuadGeometry(this))
        , m_geometryRenderer(new Qt3DRender::QGeometryRenderer(this))
        , m_material(new SplatMaterial(this))
        , m_transform(new Qt3DCore::QTransform(this))
        , m_camera(nullptr)
        , m_sortTimer(new QTimer(this))
    {
        m_geometryRenderer->setGeometry(m_geometry);
        m_geometryRenderer->setPrimitiveType(Qt3DRender::QGeometryRenderer::Triangles);
        m_geometryRenderer->setVertexCount(6);  // Single quad: 6 indices
        m_geometryRenderer->setInstanceCount(0);  // Will be set when splats are loaded

        addComponent(m_geometryRenderer);
        addComponent(m_material);
        addComponent(m_transform);

        // Setup periodic sorting
        connect(m_sortTimer, &QTimer::timeout, this, &SplatRenderer::updateSorting);
        m_sortTimer->start(1000);
    }

    bool loadPLY(const QString& filename) {
        m_loadedSplats = GaussianSplatLoader::loadPLY(filename);
        if (m_loadedSplats.isEmpty()) {
            qWarning() << "No splats loaded from" << filename;
            return false;
        }

        // Compute bounds
        GaussianSplatLoader::computeBounds(m_loadedSplats, m_minBound, m_maxBound, m_center);
        qDebug() << "Scene bounds: min=" << m_minBound << "max=" << m_maxBound << "center=" << m_center;
        qDebug() << "Scene size:" << (m_maxBound - m_minBound);

        // Debug: print first few splats
        qDebug() << "First 3 splats:";
        for (int i = 0; i < qMin(3, m_loadedSplats.size()); ++i) {
            const auto& s = m_loadedSplats[i];
            qDebug() << "  Splat" << i << ": pos=" << s.position
                     << "scale=" << s.scale << "opacity=" << s.opacity
                     << "color=" << s.color;
        }

        // Upload splat data to textures
        m_material->setSplatData(m_loadedSplats);

        // Set instance count for rendering
        m_geometryRenderer->setInstanceCount(m_loadedSplats.size());

        qDebug() << "Loaded" << m_loadedSplats.size() << "splats using instanced rendering";
        qDebug() << "Instance count set to:" << m_geometryRenderer->instanceCount();
        qDebug() << "Vertex count:" << m_geometryRenderer->vertexCount();
        return true;
    }

    QVector3D sceneCenter() const { return m_center; }
    QVector3D sceneMin() const { return m_minBound; }
    QVector3D sceneMax() const { return m_maxBound; }
    float sceneRadius() const { return (m_maxBound - m_minBound).length() * 0.5f; }

    void setCamera(Qt3DRender::QCamera* camera) {
        m_camera = camera;
    }

    void setViewportSize(const QSize& size) {
        m_material->setViewportSize(QVector2D(size.width(), size.height()));

        // Update focal length based on camera FOV and viewport
        if (m_camera) {
            float fovRad = qDegreesToRadians(m_camera->fieldOfView());
            float fy = size.height() / (2.0f * std::tan(fovRad / 2.0f));
            float fx = fy;  // Assuming square pixels
            m_material->setFocalLength(QVector2D(fx, fy));
            qDebug() << "Viewport:" << size << "Focal length:" << fx;
        }
    }

    SplatMaterial* material() { return m_material; }

private slots:
    void updateSorting() {
        if (m_camera && !m_loadedSplats.isEmpty()) {
            GaussianSplatLoader::sortByDepth(m_loadedSplats, m_camera->position());
            m_material->setSplatData(m_loadedSplats);
            qDebug() << "Re-sorted" << m_loadedSplats.size() << "splats";
        }
    }

private:
    QuadGeometry *m_geometry;
    Qt3DRender::QGeometryRenderer *m_geometryRenderer;
    SplatMaterial *m_material;
    Qt3DCore::QTransform *m_transform;
    Qt3DRender::QCamera *m_camera;
    QTimer *m_sortTimer;

    QVector<GaussianSplat> m_loadedSplats;
    QVector3D m_minBound;
    QVector3D m_maxBound;
    QVector3D m_center;
};

#endif // SPLATRENDERER_H

#ifndef GAUSSIANSPLAT_H
#define GAUSSIANSPLAT_H

#include <QVector3D>
#include <QVector4D>
#include <QQuaternion>
#include <QMatrix3x3>
#include <QString>
#include <QVector>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <cmath>
#include <algorithm>

// 3D covariance matrix stored as 6 unique values (symmetric matrix)
// [cov[0], cov[1], cov[2]]   [xx, xy, xz]
// [cov[1], cov[3], cov[4]] = [xy, yy, yz]
// [cov[2], cov[4], cov[5]]   [xz, yz, zz]
struct Cov3D {
    float data[6];  // xx, xy, xz, yy, yz, zz

    Cov3D() { std::fill(std::begin(data), std::end(data), 0.0f); }
};

struct GaussianSplat {
    QVector3D position;
    QVector3D scale;
    QQuaternion rotation;
    float opacity;
    QVector3D color;  // SH DC component (base color)
    Cov3D cov3d;      // Pre-computed 3D covariance matrix

    // Compute 3D covariance matrix from scale and rotation
    // Covariance = R * S * S^T * R^T where S is diagonal scale matrix
    void computeCovariance() {
        // Build rotation matrix from quaternion
        float w = rotation.scalar();
        float x = rotation.x();
        float y = rotation.y();
        float z = rotation.z();

        // Rotation matrix (column-major for easier reading)
        // R = [r00 r01 r02]
        //     [r10 r11 r12]
        //     [r20 r21 r22]
        float r00 = 1.f - 2.f * (y * y + z * z);
        float r01 = 2.f * (x * y - w * z);
        float r02 = 2.f * (x * z + w * y);
        float r10 = 2.f * (x * y + w * z);
        float r11 = 1.f - 2.f * (x * x + z * z);
        float r12 = 2.f * (y * z - w * x);
        float r20 = 2.f * (x * z - w * y);
        float r21 = 2.f * (y * z + w * x);
        float r22 = 1.f - 2.f * (x * x + y * y);

        // M = R * S (scale applied to columns of R)
        float sx = scale.x();
        float sy = scale.y();
        float sz = scale.z();

        float m00 = r00 * sx, m01 = r01 * sy, m02 = r02 * sz;
        float m10 = r10 * sx, m11 = r11 * sy, m12 = r12 * sz;
        float m20 = r20 * sx, m21 = r21 * sy, m22 = r22 * sz;

        // Covariance = M * M^T
        // Since covariance is symmetric, we only need 6 values
        cov3d.data[0] = m00*m00 + m01*m01 + m02*m02;  // xx
        cov3d.data[1] = m00*m10 + m01*m11 + m02*m12;  // xy
        cov3d.data[2] = m00*m20 + m01*m21 + m02*m22;  // xz
        cov3d.data[3] = m10*m10 + m11*m11 + m12*m12;  // yy
        cov3d.data[4] = m10*m20 + m11*m21 + m12*m22;  // yz
        cov3d.data[5] = m20*m20 + m21*m21 + m22*m22;  // zz
    }
};

class GaussianSplatLoader {
public:
    static QVector<GaussianSplat> loadPLY(const QString& filename) {
        QVector<GaussianSplat> splats;
        QFile file(filename);

        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open PLY file:" << filename;
            return splats;
        }

        QTextStream stream(&file);
        QString line;
        int vertexCount = 0;
        bool isBinary = false;
        bool isBigEndian = false;

        // Property indices
        int idx_x = -1, idx_y = -1, idx_z = -1;
        int idx_scale0 = -1, idx_scale1 = -1, idx_scale2 = -1;
        int idx_rot0 = -1, idx_rot1 = -1, idx_rot2 = -1, idx_rot3 = -1;
        int idx_opacity = -1;
        int idx_f_dc_0 = -1, idx_f_dc_1 = -1, idx_f_dc_2 = -1;

        QStringList propertyNames;
        QStringList propertyTypes;

        // Parse header
        while (!stream.atEnd()) {
            line = stream.readLine().trimmed();

            if (line.startsWith("format")) {
                if (line.contains("binary_little_endian")) {
                    isBinary = true;
                    isBigEndian = false;
                } else if (line.contains("binary_big_endian")) {
                    isBinary = true;
                    isBigEndian = true;
                }
            } else if (line.startsWith("element vertex")) {
                vertexCount = line.split(' ').last().toInt();
            } else if (line.startsWith("property")) {
                QStringList parts = line.split(' ');
                if (parts.size() >= 3) {
                    propertyTypes.append(parts[1]);
                    propertyNames.append(parts[2]);
                }
            } else if (line == "end_header") {
                break;
            }
        }

        // Map property names to indices
        for (int i = 0; i < propertyNames.size(); ++i) {
            const QString& name = propertyNames[i];
            if (name == "x") idx_x = i;
            else if (name == "y") idx_y = i;
            else if (name == "z") idx_z = i;
            else if (name == "scale_0") idx_scale0 = i;
            else if (name == "scale_1") idx_scale1 = i;
            else if (name == "scale_2") idx_scale2 = i;
            else if (name == "rot_0") idx_rot0 = i;
            else if (name == "rot_1") idx_rot1 = i;
            else if (name == "rot_2") idx_rot2 = i;
            else if (name == "rot_3") idx_rot3 = i;
            else if (name == "opacity") idx_opacity = i;
            else if (name == "f_dc_0") idx_f_dc_0 = i;
            else if (name == "f_dc_1") idx_f_dc_1 = i;
            else if (name == "f_dc_2") idx_f_dc_2 = i;
        }

        qDebug() << "Loading" << vertexCount << "splats from" << filename;
        qDebug() << "Binary:" << isBinary << "Properties:" << propertyNames.size();

        splats.reserve(vertexCount);

        if (isBinary) {
            // Close text stream and reopen for binary reading
            file.close();
            file.open(QIODevice::ReadOnly);

            // Skip header
            while (true) {
                QByteArray headerLine = file.readLine();
                if (headerLine.trimmed() == "end_header") {
                    break;
                }
            }

            // Read binary data
            QDataStream dataStream(&file);
            if (isBigEndian) {
                dataStream.setByteOrder(QDataStream::BigEndian);
            } else {
                dataStream.setByteOrder(QDataStream::LittleEndian);
            }
            dataStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

            for (int v = 0; v < vertexCount; ++v) {
                QVector<float> values(propertyNames.size());

                for (int p = 0; p < propertyNames.size(); ++p) {
                    if (propertyTypes[p] == "float") {
                        float val;
                        dataStream >> val;
                        values[p] = val;
                    } else if (propertyTypes[p] == "double") {
                        double val;
                        dataStream >> val;
                        values[p] = static_cast<float>(val);
                    } else if (propertyTypes[p] == "uchar" || propertyTypes[p] == "uint8") {
                        quint8 val;
                        dataStream >> val;
                        values[p] = val / 255.0f;
                    } else if (propertyTypes[p] == "int") {
                        qint32 val;
                        dataStream >> val;
                        values[p] = static_cast<float>(val);
                    }
                }

                GaussianSplat splat;

                if (idx_x >= 0) splat.position.setX(values[idx_x]);
                if (idx_y >= 0) splat.position.setY(values[idx_y]);
                if (idx_z >= 0) splat.position.setZ(values[idx_z]);

                // Scale is stored as log(scale) in Gaussian splatting PLY files
                if (idx_scale0 >= 0) splat.scale.setX(std::exp(values[idx_scale0]));
                if (idx_scale1 >= 0) splat.scale.setY(std::exp(values[idx_scale1]));
                if (idx_scale2 >= 0) splat.scale.setZ(std::exp(values[idx_scale2]));

                // Rotation quaternion (w, x, y, z format in PLY, but need to normalize)
                if (idx_rot0 >= 0 && idx_rot1 >= 0 && idx_rot2 >= 0 && idx_rot3 >= 0) {
                    float w = values[idx_rot0];
                    float x = values[idx_rot1];
                    float y = values[idx_rot2];
                    float z = values[idx_rot3];
                    float len = std::sqrt(w*w + x*x + y*y + z*z);
                    if (len > 0.0001f) {
                        splat.rotation = QQuaternion(w/len, x/len, y/len, z/len);
                    } else {
                        splat.rotation = QQuaternion();
                    }
                }

                // Opacity is stored as sigmoid inverse
                if (idx_opacity >= 0) {
                    float rawOpacity = values[idx_opacity];
                    splat.opacity = 1.0f / (1.0f + std::exp(-rawOpacity));
                } else {
                    splat.opacity = 1.0f;
                }

                // SH DC coefficients (convert from SH0 to RGB)
                const float SH_C0 = 0.28209479177387814f;
                if (idx_f_dc_0 >= 0) splat.color.setX(0.5f + SH_C0 * values[idx_f_dc_0]);
                if (idx_f_dc_1 >= 0) splat.color.setY(0.5f + SH_C0 * values[idx_f_dc_1]);
                if (idx_f_dc_2 >= 0) splat.color.setZ(0.5f + SH_C0 * values[idx_f_dc_2]);

                // Clamp color to valid range
                splat.color.setX(std::clamp(splat.color.x(), 0.0f, 1.0f));
                splat.color.setY(std::clamp(splat.color.y(), 0.0f, 1.0f));
                splat.color.setZ(std::clamp(splat.color.z(), 0.0f, 1.0f));

                // Pre-compute 3D covariance matrix
                splat.computeCovariance();

                splats.append(splat);
            }
        } else {
            // ASCII format
            for (int v = 0; v < vertexCount && !stream.atEnd(); ++v) {
                line = stream.readLine().trimmed();
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);

                if (parts.size() < propertyNames.size()) continue;

                QVector<float> values(propertyNames.size());
                for (int p = 0; p < propertyNames.size(); ++p) {
                    values[p] = parts[p].toFloat();
                }

                GaussianSplat splat;

                if (idx_x >= 0) splat.position.setX(values[idx_x]);
                if (idx_y >= 0) splat.position.setY(values[idx_y]);
                if (idx_z >= 0) splat.position.setZ(values[idx_z]);

                if (idx_scale0 >= 0) splat.scale.setX(std::exp(values[idx_scale0]));
                if (idx_scale1 >= 0) splat.scale.setY(std::exp(values[idx_scale1]));
                if (idx_scale2 >= 0) splat.scale.setZ(std::exp(values[idx_scale2]));

                if (idx_rot0 >= 0 && idx_rot1 >= 0 && idx_rot2 >= 0 && idx_rot3 >= 0) {
                    float w = values[idx_rot0];
                    float x = values[idx_rot1];
                    float y = values[idx_rot2];
                    float z = values[idx_rot3];
                    float len = std::sqrt(w*w + x*x + y*y + z*z);
                    if (len > 0.0001f) {
                        splat.rotation = QQuaternion(w/len, x/len, y/len, z/len);
                    }
                }

                if (idx_opacity >= 0) {
                    float rawOpacity = values[idx_opacity];
                    splat.opacity = 1.0f / (1.0f + std::exp(-rawOpacity));
                } else {
                    splat.opacity = 1.0f;
                }

                const float SH_C0 = 0.28209479177387814f;
                if (idx_f_dc_0 >= 0) splat.color.setX(0.5f + SH_C0 * values[idx_f_dc_0]);
                if (idx_f_dc_1 >= 0) splat.color.setY(0.5f + SH_C0 * values[idx_f_dc_1]);
                if (idx_f_dc_2 >= 0) splat.color.setZ(0.5f + SH_C0 * values[idx_f_dc_2]);

                splat.color.setX(std::clamp(splat.color.x(), 0.0f, 1.0f));
                splat.color.setY(std::clamp(splat.color.y(), 0.0f, 1.0f));
                splat.color.setZ(std::clamp(splat.color.z(), 0.0f, 1.0f));

                // Pre-compute 3D covariance matrix
                splat.computeCovariance();

                splats.append(splat);
            }
        }

        file.close();
        qDebug() << "Loaded" << splats.size() << "splats";
        return splats;
    }

    // Sort splats by depth (back to front) for proper alpha blending
    static void sortByDepth(QVector<GaussianSplat>& splats, const QVector3D& cameraPosition) {
        std::sort(splats.begin(), splats.end(),
            [&cameraPosition](const GaussianSplat& a, const GaussianSplat& b) {
                float distA = (a.position - cameraPosition).lengthSquared();
                float distB = (b.position - cameraPosition).lengthSquared();
                return distA > distB;  // Back to front
            });
    }

    // Compute bounding box of splats
    static void computeBounds(const QVector<GaussianSplat>& splats,
                              QVector3D& minBound, QVector3D& maxBound, QVector3D& center) {
        if (splats.isEmpty()) {
            minBound = maxBound = center = QVector3D(0, 0, 0);
            return;
        }

        minBound = QVector3D(1e10f, 1e10f, 1e10f);
        maxBound = QVector3D(-1e10f, -1e10f, -1e10f);

        for (const auto& splat : splats) {
            minBound.setX(std::min(minBound.x(), splat.position.x()));
            minBound.setY(std::min(minBound.y(), splat.position.y()));
            minBound.setZ(std::min(minBound.z(), splat.position.z()));
            maxBound.setX(std::max(maxBound.x(), splat.position.x()));
            maxBound.setY(std::max(maxBound.y(), splat.position.y()));
            maxBound.setZ(std::max(maxBound.z(), splat.position.z()));
        }

        center = (minBound + maxBound) * 0.5f;
    }
};

#endif // GAUSSIANSPLAT_H

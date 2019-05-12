#include "houseobject.h"

using namespace Eigen;

HouseObject::HouseObject(GL_ViewRenderer * view,
                         const Vector3i & n_size,
                         const Vector3f & size,
                         const Eigen::Vector3f & borderFirst,
                         const Eigen::Vector3f & borderSecond):
    m_n_size(n_size),
    m_size(size),
    m_borderFirst(borderFirst),
    m_borderSecond(borderSecond)
{
    _createMesh();
    m_material = view->createMaterial(MaterialType::Morph);
    m_screenTempObject = GL_ScreenObjectPtr::create(GL_MeshPtr(), GL_ShaderMaterialPtr());
}

void HouseObject::draw(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix,
                       GLuint frameTextureId, const QSize & frameTextureSize)
{
    m_screenTempObject->setFillMode(view->parent()->fillFrameMode());
    m_screenTempObject->setOrigin(Vector2f(0.0f, 0.0f));
    m_screenTempObject->setSize(Vector2f(frameTextureSize.width(), frameTextureSize.height()));

    QMatrix4x4 matrixMVP = view->projectionMatrix() * viewMatrix;
    QMatrix4x4 scaleFrameTransform;
    scaleFrameTransform.scale(1.0f / frameTextureSize.width(), 1.0f / frameTextureSize.height());
    QMatrix4x4 invUvTransfrom = m_screenTempObject->getMatrixMVP(view->viewportSize()).inverted();
    m_material->setValue("matrixMVP", matrixMVP);
    m_material->setValue("matrixView2FrameUV", invUvTransfrom);
    m_material->setTexture("screen_texture", frameTextureId);

    {
        static float t = 0.0f;
        for (int i = 1; i < m_floorInfos.size(); ++i)
        {
            for (int j = 2; j < 3; ++j)
            {
                const QVector<GLuint> & indices = m_floorInfos[i].i_sides[j];
                for (int k = 2; k < indices.size() - 2; ++k)
                {
                    float kk = k / static_cast<float>(indices.size());
                    m_textureCoords[indices[k]].setY(std::sin(t + kk * 0.0f) * 3.0f);
                }
            }
        }
        t += 0.05f;
        m_mesh->updateTextureCoords(m_textureCoords);
    }

    m_mesh->draw(view, *m_material);
}

void HouseObject::_createMesh()
{
    QVector<QVector3D> vertices;

    QVector3D floorSize(m_size.x() - (m_borderFirst.x() + m_borderSecond.x()),
                        m_size.y() - (m_borderFirst.y() + m_borderSecond.y()),
                        m_size.z() - (m_borderFirst.z() + m_borderSecond.z()));

    auto addFloor = [&, this] (float y) -> _FloorInfo
    {
        _FloorInfo info;
        info.y = y;
        /*{
            QVector<GLuint> & indices = info.i_sides[0];
            GLuint i_o = static_cast<GLuint>(vertices.size());
            vertices.push_back(QVector3D(- m_size.x() * 0.5f, y, m_size.z() * 0.5f));
            indices.push_back(i_o);
            vertices.push_back(QVector3D(m_size.x() * 0.5f, y, m_size.z() * 0.5f));
            indices.push_back(i_o + 1);
        }*/
        {
            QVector<GLuint> & indices = info.i_sides[1];
            GLuint i_o = static_cast<GLuint>(vertices.size());
            vertices.push_back(QVector3D(m_size.x() * 0.5f, y, m_size.z() * 0.5f));
            indices.push_back(i_o);
            vertices.push_back(QVector3D(m_size.x() * 0.5f, y, - m_size.z() * 0.5f));
            indices.push_back(i_o + 1);
        }
        {
            QVector<GLuint> & indices = info.i_sides[2];
            GLuint i_o = static_cast<GLuint>(vertices.size());
            vertices.push_back(QVector3D(m_size.x() * 0.5f, y, - m_size.z() * 0.5f));
            indices.push_back(i_o);
            float t_delta = 1.0f / static_cast<float>(m_n_size.x());
            QVector3D v1((0.5f - t_delta) * floorSize.x(), y, - m_size.z() * 0.5f);
            vertices.push_back(v1);
            indices.push_back(i_o + 1);
            vertices.push_back(v1);
            indices.push_back(i_o + 2);
            QVector3D v2((t_delta - 0.5f) * floorSize.x(), y, - m_size.z() * 0.5f);
            vertices.push_back(v2);
            indices.push_back(i_o + 3);
            vertices.push_back(v2);
            indices.push_back(i_o + 4);
            vertices.push_back(QVector3D(- m_size.x() * 0.5f, y, - m_size.z() * 0.5f));
            indices.push_back(i_o + 5);
        }
        {
            QVector<GLuint> & indices = info.i_sides[3];
            GLuint i_o = static_cast<GLuint>(vertices.size());
            vertices.push_back(QVector3D(- 0.5f * m_size.x(), y, - m_size.z() * 0.5f));
            indices.push_back(i_o);
            vertices.push_back(QVector3D(- m_size.x() * 0.5f, y, m_size.z() * 0.5f));
            indices.push_back(i_o + 1);
        }
        return info;
    };

    QVector<_FloorInfo> floors(m_n_size.y() + 3);
    floors[0] = addFloor(0.0f);
    for (int i = 0; i <= m_n_size.y(); ++i)
    {
        float t = i / static_cast<float>(m_n_size.y());
        floors[i + 1] = addFloor(m_borderFirst.y() + t * floorSize.y());
    }
    floors[m_n_size.y() + 2] = addFloor(m_size.y());

    QVector<GLuint> indices;
    for (int i = 0; i < floors.size() - 1; ++i)
    {
        const _FloorInfo & down = floors[i];
        const _FloorInfo & up = floors[i + 1];
        {
            const QVector<GLuint> & d_ind = down.i_sides[1];
            const QVector<GLuint> & u_ind = up.i_sides[1];
            assert(d_ind.size() == u_ind.size());
            indices.append({ d_ind[1], d_ind[0], u_ind[0],
                             d_ind[1], u_ind[0], u_ind[1] });
        }
        {
            const QVector<GLuint> & d_ind = down.i_sides[2];
            const QVector<GLuint> & u_ind = up.i_sides[2];
            assert(d_ind.size() == u_ind.size());
            for (int k = 0; k < d_ind.size() - 1; ++k)
            {
                if ((k == 1) || (k == (d_ind.size() - 3)))
                    continue;
                indices.append({ d_ind[k + 1], d_ind[k + 0], u_ind[k + 0],
                                 d_ind[k + 1], u_ind[k + 0], u_ind[k + 1] });
                indices.append({ d_ind[k + 1], d_ind[k + 0], u_ind[k + 0],
                                 d_ind[k + 1], u_ind[k + 0], u_ind[k + 1] });
            }
        }
        {
            const QVector<GLuint> & d_ind = down.i_sides[3];
            const QVector<GLuint> & u_ind = up.i_sides[3];
            assert(d_ind.size() == u_ind.size());
            indices.append({ d_ind[1], d_ind[0], u_ind[0],
                             d_ind[1], u_ind[0], u_ind[1] });
        }
    }

    m_floorInfos = floors;

    /*m_floorInfos.resize(m_n_size.y() + 1);
    for (int i = 0; i < m_n_size.y(); ++i)
        m_floorInfos[i] = floors[i + 1];*/

    QVector<QVector2D> textureCoords(vertices.size(), QVector2D(0.0f, 0.0f));

    m_mesh = GL_MeshPtr::create(GL_Mesh::createMesh(vertices, textureCoords, indices));
    m_textureCoords = std::move(textureCoords);
}

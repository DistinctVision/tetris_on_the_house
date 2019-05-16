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
    _createMeshForward();
    _createMeshBackward();
    m_materialForward = view->createMaterial(MaterialType::Morph);
    m_materialBackward = view->createMaterial(MaterialType::Color);
    m_materialBackward->setValue("mainColor", QColor(0, 0, 0, 255));
    m_screenTempObject = GL_ScreenObjectPtr::create(GL_MeshPtr(), GL_ShaderMaterialPtr());
}

Vector3f HouseObject::size() const
{
    return m_size;
}

Vector3i HouseObject::n_size() const
{
    return m_n_size;
}

Vector3f HouseObject::borderFirst() const
{
    return m_borderFirst;
}

Vector3f HouseObject::borderSecond() const
{
    return m_borderSecond;
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
    m_materialForward->setValue("matrixMVP", matrixMVP);
    m_materialForward->setValue("matrixView2FrameUV", invUvTransfrom);
    m_materialForward->setTexture("screen_texture", frameTextureId);

    m_meshForward->draw(view, *m_materialForward);

    m_materialBackward->setValue("matrixMVP", matrixMVP);
    m_meshBackward->draw(view, *m_materialBackward);
}

void HouseObject::_createMeshForward()
{
    float k_floor = 2.7f;

    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QVector<GLuint> indices;

    auto createRect = [&] (const QVector3D & origin, const QVector3D & axisX, const QVector3D & axisY,
                           const QSize & size) -> std::tuple<QVector<QVector3D>, QVector<QVector2D>, QVector<GLuint>>
    {
        QVector<QVector3D> vertices;
        QVector<QVector2D> texCoords;
        QVector<GLuint> indices;

        for (int j = 0; j <= size.height(); ++j)
        {
            float v = j / static_cast<float>(size.height());
            for (int i = 0; i <= size.width(); ++i)
            {
                float u = i / static_cast<float>(size.width());
                vertices.push_back(origin + axisX * u + axisY * v);
                texCoords.push_back(QVector2D(u, v));
            }
        }
        GLuint str_i_offset = static_cast<GLuint>(size.width() + 1);
        for (int i = 0; i < size.width(); ++i)
        {
            for (int j = 0; j < size.height(); ++j)
            {
                GLuint i_o = static_cast<GLuint>(i * str_i_offset + j);
                indices.append({ i_o + 0, i_o + 1, i_o + str_i_offset + 0,
                                 i_o + 1, i_o + str_i_offset + 0, i_o + str_i_offset + 1 });
            }
        }

        return std::make_tuple(vertices, texCoords, indices);
    };

    auto merge = [&] (const std::tuple<QVector<QVector3D>, QVector<QVector2D>, QVector<GLuint>> & tuple)
    {
        const QVector<QVector3D> & c_vertices = std::get<0>(tuple);
        const QVector<QVector2D> & c_texCoords = std::get<1>(tuple);
        const QVector<GLuint> & c_indices = std::get<2>(tuple);

        int i_o = vertices.size();
        indices.resize(indices.size() + c_indices.size());
        for (int i = 0; i < c_indices.size(); ++i)
            indices[i_o + i] = c_indices[i];
        vertices.append(c_vertices);
        texCoords.append(c_texCoords);
    };

    merge(createRect(QVector3D(- 31.0f, 19.0f * k_floor, - 3.0f),
                     QVector3D(0.0f, 0.0f, - 3.0f),
                     QVector3D(0.0f, (8.0f - 19.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(createRect(QVector3D(- 31.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(- 24.0f - 31.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, (8.0f - 19.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(createRect(QVector3D(- 23.0f, 20.0f, - 1.0f),
                     QVector3D(- 23.0f - 20.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, (0.0f - 20.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(createRect(QVector3D(- 19.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(- 12.0f - 19.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, (8.0f - 19.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(createRect(QVector3D(- 12.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 3.0f),
                     QVector3D(0.0f, (8.0f - 19.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
}

void HouseObject::_createMeshBackward()
{
}

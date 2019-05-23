#include "glowedgesscene.h"

#include <cmath>

#include "texturereceiver.h"

using namespace std;

GlowEdgesScene::GlowEdgesScene(int duration, const QVector3D & planeDir):
    AnimationScene(duration),
    m_planeDir(planeDir.normalized())
{
}

void GlowEdgesScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_material = view->createMaterial(MaterialType::ScreenMorph_glowEdgesOnPlane);
    _createBackwardMesh();
}

void GlowEdgesScene::destroy(GL_ViewRenderer * view)
{
    m_backwardMesh.reset();
    m_material.reset();
    AnimationScene::destroy(view);
}

void GlowEdgesScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    view->glDisable(GL_BLEND);
    {
        view->glDepthFunc(GL_LESS);

        QVector3D color_a(1.0f, 1.0f, 1.0f);
        QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
        m_material->setValue("matrixMVP", matrixVP);
        m_material->setValue("matrixView2FrameUV", house->matrixView2FrameUV(view,
                                                                             textureReceiver()->textureSize(),
                                                                             textureReceiver()->orientation()));
        m_material->setTexture("screen_texture", textureReceiver()->textureId());
        m_material->setValue("color_a", color_a);
        m_material->setValue("color_b", color_b);
        m_material->setValue("edges_size", 0.25f);
        m_material->setValue("edges_color", QColor(200, 255, 200, 255));
        m_material->setValue("edges_power", 2.0f);
        m_material->setValue("plane", QVector4D(m_planeDir, 60.0f - 120.0f * time));
        house->meshHouse()->draw(view, *m_material);


        view->glDepthFunc(GL_GREATER);
        m_material->setValue("edges_color", QColor(100, 255, 220, 255));
        m_material->setValue("edges_power", 1.0f);
        house->meshHouse()->draw(view, *m_material);
        m_backwardMesh->draw(view, *m_material);
    }

    view->glDepthFunc(GL_LESS);
    view->glEnable(GL_BLEND);
    GL_ShaderMaterialPtr materialGrid = house->materialGrid();
    materialGrid->setValue("matrixMVP", matrixVP);
    materialGrid->setValue("mainColor", QColor(255, 255, 255, 100));
    house->meshGrid()->draw(view, *materialGrid);
}

void GlowEdgesScene::_createBackwardMesh()
{
    float k_floor = 2.7f;

    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QVector<GLuint> indices;

    auto createRect = [&] (const QVector3D & v1, const QVector3D & v2, const QVector3D & v3, const QVector3D & v4) ->
            std::tuple<QVector<QVector3D>, QVector<QVector2D>, QVector<GLuint>>
    {
        QVector<QVector3D> vertices = { v1, v2, v3, v4 };
        QVector<QVector2D> texCoords = { QVector2D(0.0f, 0.0f), QVector2D(1.0f, 0.0f),
                                         QVector2D(1.0f, 1.0f), QVector2D(0.0f, 1.0f) };
        QVector<GLuint> indices = { 0, 1, 2, 0, 2, 3, 0, 2, 1, 0, 3, 2 };

        return std::make_tuple(vertices, texCoords, indices);
    };

    auto merge = [&] (const std::tuple<QVector<QVector3D>, QVector<QVector2D>, QVector<GLuint>> & tuple) ->
            std::pair<GLuint, GLuint>
    {
        const QVector<QVector3D> & c_vertices = std::get<0>(tuple);
        const QVector<QVector2D> & c_texCoords = std::get<1>(tuple);
        const QVector<GLuint> & c_indices = std::get<2>(tuple);

        GLuint i_o = static_cast<GLuint>(vertices.size());
        int offset = indices.size();
        indices.resize(indices.size() + c_indices.size());
        for (int i = 0; i < c_indices.size(); ++i)
            indices[offset + i] = i_o + c_indices[i];
        vertices.append(c_vertices);
        texCoords.append(c_texCoords);

        return std::make_pair(i_o, static_cast<GLuint>(vertices.size()));
    };

    merge(createRect(QVector3D(-31.0f, 19.0f * k_floor, 4.0f),
                     QVector3D(-31.0f, 8.0f * k_floor, 4.0f),
                     QVector3D(-31.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(-31.0f, 19.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(-12.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(-12.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(-12.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(-12.0f, 19.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(-31.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(-31.0f, 19.0f * k_floor, 30.0f),
                     QVector3D(-12.0f, 19.0f * k_floor, 30.0f),
                     QVector3D(-12.0f, 19.0f * k_floor, 0.0f)));
    merge(createRect(QVector3D(-31.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(-31.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(-12.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(-12.0f, 8.0f * k_floor, 0.0f)));

    merge(createRect(QVector3D(-30.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(-30.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(-30.0f, 0.0f * k_floor, 30.0f),
                     QVector3D(-30.0f, 8.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(-14.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(-14.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(-14.0f, 0.0f * k_floor, 30.0f),
                     QVector3D(-14.0f, 8.0f * k_floor, 30.0f)));

    merge(createRect(QVector3D(0.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(0.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(0.0f, 0.0f * k_floor, 30.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 30.0f)));

    merge(createRect(QVector3D(31.0f, 19.0f * k_floor, 4.0f),
                     QVector3D(31.0f, 8.0f * k_floor, 4.0f),
                     QVector3D(31.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(31.0f, 19.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(12.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(12.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(12.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(12.0f, 19.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(31.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(31.0f, 19.0f * k_floor, 30.0f),
                     QVector3D(12.0f, 19.0f * k_floor, 30.0f),
                     QVector3D(12.0f, 19.0f * k_floor, 0.0f)));
    merge(createRect(QVector3D(31.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(31.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(12.0f, 8.0f * k_floor, 30.0f),
                     QVector3D(12.0f, 8.0f * k_floor, 0.0f)));

    merge(createRect(QVector3D(30.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(30.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(30.0f, 0.0f * k_floor, 30.0f),
                     QVector3D(30.0f, 8.0f * k_floor, 30.0f)));
    merge(createRect(QVector3D(14.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(14.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(14.0f, 0.0f * k_floor, 30.0f),
                     QVector3D(14.0f, 8.0f * k_floor, 30.0f)));

    m_backwardMesh = GL_MeshPtr::create(GL_Mesh::createMesh(vertices, texCoords, indices));
}

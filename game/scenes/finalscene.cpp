#include "finalscene.h"

#include <cmath>
#include <QtMath>

#include "texturereceiver.h"

using namespace std;

FinalScene::FinalScene(int duration):
    AnimationScene(duration)
{
}

void FinalScene::init(GL_ViewRenderer * view)
{
    AnimationScene::init(view);
    m_materialHouse = view->createMaterial(MaterialType::ScreenMorph_glowEdges);
    m_materialForDoors = view->createMaterial(MaterialType::ScreenMorph_glowEdges_transform);
    m_materialColor = view->createMaterial(MaterialType::Color);
    m_materialTunnel = view->createMaterial(MaterialType::Tunnel);
    _createTunnelGrid();
    _createBirds(view, 10);
}

void FinalScene::destroy(GL_ViewRenderer * view)
{
    m_birds.clear();
    m_meshTunnelGrid.reset();
    m_materialHouse.reset();
    m_materialForDoors.reset();
    m_materialColor.reset();
    m_materialTunnel.reset();
    AnimationScene::destroy(view);
}

void FinalScene::draw(GL_ViewRenderer * view)
{
    float time = this->timeState();

    HouseObjectPtr house = this->house();
    QMatrix4x4 matrixVP = view->projectionMatrix() * viewMatrix();

    m_materialHouse->setValue("edges_size", min(min(time / 0.1f, 1.0f),
                                                min((1.0f - time) / 0.1f, 1.0f)) * 0.125f);

    QVector3D color_a(1.0f, min((1.0f - time) / 0.1f, 1.0f), 1.0f);
    QVector3D color_b(0.0f, 0.0f, house->activityLevel() * 0.25f);
    view->glDisable(GL_BLEND);
    {
        m_materialHouse->setValue("matrixMVP", matrixVP);
        m_materialHouse->setValue("matrixView2FrameUV",
                                  house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_materialHouse->setTexture("screen_texture", textureReceiver()->textureId());
        m_materialHouse->setValue("color_a", color_a);
        m_materialHouse->setValue("color_b", color_b);
        house->meshHouse_wo_doors()->draw(view, *m_materialHouse);
        if (time < 0.1f)
        {
            house->meshLeftDoor()->draw(view, *m_materialHouse);
            house->meshRightDoor()->draw(view, *m_materialHouse);
        }
    }

    if (time >= 0.1f)
    {
        view->glDepthMask(GL_FALSE);
        m_materialColor->setValue("matrixMVP", matrixVP);
        m_materialColor->setValue("mainColor", QColor(0, 0, 0, 255));
        house->meshLeftDoor()->draw(view, *m_materialColor);
        house->meshRightDoor()->draw(view, *m_materialColor);
        view->glDepthMask(GL_TRUE);
    }

    m_materialTunnel->setValue("matrixMVP", matrixVP);
    m_materialTunnel->setValue("colorA", QColor(255, 255, 255, 255));
    m_materialTunnel->setValue("colorB", QColor(0, 0, 0, 255));
    m_materialTunnel->setValue("plane_z", 190.0f * (1.0f - time) - 30.0f);
    m_materialTunnel->setValue("plane_delta", 25.0f);
    m_materialTunnel->setValue("begin_z", 19.0f);
    m_meshTunnelGrid->draw(view, *m_materialTunnel);

    float time_for_doors = (time - 0.1f) / 0.2f;
    if ((time_for_doors < 0.0f) || (time_for_doors > 1.0f))
    {
        time_for_doors = (1.0f - time) / 0.1f;
    }
    if ((time_for_doors >= 0.0f) && (time_for_doors <= 1.0f))
    {
        const float delta_x = 14.0f;
        const float delta_z = 14.5f;

        QMatrix4x4 worldMatrixForLeftDoor;
        QMatrix4x4 worldMatrixForRightDoor;

        if (time_for_doors < 0.5f)
        {
            float t = (time_for_doors * 2.0f);
            t *= t;
            worldMatrixForLeftDoor(2, 3) = delta_z * t;
            worldMatrixForRightDoor(2, 3) = delta_z * t;
        }
        else
        {
            float t = ((time_for_doors - 0.5f) * 2.0f);
            t *= t;
            worldMatrixForLeftDoor(0, 3) = - t * delta_x;
            worldMatrixForLeftDoor(2, 3) = delta_z;
            worldMatrixForRightDoor(0, 3) = t * delta_x;
            worldMatrixForRightDoor(2, 3) = delta_z;
        }
        m_materialForDoors->setValue("edges_size", 0.125f);
        m_materialForDoors->setValue("matrixView2FrameUV",
                                     house->matrixView2FrameUV(view, textureReceiver()->textureSize()));
        m_materialForDoors->setTexture("screen_texture", textureReceiver()->textureId());
        m_materialForDoors->setValue("color_a", color_a);
        m_materialForDoors->setValue("color_b", color_b);
        m_materialForDoors->setValue("matrixMVP", matrixVP);
        m_materialForDoors->setValue("matrixMVP_transform", matrixVP * worldMatrixForLeftDoor);
        house->meshLeftDoor()->draw(view, *m_materialForDoors);
        m_materialForDoors->setValue("matrixMVP_transform", matrixVP * worldMatrixForRightDoor);
        house->meshRightDoor()->draw(view, *m_materialForDoors);
    }

    if (time < 0.1f)
    {
        view->glEnable(GL_BLEND);
        float time_grid = min(max(time / 0.1f, 0.0f), 1.0f);
        GL_ShaderMaterialPtr materialGrid = house->materialGrid();
        QMatrix4x4 worldGridMatrix;
        worldGridMatrix(2, 2) = 1.0f - time_grid;
        worldGridMatrix(2, 3) = house->grid_end().z() * time_grid;
        materialGrid->setValue("matrixMVP", matrixVP * worldGridMatrix);
        materialGrid->setValue("mainColor", QColor(255, 255, 255, static_cast<int>(100 * (1.0f - time_grid))));
        house->meshGrid()->draw(view, *materialGrid);
    }

    view->glDisable(GL_DEPTH_TEST);
    view->glDisable(GL_CULL_FACE);
    _drawBirds(view, viewMatrix());
    view->glEnable(GL_CULL_FACE);
    view->glEnable(GL_DEPTH_TEST);
}

void FinalScene::_createTunnelGrid()
{
    const float k_floor = 2.7f;

    QVector3D origin(-12.0f, 8.0f * k_floor, 4.0f);
    QVector3D size(24.0f, 8.0f * k_floor, 15.0f);
    float gridStep = 15.0f;
    int numberSteps = 15;
    QSize gridSize(7, 7);

    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QVector<GLuint> indices;

    {
        QVector2D d(size.x() * 0.025f, size.y() * 0.025f);
        for (int i = 1; i <= numberSteps; ++i)
        {
            GLuint i_o = static_cast<GLuint>(vertices.size());
            float z = i * gridStep + origin.z();
            vertices.push_back(QVector3D(origin.x(), origin.y(), z));
            vertices.push_back(QVector3D(origin.x() + size.x(), origin.y(), z));
            vertices.push_back(QVector3D(origin.x() + size.x(), origin.y() + size.y(), z));
            vertices.push_back(QVector3D(origin.x(), origin.y() + size.y(), z));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            vertices.push_back(QVector3D(origin.x() + d.x(), origin.y() + d.y(), z));
            vertices.push_back(QVector3D(origin.x() + size.x() - d.x(), origin.y() + d.y(), z));
            vertices.push_back(QVector3D(origin.x() + size.x() - d.x(), origin.y() + size.y() - d.y(), z));
            vertices.push_back(QVector3D(origin.x() + d.x(), origin.y() + size.y() - d.y(), z));
            texCoords.push_back(QVector2D(d.x(), d.y()));
            texCoords.push_back(QVector2D(- d.x(), d.y()));
            texCoords.push_back(QVector2D(- d.x(), - d.y()));
            texCoords.push_back(QVector2D(d.x(), - d.y()));

            indices.append({ i_o + 0, i_o + 1, i_o + 4, i_o + 1, i_o + 5, i_o + 4 });
            indices.append({ i_o + 1, i_o + 2, i_o + 5, i_o + 2, i_o + 6, i_o + 5 });
            indices.append({ i_o + 2, i_o + 3, i_o + 6, i_o + 3, i_o + 7, i_o + 6 });
            indices.append({ i_o + 3, i_o + 0, i_o + 7, i_o + 0, i_o + 4, i_o + 7 });
        }
    }
    {
        float w = size.x() / static_cast<float>(gridSize.width());
        float z_end = origin.z() + size.z();
        float y_end = origin.y() + size.y();
        for (int i = 0; i <= gridSize.width(); ++i)
        {
            GLuint i_o = static_cast<GLuint>(vertices.size());
            //float d = ((i == 0) || (i == gridSize.width())) ? size.z() * 0.5f : size.z();
            float d = 0.5f;
            vertices.push_back(QVector3D(origin.x() + w * i, origin.y(), origin.z()));
            vertices.push_back(QVector3D(origin.x() + w * i + d, origin.y(), origin.z()));
            vertices.push_back(QVector3D(origin.x() + w * i, origin.y(), z_end));
            vertices.push_back(QVector3D(origin.x() + w * i + d, origin.y(), z_end));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            vertices.push_back(QVector3D(origin.x() + w * i, y_end, origin.z()));
            vertices.push_back(QVector3D(origin.x() + w * i + d, y_end, origin.z()));
            vertices.push_back(QVector3D(origin.x() + w * i, y_end, z_end));
            vertices.push_back(QVector3D(origin.x() + w * i + d, y_end, z_end));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            texCoords.push_back(QVector2D(0.0f, 0.0f));
            indices.append({ i_o + 0, i_o + 2, i_o + 1, i_o + 1, i_o + 2, i_o + 3 });
            indices.append({ i_o + 4, i_o + 6, i_o + 5, i_o + 5, i_o + 6, i_o + 7 });
        }
    }
    /*{
        float h = size.x() / static_cast<float>(gridSize.height());
        float z_end = origin.z() + size.z();
        float x_end = origin.x() + size.x();
        for (int i = 0; i <= gridSize.height(); ++i)
        {
            GLuint i_o = static_cast<GLuint>(vertices.size());
            //float d = ((i == 0) || (i == gridSize.height())) ? size.z() * 0.5f : size.z();
            float d = 0.5f;
            vertices.push_back(QVector3D(origin.x(), origin.y() + h * i, origin.z()));
            vertices.push_back(QVector3D(origin.x(), origin.y() + h * i + d, origin.z()));
            vertices.push_back(QVector3D(origin.x(), origin.y() + h * i, z_end));
            vertices.push_back(QVector3D(origin.x(), origin.y() + h * i + d, z_end));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            vertices.push_back(QVector3D(x_end, origin.y() + h * i, origin.z()));
            vertices.push_back(QVector3D(x_end, origin.y() + h * i + d, origin.z()));
            vertices.push_back(QVector3D(x_end, origin.y() + h * i, z_end));
            vertices.push_back(QVector3D(x_end, origin.y() + h * i + d, z_end));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            texCoords.push_back(QVector2D(1.0f, 1.0f));
            indices.append({ i_o + 0, i_o + 1, i_o + 2, i_o + 1, i_o + 3, i_o + 2 });
            indices.append({ i_o + 4, i_o + 5, i_o + 6, i_o + 5, i_o + 7, i_o + 6 });
        }
    }*/
    m_meshTunnelGrid = GL_MeshPtr::create(GL_Mesh::createMesh(vertices, texCoords, indices));
}

void FinalScene::_createBirds(GL_ViewRenderer * view, int number)
{
    BirdMeshPtr mesh = BirdMeshPtr::create();
    mesh->load(":/models/bird_1.obj",
               ":/models/bird_2.obj",
               ":/models/bird_3.obj");
    m_birds.push_back(BirdObjectPtr::create(view, mesh, QVector3D(0.0f, 2.7f * 14.0f, 0.0f),
                                            1.0f, QVector3D(0.0f, 2.7f * 14.0f,  -30.0f)));
}

void FinalScene::_drawBirds(GL_ViewRenderer * view, const QMatrix4x4 & viewMatrix)
{
    for (const BirdObjectPtr & bird : m_birds)
    {
        bird->updateStep();
        bird->draw(view, viewMatrix);
    }
}

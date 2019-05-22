#include "houseobject.h"

#include "tetrisgame.h"

using namespace Eigen;

HouseObject::HouseObject(GL_ViewRenderer * view,
                         const Vector3i & grid_n_size,
                         const Vector3f & grid_begin,
                         const Vector3f & grid_end):
    m_grid_n_size(grid_n_size),
    m_grid_begin(grid_begin),
    m_grid_end(grid_end),
    m_activityLevel(0.0f)
{
    _createMeshHouse();
    _createMeshGrid(0.1f);
    m_screenTempObject = GL_ScreenObjectPtr::create(GL_MeshPtr(), GL_ShaderMaterialPtr());
    m_materialGrid = view->createMaterial(MaterialType::Color);
    m_materialGrid->setValue("mainColor", QColor(255, 255, 255, 100));
    m_meshBlock = GL_MeshPtr::create(GL_Mesh::createQuad(QVector2D(1.0f, 1.0f),
                                                         QVector2D(0.0f, 0.0f), true));
    m_materialBlock = view->createMaterial(MaterialType::ContourFallOff);
    m_materialTables = view->createMaterial(MaterialType::Table);
}

Vector3i HouseObject::grid_n_size() const
{
    return m_grid_n_size;
}

Vector3f HouseObject::grid_begin() const
{
    return m_grid_begin;
}

Vector3f HouseObject::grid_end() const
{
    return m_grid_end;
}

float HouseObject::activityLevel() const
{
    return m_activityLevel;
}

void HouseObject::setActivityLevel(float activityLevel)
{
    m_activityLevel = activityLevel;
}

GL_MeshPtr HouseObject::meshGrid() const
{
    return m_meshGrid;
}

GL_ShaderMaterialPtr HouseObject::materialGrid() const
{
    return m_materialGrid;
}

GL_MeshPtr HouseObject::meshHouse() const
{
    return m_meshHouse;
}

GL_MeshPtr HouseObject::meshHouse_wo_doors() const
{
    return m_meshHouse_wo_doors;
}

GL_MeshPtr HouseObject::meshLeftDoor() const
{
    return m_meshLeftDoor;
}

GL_MeshPtr HouseObject::meshRightDoor() const
{
    return m_meshRightDoor;
}

GL_MeshPtr HouseObject::meshBlock() const
{
    return m_meshBlock;
}

GL_ShaderMaterialPtr HouseObject::materialBlock() const
{
    return m_materialBlock;
}

GL_MeshPtr HouseObject::meshTables() const
{
    return m_meshTables;
}

GL_ShaderMaterialPtr HouseObject::materialTables() const
{
    return m_materialTables;
}

QMatrix4x4 HouseObject::matrixView2FrameUV(GL_ViewRenderer * view, const QSize & frameTextureSize) const
{
    m_screenTempObject->setFillMode(view->parent()->fillFrameMode());
    m_screenTempObject->setOrigin(Vector2f(0.0f, 0.0f));
    m_screenTempObject->setSize(Vector2f(frameTextureSize.width(), frameTextureSize.height()));

    //QMatrix4x4 scaleFrameTransform;
    //scaleFrameTransform.scale(1.0f / frameTextureSize.width(), 1.0f / frameTextureSize.height());
    QMatrix4x4 invUvTransfrom = m_screenTempObject->getMatrixMVP(view->viewportSize()).inverted();
    return invUvTransfrom;
}

void HouseObject::drawBlocks(GL_ViewRenderer * view, const TetrisGame * game, const QMatrix4x4 & viewMatrix,
                             float blockAlpha, float blockEdgeAlpha, bool drawFiguresFlag)
{
    using Figure = TetrisGame::Figure;

    const float offset = 0.05f;

    m_materialBlock->setValue("border_size", blockEdgeAlpha * 0.1f);

    QColor colorBlock = m_materialBlock->value("mainColor").value<QColor>();
    colorBlock.setAlphaF(static_cast<qreal>(blockAlpha));
    m_materialBlock->setValue("mainColor", colorBlock);
    Vector3f fieldSize = m_grid_end - m_grid_begin;
    Vector2f blockSize(fieldSize.x() / static_cast<float>(m_grid_n_size.x()),
                       fieldSize.y() / static_cast<float>(m_grid_n_size.y()));
    QMatrix4x4 projViewMatrix = view->projectionMatrix() * viewMatrix;
    QMatrix4x4 worldMatrix;
    worldMatrix(0, 0) = worldMatrix(1, 1) = std::min(blockSize.x(), blockSize.y()) * 0.9f;
    worldMatrix(2, 3) = m_grid_begin.z() - offset;
    game->for_each_blocks([&, this] (const Vector2i & p) {
        worldMatrix(0, 3) = p.x() * blockSize.x() + m_grid_begin.x();
        worldMatrix(1, 3) = p.y() * blockSize.y() + m_grid_begin.y();
        m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
        m_meshBlock->draw(view, *m_materialBlock);
    });

    if (!drawFiguresFlag)
        return;

    if (game->currentFigureState() > 0.0f)
    {
        float x = (game->figurePos().x() - TetrisGame::figureAnchor.x()) * blockSize.x() + m_grid_begin.x();
        float y = (game->figurePos().y() - TetrisGame::figureAnchor.y()) * blockSize.y() + m_grid_begin.y();
        if (game->currentFigureState() < 1.0f)
        {
            colorBlock.setAlphaF(static_cast<qreal>(game->currentFigureState() * blockAlpha));
            float t = 1.0f - game->currentFigureState();
            t *= t;
            t *= t;
            y += t * 30.0f;
        }
        else
        {
            colorBlock.setAlphaF(static_cast<qreal>(blockAlpha));
        }
        m_materialBlock->setValue("mainColor", colorBlock);
        TetrisGame::Figure figure = game->currentFigure();
        for (int i = 0; i < Figure::RowsAtCompileTime; ++i)
        {
            worldMatrix(1, 3) = y + blockSize.y() * i;
            for (int j = 0; j < Figure::ColsAtCompileTime; ++j)
            {
                if (figure(i, j) > 0)
                {
                    worldMatrix(0, 3) = x + blockSize.x() * j;
                    m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
                    m_meshBlock->draw(view, *m_materialBlock);
                }
            }
        }
    }

    {
        colorBlock.setAlpha(0);
        m_materialBlock->setValue("mainColor", colorBlock);
        Vector2i offset(- (2 + Figure::ColsAtCompileTime), 2 + m_grid_n_size.y());

        Figure nextFigure = game->nextFigure();
        for (int i = 0; i < Figure::RowsAtCompileTime; ++i)
        {
            worldMatrix(1, 3) = (i + offset.y()) * blockSize.y() + m_grid_begin.y();
            for (int j = 0; j < Figure::ColsAtCompileTime; ++j)
            {
                if (nextFigure(i, j) <= 0)
                    continue;
                worldMatrix(0, 3) = (j + offset.x()) * blockSize.x() + m_grid_begin.x();
                m_materialBlock->setValue("matrixMVP", projViewMatrix * worldMatrix);
                m_meshBlock->draw(view, *m_materialBlock);
            }
        }
    }
}

void HouseObject::_createMeshHouse()
{
    using MeshData = std::tuple<QVector<QVector3D>, QVector<QVector2D>, QVector<GLuint>>;

    const float k_floor = 2.7f;

    MeshData house;

    auto createRect = [&] (const QVector3D & origin, const QVector3D & axisX, const QVector3D & axisY,
                           const QSize & size) -> MeshData
    {
        QVector<QVector3D> vertices;
        QVector<QVector2D> texCoords;
        QVector<GLuint> indices;

        vertices.reserve((size.width() + 1) * (size.height() + 1));
        texCoords.reserve(vertices.capacity());

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
        for (int j = 0; j < size.height(); ++j)
        {
            for (int i = 0; i < size.width(); ++i)
            {
                GLuint i_o = static_cast<GLuint>(j * str_i_offset + i);
                indices.append({ i_o + 0, i_o + 1, i_o + str_i_offset + 0,
                                 i_o + 1, i_o + str_i_offset + 1, i_o + str_i_offset + 0 });
            }
        }

        return std::make_tuple(vertices, texCoords, indices);
    };

    auto merge = [&] (MeshData & source, const MeshData & target) -> std::pair<GLuint, GLuint>
    {
        QVector<QVector3D> & vertices = std::get<0>(source);
        QVector<QVector2D> & texCoords = std::get<1>(source);
        QVector<GLuint> & indices = std::get<2>(source);

        const QVector<QVector3D> & c_vertices = std::get<0>(target);
        const QVector<QVector2D> & c_texCoords = std::get<1>(target);
        const QVector<GLuint> & c_indices = std::get<2>(target);

        GLuint i_o = static_cast<GLuint>(vertices.size());
        int offset = indices.size();
        indices.resize(indices.size() + c_indices.size());
        for (int i = 0; i < c_indices.size(); ++i)
            indices[offset + i] = i_o + c_indices[i];
        vertices.append(c_vertices);
        texCoords.append(c_texCoords);

        return std::make_pair(i_o, static_cast<GLuint>(vertices.size()));
    };

    //left upper and vertical part
    merge(house,
          createRect(QVector3D(-31.0f, 8.0f * k_floor, 4.0f),
                     QVector3D(0.0f, 0.0f, - 4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(house,
          createRect(QVector3D(- 31.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(- 23.5f - (- 31.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(- 23.5f, 8.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(-31.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(- 23.5f - (- 31.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, 0.0f, -2.0f),
                     QSize(1, 1)));

    merge(house,
          createRect(QVector3D(- 23.5f, 0.0f * k_floor, 2.0f),
                     QVector3D(- 20.0f - (- 23.5f), 0.0f, 0.0f),
                     QVector3D(0.0f, (20.25f) * k_floor, 0.0f),
                     QSize(2, 1)));

    merge(house,
          createRect(QVector3D(- 20.0f, 8.0f * k_floor, 4.0f),
                     QVector3D(0.0f, 0.0f, -4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(- 20.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(- 12.0f - (- 20.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(- 12.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(house,
          createRect(QVector3D(-19.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(- 12.0f - (- 19.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, 0.0f, -2.0f),
                     QSize(1, 1)));


    //right upper and vertical part
    merge(house,
          createRect(QVector3D(12.0f, 19.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 4.0f),
                     QVector3D(0.0f, (8.0f - 19.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(house,
          createRect(QVector3D(12.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(20.0f - 12.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(20.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(12.0f, 8.0f * k_floor, 2.0f),
                     QVector3D(20.0f - 12.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, 0.0f, -2.0f),
                     QSize(1, 1)));

    merge(house,
          createRect(QVector3D(20.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(23.5f - 20.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, (20.25f) * k_floor, 0.0f),
                     QSize(2, 1)));

    merge(house,
          createRect(QVector3D(23.5f, 8.0f * k_floor, 4.0f),
                     QVector3D(0.0f, 0.0f, -4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(23.5f, 8.0f * k_floor, 0.0f),
                     QVector3D(31.0f - 23.5f, 0.0f, 0.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(4, 11)));
    merge(house,
          createRect(QVector3D(31.0f, 8.0f * k_floor, 0.0f),
                     QVector3D(0.0f, 0.0f, 4.0f),
                     QVector3D(0.0f, (19.0f - 8.0f) * k_floor, 0.0f),
                     QSize(1, 1)));
    merge(house,
          createRect(QVector3D(23.5f, 8.0f * k_floor, 2.0f),
                     QVector3D(31.0f - 23.5f, 0.0f, 0.0f),
                     QVector3D(0.0f, 0.0f, -2.0f),
                     QSize(1, 1)));

    // bottom
    merge(house,
          createRect(QVector3D(-30.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(- 23.5f - (- 30.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 0.0f),
                     QSize(5, 8)));
    merge(house,
          createRect(QVector3D(-20.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(- 14.0f - (- 20.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 0.0f),
                     QSize(5, 8)));
    merge(house,
          createRect(QVector3D(- 14.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(14.0f - (-14.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 0.0f),
                     QSize(4, 8)));
    merge(house,
          createRect(QVector3D(14.0f, 0.0f * k_floor, 2.0f),
                     QVector3D(20.0f - 14.0f, 0.0f, 0.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 0.0f),
                     QSize(5, 8)));
    merge(house,
          createRect(QVector3D(23.5f, 0.0f * k_floor, 2.0f),
                     QVector3D(30.0f - 23.5f, 0.0f, 0.0f),
                     QVector3D(0.0f, 8.0f * k_floor, 0.0f),
                     QSize(5, 8)));

    // cental upper
    merge(house,
          createRect(QVector3D(- 12.0f, 16.0f * k_floor, 4.0f),
                     QVector3D(12.0f - (- 12.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, (19.125f - 16.0f) * k_floor, 0.0f),
                     QSize(4, 11)));

    MeshData house_wo_doors = house;

    MeshData leftDoor = createRect(QVector3D(-12.0f, 8.0f * k_floor, 4.0f),
                                   QVector3D(0.0f - (- 12.0f), 0.0f, 0.0f),
                                   QVector3D(0.0f, (16.0f - 8.0f) * k_floor, 0.0f),
                                   QSize(1, 1));
    MeshData rightDoor = createRect(QVector3D(0.0f, 8.0f * k_floor, 4.0f),
                                    QVector3D(12.0f - (0.0f), 0.0f, 0.0f),
                                    QVector3D(0.0f, (16.0f - 8.0f) * k_floor, 0.0f),
                                    QSize(1, 1));

    merge(house, leftDoor);
    merge(house, rightDoor);

    m_meshHouse_wo_doors = GL_MeshPtr::create(GL_Mesh::createMesh(std::get<0>(house_wo_doors),
                                                                  std::get<1>(house_wo_doors),
                                                                  std::get<2>(house_wo_doors)));

    m_meshLeftDoor = GL_MeshPtr::create(GL_Mesh::createMesh(std::get<0>(leftDoor),
                                                            std::get<1>(leftDoor),
                                                            std::get<2>(leftDoor)));
    m_meshRightDoor = GL_MeshPtr::create(GL_Mesh::createMesh(std::get<0>(rightDoor),
                                                             std::get<1>(rightDoor),
                                                             std::get<2>(rightDoor)));

    m_meshHouse = GL_MeshPtr::create(GL_Mesh::createMesh(std::get<0>(house),
                                                         std::get<1>(house),
                                                         std::get<2>(house)));

    MeshData tables;
    merge(tables,
          createRect(QVector3D(- 22.5f, 0.0f * k_floor, - 2.0f),
                     QVector3D(- 21.0f - (- 22.5f), 0.0f, 0.0f),
                     QVector3D(0.0f, (20.25f) * k_floor, 0.0f),
                     QSize(2, 1)));
    merge(tables,
          createRect(QVector3D(21.f, 0.0f * k_floor, - 2.0f),
                     QVector3D(22.5f - (21.0f), 0.0f, 0.0f),
                     QVector3D(0.0f, (20.25f) * k_floor, 0.0f),
                     QSize(2, 1)));
    m_meshTables = GL_MeshPtr::create(GL_Mesh::createMesh(std::get<0>(tables),
                                                          std::get<1>(tables),
                                                          std::get<2>(tables)));
}

void HouseObject::_createMeshGrid(float border)
{
    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
    QVector<GLuint> indices;

    Vector3f delta = m_grid_end - m_grid_begin;

    for (int i = 0; i <= m_grid_n_size.y(); ++i)
    {
        float y = m_grid_begin.y() + delta.y() * (i / static_cast<float>(m_grid_n_size.y()));

        float y1 = y - border;
        float y2 = y + border;

        GLuint i_o = 8 * static_cast<GLuint>(i);
        vertices.push_back(QVector3D(m_grid_begin.x() - border, y1, m_grid_end.z()));
        vertices.push_back(QVector3D(m_grid_begin.x() - border, y2, m_grid_end.z()));
        vertices.push_back(QVector3D(m_grid_begin.x() - border, y1, m_grid_begin.z()));
        vertices.push_back(QVector3D(m_grid_begin.x() - border, y2, m_grid_begin.z()));
        vertices.push_back(QVector3D(m_grid_end.x() + border, y1, m_grid_begin.z()));
        vertices.push_back(QVector3D(m_grid_end.x() + border, y2, m_grid_begin.z()));
        vertices.push_back(QVector3D(m_grid_end.x() + border, y1, m_grid_end.z()));
        vertices.push_back(QVector3D(m_grid_end.x() + border, y2, m_grid_end.z()));

        texCoords.push_back(QVector2D(0.0f, 0.0f));
        texCoords.push_back(QVector2D(0.0f, 1.0f));
        texCoords.push_back(QVector2D(0.25f, 0.0f));
        texCoords.push_back(QVector2D(0.25f, 1.0f));
        texCoords.push_back(QVector2D(0.5f, 0.0f));
        texCoords.push_back(QVector2D(0.5f, 1.0f));
        texCoords.push_back(QVector2D(1.0f, 0.0f));
        texCoords.push_back(QVector2D(1.0f, 1.0f));

        indices.append({ i_o + 0, i_o + 3, i_o + 1,
                         i_o + 0, i_o + 2, i_o + 3,
                         i_o + 0, i_o + 1, i_o + 3,
                         i_o + 0, i_o + 3, i_o + 2,
                         i_o + 2, i_o + 5, i_o + 3,
                         i_o + 2, i_o + 4, i_o + 5,
                         i_o + 4, i_o + 7, i_o + 5,
                         i_o + 4, i_o + 6, i_o + 7,
                         i_o + 4, i_o + 5, i_o + 7,
                         i_o + 4, i_o + 7, i_o + 6
                       });
    }

    for (int i = 0; i <= m_grid_n_size.x(); ++i)
    {
        float x = m_grid_begin.x() + delta.x() * (i / static_cast<float>(m_grid_n_size.x()));

        GLuint i_o = static_cast<GLuint>(vertices.size());
        vertices.push_back(QVector3D(x - border, m_grid_begin.y() - border, m_grid_begin.z()));
        vertices.push_back(QVector3D(x + border, m_grid_begin.y() - border, m_grid_begin.z()));
        vertices.push_back(QVector3D(x - border, m_grid_end.y() + border, m_grid_begin.z()));
        vertices.push_back(QVector3D(x + border, m_grid_end.y() + border, m_grid_begin.z()));

        texCoords.push_back(QVector2D(0.0f, 0.0f));
        texCoords.push_back(QVector2D(1.0f, 0.0f));
        texCoords.push_back(QVector2D(0.0f, 1.0f));
        texCoords.push_back(QVector2D(1.0f, 1.0f));

        indices.append({ i_o + 0, i_o + 1, i_o + 3,
                         i_o + 0, i_o + 3, i_o + 2
                       });
    }

    m_meshGrid = GL_MeshPtr::create(GL_Mesh::createMesh(vertices, texCoords, indices));
}

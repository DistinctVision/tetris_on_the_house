#include "objectmodel.h"

#include <cmath>
#include <climits>
#include <limits>
#include <set>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "pinholecamera.h"

using namespace std;
using namespace Eigen;

ObjectModel::ObjectModel()
{}

ObjectModel ObjectModel::createBox(const Vector3f & size)
{
    Vector3f halfSize = size * 0.5f;

    ObjectModel box;
    box.m_vertices = {
        Vector3f(- halfSize.x(), - halfSize.y(), halfSize.z()),
        Vector3f(halfSize.x(), - halfSize.y(), halfSize.z()),
        Vector3f(halfSize.x(), halfSize.y(), halfSize.z()),
        Vector3f(- halfSize.x(), halfSize.y(), halfSize.z()),
        Vector3f(- halfSize.x(), - halfSize.y(), - halfSize.z()),
        Vector3f(halfSize.x(), - halfSize.y(), - halfSize.z()),
        Vector3f(halfSize.x(), halfSize.y(), - halfSize.z()),
        Vector3f(- halfSize.x(), halfSize.y(), - halfSize.z())
    };
    box.m_polygons = {
        Polygon {
            (VectorXi(4) << 0, 1, 2, 3).finished(),
            Vector3f(0.0f, 0.0f, 1.0f)
        },
        Polygon {
            (VectorXi(4) << 7, 6, 5, 4).finished(),
            Vector3f(0.0f, 0.0f, -1.0f)
        },
        Polygon {
            (VectorXi(4) << 1, 5, 6, 2).finished(),
            Vector3f(1.0f, 0.0f, 0.0f)
        },
        Polygon {
            (VectorXi(4) << 0, 3, 7, 4).finished(),
            Vector3f(-1.0f, 0.0f, 0.0f)
        },
        Polygon {
            (VectorXi(4) << 2, 6, 7, 3).finished(),
            Vector3f(0.0f, 1.0f, 0.0f)
        },
        Polygon {
            (VectorXi(4) << 0, 4, 5, 1).finished(),
            Vector3f(0.0f, -1.0f, 0.0f)
        }
    };

    /*const Vectors3d & v = box.vertices();
    for (size_t i = 0; i < box.polygons().size(); ++i)
    {
        const Polygon & polygon = box.polygons()[i];
        const VectorXi & vi = polygon.vertexIndices;
        Vector3d n = (v[vi[2]] - v[vi[1]]).cross(v[vi[0]] - v[vi[1]]).normalized();
        double nd = polygon.normal.dot(n);
        double dd = polygon.normal.dot(v[vi[3]] - v[vi[0]]);
        assert(fabs(nd - 1.0) < std::numeric_limits<double>::epsilon());
        assert(fabs(dd) < std::numeric_limits<double>::epsilon());
    }*/

    return box;
}

ObjectModel ObjectModel::createCubikRubik(float border)
{
    float i_border = 1.0f - border;

    ObjectModel model;

    Vector3f axisX(1.0f, 0.0f, 0.0f),
             axisY(0.0f, 1.0f, 0.0f),
             axisZ(0.0f, 0.0f, 1.0f);

    auto addSide = [&] ()
    {
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Vector2f v(i * (1.0f / 3.0f), j * (1.0f / 3.0f));
                int i_o = static_cast<int>(model.m_vertices.size());
                model.m_vertices.push_back((v.x() + (border / 3.0f) - 0.5f) * axisX +
                                           (v.y() + (border / 3.0f) - 0.5f) * axisY +
                                           axisZ * 0.5f);
                model.m_vertices.push_back((v.x() + (i_border / 3.0f) - 0.5f) * axisX +
                                           (v.y() + (border / 3.0f) - 0.5f) * axisY +
                                           axisZ * 0.5f);
                model.m_vertices.push_back((v.x() + (i_border / 3.0f) - 0.5f) * axisX +
                                           (v.y() + (i_border / 3.0f) - 0.5f) * axisY +
                                           axisZ * 0.5f);
                model.m_vertices.push_back((v.x() + (border / 3.0f) - 0.5f) * axisX +
                                           (v.y() + (i_border / 3.0f) - 0.5f) * axisY +
                                           axisZ * 0.5f);
                model.m_polygons.push_back(Polygon { (VectorXi(4) << i_o + 0, i_o + 1, i_o + 2, i_o + 3).finished(),
                                                      axisZ });
            }
        }
    };

    for (int k = 0; k < 4; ++k)
    {
        addSide();
        axisZ.swap(axisY);
        axisZ = - axisZ;
    }

    axisX = Vector3f(0.0f, 0.0f, 1.0f);
    axisY = Vector3f(0.0f, 1.0f, 0.0f);
    axisZ = Vector3f(-1.0f, 0.0f, 0.0f);
    addSide();

    axisX = - axisX;
    axisZ = - axisZ;
    addSide();

    /*const Vectors3d & v = model.vertices();
    for (size_t i = 0; i < model.polygons().size(); ++i)
    {
        const Polygon & polygon = model.polygons()[i];
        const VectorXi & vi = polygon.vertexIndices;
        Vector3d n = (v[vi[2]] - v[vi[1]]).cross(v[vi[0]] - v[vi[1]]).normalized();
        double nd = polygon.normal.dot(n);
        double dd = polygon.normal.dot(v[vi[3]] - v[vi[0]]);
        assert(fabs(nd - 1.0) < std::numeric_limits<double>::epsilon());
        assert(fabs(dd) < std::numeric_limits<double>::epsilon());
    }*/

    return model;
}


ObjectModel ObjectModel::createHouse()
{
    float k_floor = 2.7f;

    ObjectModel model;
    model.m_vertices = {
        Vector3f(- 31.0f, 8.0f * k_floor, 4.0f),       // 0
        Vector3f(- 31.0f, 19.0f * k_floor, 4.0f),      // 1
        Vector3f(- 31.0f, 19.0f * k_floor, 0.0f),     // 2
        Vector3f(- 31.0f, 8.0f * k_floor, 0.0f),      // 3

        Vector3f(- 23.5f, 19.0f * k_floor, 0.0f),     // 4
        Vector3f(- 23.5f, 8.0f * k_floor,  0.0f),     // 5

        Vector3f( - 23.0f, 0.0f * k_floor, 2.0f),    // 6
        Vector3f( - 23.0f, 20.25f * k_floor, 2.0f),   // 7
        Vector3f( - 20.0f, 20.25f * k_floor, 2.0f),   // 8
        Vector3f( - 20.0f, 0.0f * k_floor, 2.0f),    // 9

        Vector3f(- 19.5f, 8.0f * k_floor, 0.0f),      // 10
        Vector3f(- 19.5f, 19.0f * k_floor, 0.0f),    // 11
        Vector3f(- 12.0f, 19.0f * k_floor, 0.0f),     // 12
        Vector3f(- 12.0f, 8.0f * k_floor, 0.0f),     // 13

        Vector3f(- 12.0f, 8.0f * k_floor, 4.0f),       // 14
        Vector3f(- 12.0f, 19.0f * k_floor,  4.0f),     // 15
    };
    model.m_polygons = {
        Polygon {
            (VectorXi(4) << 0, 1, 2, 3).finished(),
            Vector3f(-1.0f, 0.0f, 0.0f)
        },
        Polygon {
            (VectorXi(4) << 3, 2, 4, 5).finished(),
            Vector3f(0.0f, 0.0f, -1.0f)
        },
        Polygon {
            (VectorXi(4) << 6, 7, 8, 9).finished(),
            Vector3f(0.0f, 0.0f, -1.0f)
        },
        Polygon {
            (VectorXi(4) << 10, 11, 12, 13).finished(),
            Vector3f(0.0f, 0.0f, -1.0f)
        },
        Polygon {
            (VectorXi(4) << 12, 15, 14, 13).finished(),
            Vector3f(1.0f, 0.0f, 0.0f)
        },
    };

    model.m_disabledEdges.insert(make_pair(0, 1));
    model.m_disabledEdges.insert(make_pair(2, 3));
    model.m_disabledEdges.insert(make_pair(4, 5));
    model.m_disabledEdges.insert(make_pair(7, 8));
    model.m_disabledEdges.insert(make_pair(6, 9));
    model.m_disabledEdges.insert(make_pair(10, 11));
    model.m_disabledEdges.insert(make_pair(12, 13));
    model.m_disabledEdges.insert(make_pair(13, 14));
    model.m_disabledEdges.insert(make_pair(14, 15));

    model.merge(mirroredX(model));

    const Vectors3f & v = model.m_vertices;
    for (size_t i = 0; i < model.polygons().size(); ++i)
    {
        const Polygon & polygon = model.polygons()[i];
        const VectorXi & vi = polygon.vertexIndices;
        Vector3f n = (v[vi[2]] - v[vi[1]]).cross(v[vi[0]] - v[vi[1]]).normalized();
        float nd = polygon.normal.dot(n);
        float dd = polygon.normal.dot(v[vi[3]] - v[vi[0]]);
        assert(fabs(nd - 1.0f) < std::numeric_limits<float>::epsilon());
        assert(fabs(dd) < std::numeric_limits<float>::epsilon());
    }

    return model;
}

ObjectModel ObjectModel::mirroredX(const ObjectModel & model)
{
    ObjectModel r(model);
    for (Vector3f & v : r.m_vertices)
    {
        v.x() = - v.x();
    }
    for (Polygon & polygon : r.m_polygons)
    {
        polygon.normal.x() = - polygon.normal.x();
        VectorXi vertexIndices(polygon.vertexIndices);
        for (int i = 0; i < polygon.vertexIndices.size(); ++i)
            vertexIndices[i] = polygon.vertexIndices[polygon.vertexIndices.size() - 1 - i];
        polygon.vertexIndices = move(vertexIndices);
    }
    return r;
}

ObjectModel & ObjectModel::merge(const ObjectModel & model)
{
    int offset_v = static_cast<int>(m_vertices.size());
    Polygons new_polygons = model.m_polygons;
    for (Polygon & polygon: new_polygons)
    {
        for (int i = 0; i < polygon.vertexIndices.size(); ++i)
        {
            polygon.vertexIndices[i] += offset_v;
        }
    }
    m_vertices.insert(m_vertices.end(), model.m_vertices.cbegin(), model.m_vertices.cend());
    m_polygons.insert(m_polygons.end(), new_polygons.cbegin(), new_polygons.cend());
    for (const pair<int, int> & i_d_edge : model.m_disabledEdges)
        m_disabledEdges.insert(make_pair(i_d_edge.first + offset_v, i_d_edge.second + offset_v));
    return (*this);
}

const Vectors3f & ObjectModel::vertices() const
{
    return m_vertices;
}

const ObjectModel::Polygons & ObjectModel::polygons() const
{
    return m_polygons;
}

Vectors3f ObjectModel::getControlPoints(const std::shared_ptr<PinholeCamera> & camera,
                                        float controlPixelDistance,
                                        const Matrix3f & R, const Vector3f & t) const
{
    std::set<int> setOfVertices;
    std::set<std::pair<int, int>, less_pair_i> setOfEdges;
    Vector3f cam_pose = - R.inverse() * t;
    for (const Polygon & polygon : m_polygons)
    {
        if (polygon.normal.dot(cam_pose - m_vertices[polygon.vertexIndices[0]]) > 0.0f)
        {
            for (int i = 0; i < polygon.vertexIndices.size(); ++i)
            {
                std::pair<int, int> edge(polygon.vertexIndices[i],
                                         polygon.vertexIndices[(i + 1) % polygon.vertexIndices.size()]);
                if (edge.first > edge.second)
                    std::swap(edge.first, edge.second);
                if (m_disabledEdges.find(edge) != m_disabledEdges.cend())
                    continue;
                setOfEdges.insert(edge);
                setOfVertices.insert(polygon.vertexIndices[i]);
            }
        }
    }

    Vectors3f controlModelPoints;
    for (auto it = setOfVertices.cbegin(); it != setOfVertices.cend(); ++it)
    {
        const Vector3f & vertex = m_vertices[*it];
        bool inViewFlag;
        Vector2f p = camera->project((R * vertex + t).eval(), inViewFlag);
        (void)(p);
        if (!inViewFlag)
            continue;
        controlModelPoints.push_back(vertex);
    }
    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        const Vector3f & vertex1 = m_vertices[itEdge->first];
        const Vector3f & vertex2 = m_vertices[itEdge->second];

        Vector3f v1 = (R * vertex1 + t);
        if (v1.z() < numeric_limits<float>::epsilon())
            continue;
        Vector3f v2 = (R * vertex2 + t);
        if (v2.z() < numeric_limits<float>::epsilon())
            continue;

        Vector2f p1 = camera->project(v1);
        Vector2f p2 = camera->project(v2);

        float distance = (p2 - p1).norm();
        int n = static_cast<int>(ceil(distance / controlPixelDistance));
        if (n <= 1)
            continue;

        Vector3f delta = vertex2 - vertex1;
        float step = 1.0f / static_cast<float>(n);
        for (int i = 1; i < n; ++i)
        {
            float k = i * step;
            Vector3f v = vertex1 + delta * k;
            bool inViewFlag;
            Vector2f p = camera->project((R * v + t).eval(), inViewFlag);
            (void)(p);
            if (!inViewFlag)
                continue;
            controlModelPoints.push_back(v);
        }
    }
    return controlModelPoints;
}

tuple<Vectors3f, Vectors2f> ObjectModel::getControlAndImagePoints(const shared_ptr<PinholeCamera> & camera,
                                                                  float controlPixelDistance,
                                                                  const Matrix3f & R, const Vector3f & t) const
{
    set<int> setOfVertices;
    set<std::pair<int, int>, less_pair_i> setOfEdges;
    Vector3f cam_pose = - R.inverse() * t;
    for (const Polygon & polygon : m_polygons)
    {
        if (polygon.normal.dot(cam_pose - m_vertices[polygon.vertexIndices[0]]) > 0.0f)
        {
            for (int i = 0; i < polygon.vertexIndices.size(); ++i)
            {
                std::pair<int, int> edge(polygon.vertexIndices[i],
                                         polygon.vertexIndices[(i + 1) % polygon.vertexIndices.size()]);
                if (edge.first > edge.second)
                    std::swap(edge.first, edge.second);
                if (m_disabledEdges.find(edge) != m_disabledEdges.cend())
                    continue;
                setOfEdges.insert(edge);
                setOfVertices.insert(polygon.vertexIndices[i]);
            }
        }
    }

    Vectors3f controlModelPoints;
    Vectors2f imagePoints;
    for (auto it = setOfVertices.cbegin(); it != setOfVertices.cend(); ++it)
    {
        const Vector3f & vertex = m_vertices[*it];
        Vector3f v = R * vertex + t;
        if (v.z() < numeric_limits<float>::epsilon())
            continue;
        bool inViewFlag;
        Vector2f p = camera->project(v, inViewFlag);
        if (!inViewFlag)
            continue;
        controlModelPoints.push_back(vertex);
        imagePoints.push_back(p);
    }

    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        const Vector3f & vertex1 = m_vertices[itEdge->first];
        Vector3f v1 = R * vertex1 + t;
        if (v1.z() < numeric_limits<float>::epsilon())
            continue;
        const Vector3f & vertex2 = m_vertices[itEdge->second];
        Vector3f v2 = R * vertex2 + t;
        if (v2.z() < numeric_limits<float>::epsilon())
            continue;

        bool inViewFlag;
        Vector2f p1 = camera->project(v1, inViewFlag);
        if (!inViewFlag)
            continue;
        Vector2f p2 = camera->project(v2, inViewFlag);
        if (!inViewFlag)
            continue;

        float distance = (p2 - p1).norm();
        int n = static_cast<int>(ceil(distance / controlPixelDistance));
        if (n <= 1)
            continue;

        Vector3f delta = vertex2 - vertex1;
        float step = 1.0f / static_cast<float>(n);
        for (int i = 1; i < n; ++i)
        {
            float k = i * step;
            Vector3f v = vertex1 + delta * k;
            Vector2f p = camera->project((R * v + t).eval());
            controlModelPoints.push_back(v);
            imagePoints.push_back(p);
        }
    }
    return make_tuple(controlModelPoints, imagePoints);
}

void ObjectModel::draw(const cv::Mat & image,
                       const shared_ptr<PinholeCamera> & camera,
                       const Matrix3f & R, const Vector3f & t) const
{
    std::set<std::pair<int, int>> setOfEdges;
    Vector3f cam_pose = - R.inverse() * t;
    for (const Polygon & polygon : m_polygons)
    {
        if (polygon.normal.dot(cam_pose - m_vertices[polygon.vertexIndices[0]]) > 0.0f)
        {
            for (int i = 0; i < polygon.vertexIndices.size(); ++i)
            {
                std::pair<int, int> edge(polygon.vertexIndices[i],
                                         polygon.vertexIndices[(i + 1) % polygon.vertexIndices.size()]);
                if (edge.first > edge.second)
                    std::swap(edge.first, edge.second);
                setOfEdges.insert(edge);
            }
        }
    }

    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        const Vector3f & vertex1 = m_vertices[std::get<0>(*itEdge)];
        Vector3f v1 = R * vertex1 + t;
        if (v1.z() < numeric_limits<float>::epsilon())
            continue;
        const Vector3f & vertex2 = m_vertices[itEdge->second];
        Vector3f v2 = R * vertex2 + t;
        if (v2.z() < numeric_limits<float>::epsilon())
            continue;

        Vector2f p1 = camera->project(v1);
        Vector2f p2 = camera->project(v2);

        cv::Scalar color = (m_disabledEdges.find(*itEdge) == m_disabledEdges.cend()) ? cv::Scalar(0, 255, 0) :
                                                                                       cv::Scalar(255, 0, 0);

        cv::line(image, cv::Point2f(p1.x(), p1.y()), cv::Point2f(p2.x(), p2.y()), color, 1);
    }
}


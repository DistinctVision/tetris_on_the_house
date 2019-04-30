#include "objectmodel.h"

#include <cmath>
#include <climits>
#include <limits>
#include <set>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "pinholecamera.h"

using namespace Eigen;

ObjectModel::ObjectModel()
{}

ObjectModel ObjectModel::createBox(const Vector3d & size)
{
    Vector3d halfSize = size * 0.5;

    ObjectModel box;
    box.m_vertices = {
        Vector3d(- halfSize.x(), - halfSize.y(), halfSize.z()),
        Vector3d(halfSize.x(), - halfSize.y(), halfSize.z()),
        Vector3d(halfSize.x(), halfSize.y(), halfSize.z()),
        Vector3d(- halfSize.x(), halfSize.y(), halfSize.z()),
        Vector3d(- halfSize.x(), - halfSize.y(), - halfSize.z()),
        Vector3d(halfSize.x(), - halfSize.y(), - halfSize.z()),
        Vector3d(halfSize.x(), halfSize.y(), - halfSize.z()),
        Vector3d(- halfSize.x(), halfSize.y(), - halfSize.z())
    };
    box.m_polygons = {
        Polygon {
            (VectorXi(4) << 0, 1, 2, 3).finished(),
            Vector3d(0.0, 0.0, 1.0)
        },
        Polygon {
            (VectorXi(4) << 7, 6, 5, 4).finished(),
            Vector3d(0.0, 0.0, -1.0)
        },
        Polygon {
            (VectorXi(4) << 1, 5, 6, 2).finished(),
            Vector3d(1.0, 0.0, 0.0)
        },
        Polygon {
            (VectorXi(4) << 0, 3, 7, 4).finished(),
            Vector3d(-1.0, 0.0, 0.0)
        },
        Polygon {
            (VectorXi(4) << 2, 6, 7, 3).finished(),
            Vector3d(0.0, 1.0, 0.0)
        },
        Polygon {
            (VectorXi(4) << 0, 4, 5, 1).finished(),
            Vector3d(0.0, -1.0, 0.0)
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

ObjectModel ObjectModel::createCubikRurbik()
{
    ObjectModel model;

    Vector3d axisX(1.0, 0.0, 0.0),
             axisY(0.0, 1.0, 0.0),
             axisZ(0.0, 0.0, 1.0);

    auto addSide = [&] ()
    {
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                Vector2d v(i * (1.0 / 3.0), j * (1.0 / 3.0));
                int i_o = static_cast<int>(model.m_vertices.size());
                model.m_vertices.push_back((v.x() + (0.1 / 3.0) - 0.5) * axisX +
                                           (v.y() + (0.1 / 3.0) - 0.5) * axisY +
                                           axisZ * 0.5);
                model.m_vertices.push_back((v.x() + (0.9 / 3.0) - 0.5) * axisX +
                                           (v.y() + (0.1 / 3.0) - 0.5) * axisY +
                                           axisZ * 0.5);
                model.m_vertices.push_back((v.x() + (0.9 / 3.0) - 0.5) * axisX +
                                           (v.y() + (0.9 / 3.0) - 0.5) * axisY +
                                           axisZ * 0.5);
                model.m_vertices.push_back((v.x() + (0.1 / 3.0) - 0.5) * axisX +
                                           (v.y() + (0.9 / 3.0) - 0.5) * axisY +
                                           axisZ * 0.5);
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

    axisX = Vector3d(0.0, 0.0, 1.0);
    axisY = Vector3d(0.0, 1.0, 0.0);
    axisZ = Vector3d(-1.0, 0.0, 0.0);
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

const Vectors3d & ObjectModel::vertices() const
{
    return m_vertices;
}

const ObjectModel::Polygons & ObjectModel::polygons() const
{
    return m_polygons;
}

std::tuple<Vectors3d, Vectors2f>
ObjectModel::getControlPoints(const std::shared_ptr<PinholeCamera> & camera,
                              float controlPixelDistance,
                              const Matrix3d & R, const Vector3d & t) const
{
    std::set<int> setOfVertices;
    std::set<std::pair<int, int>, less_pair_i> setOfEdges;
    Vector3d cam_pose = - R.inverse() * t;
    for (const Polygon & polygon : m_polygons)
    {
        if (polygon.normal.dot(cam_pose - m_vertices[polygon.vertexIndices[0]]) > 0.0)
        {
            for (int i = 0; i < polygon.vertexIndices.size(); ++i)
            {
                std::pair<int, int> edge(polygon.vertexIndices[i],
                                         polygon.vertexIndices[(i + 1) % polygon.vertexIndices.size()]);
                if (edge.first > edge.second)
                    std::swap(edge.first, edge.second);
                setOfEdges.insert(edge);
                setOfVertices.insert(polygon.vertexIndices[i]);
            }
        }
    }

    Vectors3d controlModelPoints;
    Vectors2f controlViewPoints;
    for (auto it = setOfVertices.cbegin(); it != setOfVertices.cend(); ++it)
    {
        const Vector3d & v = m_vertices[*it];
        bool inViewFlag;
        Vector2f p = camera->project((R * v + t).cast<float>(), inViewFlag);
        if (!inViewFlag)
            continue;
        controlModelPoints.push_back(v);
        controlViewPoints.push_back(p);
    }
    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        const Vector3d & v1 = m_vertices[itEdge->first];
        const Vector3d & v2 = m_vertices[itEdge->second];

        bool inViewFlag;
        Vector2f p1 = camera->project((R * v1 + t).cast<float>(), inViewFlag);
        if (!inViewFlag)
            continue;
        Vector2f p2 = camera->project((R * v2 + t).cast<float>(), inViewFlag);
        if (!inViewFlag)
            continue;

        float distance = (p2 - p1).norm();
        int n = static_cast<int>(ceil(distance / controlPixelDistance));
        if (n <= 1)
            continue;

        Vector3d delta = v2 - v1;
        double step = 1.0 / static_cast<double>(n);
        for (int i = 1; i < n; ++i)
        {
            double k = i * step;
            Vector3d v = v1 + delta * k;
            Vector2f p = camera->project((R * v + t).cast<float>());
            controlModelPoints.push_back(v);
            controlViewPoints.push_back(p);
        }
    }
    return std::make_tuple(controlModelPoints, controlViewPoints);
}

void ObjectModel::draw(const cv::Mat & image,
                       const std::shared_ptr<PinholeCamera> & camera,
                       const Matrix3d & R, const Vector3d & t) const
{
    std::set<std::pair<int, int>> setOfEdges;
    Vector3d cam_pose = - R.inverse() * t;
    for (const Polygon & polygon : m_polygons)
    {
        if (polygon.normal.dot(cam_pose - m_vertices[polygon.vertexIndices[0]]) > 0.0)
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
        const Vector3d & v1 = m_vertices[itEdge->first];
        const Vector3d & v2 = m_vertices[itEdge->second];

        bool inViewFlag;
        Vector2f p1 = camera->project((R * v1 + t).cast<float>(), inViewFlag);
        if (!inViewFlag)
            continue;
        Vector2f p2 = camera->project((R * v2 + t).cast<float>(), inViewFlag);
        if (!inViewFlag)
            continue;

        cv::line(image, cv::Point2f(p1.x(), p1.y()), cv::Point2f(p2.x(), p2.y()),
                 cv::Scalar(255, 0, 0), 2);
    }
}


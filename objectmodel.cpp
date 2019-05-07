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
                setOfEdges.insert(edge);
                setOfVertices.insert(polygon.vertexIndices[i]);
            }
        }
    }

    Vectors3f controlModelPoints;
    for (auto it = setOfVertices.cbegin(); it != setOfVertices.cend(); ++it)
    {
        const Vector3f & v = m_vertices[*it];
        bool inViewFlag;
        Vector2f p = camera->project((R * v + t).eval(), inViewFlag);
        if (!inViewFlag)
            continue;
        controlModelPoints.push_back(v);
    }
    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        const Vector3f & v1 = m_vertices[itEdge->first];
        const Vector3f & v2 = m_vertices[itEdge->second];

        bool inViewFlag;
        Vector2f p1 = camera->project((R * v1 + t).eval(), inViewFlag);
        if (!inViewFlag)
            continue;
        Vector2f p2 = camera->project((R * v2 + t).eval(), inViewFlag);
        if (!inViewFlag)
            continue;

        float distance = (p2 - p1).norm();
        int n = static_cast<int>(ceil(distance / controlPixelDistance));
        if (n <= 1)
            continue;

        Vector3f delta = v2 - v1;
        float step = 1.0f / static_cast<float>(n);
        for (int i = 1; i < n; ++i)
        {
            float k = i * step;
            Vector3f v = v1 + delta * k;
            //Vector2f p = camera->project((R * v + t).eval());
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
                setOfEdges.insert(edge);
                setOfVertices.insert(polygon.vertexIndices[i]);
            }
        }
    }

    Vectors3f controlModelPoints;
    Vectors2f imagePoints;
    for (auto it = setOfVertices.cbegin(); it != setOfVertices.cend(); ++it)
    {
        Vector3f v = R * m_vertices[*it] + t;
        if (v.z() < numeric_limits<float>::epsilon())
            continue;
        bool inViewFlag;
        Vector2f view = v.segment<2>(0) / v.z();
        Vector2f p = camera->project(view, inViewFlag);
        if (!inViewFlag)
            continue;
        controlModelPoints.push_back(v);
        imagePoints.push_back(p);
    }
    for (auto itEdge = setOfEdges.cbegin(); itEdge != setOfEdges.cend(); ++itEdge)
    {
        Vector3f v1 = R * m_vertices[itEdge->first] + t;
        if (v1.z() < numeric_limits<float>::epsilon())
            continue;
        Vector3f v2 = R * m_vertices[itEdge->second] + t;
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

        Vector3f delta = v2 - v1;
        float step = 1.0f / static_cast<float>(n);
        for (int i = 1; i < n; ++i)
        {
            float k = i * step;
            Vector3f v = R * (v1 + delta * k) + t;
            Vector2f p = camera->project(v);
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
        const Vector3f & v1 = m_vertices[itEdge->first];
        const Vector3f & v2 = m_vertices[itEdge->second];

        bool inViewFlag;
        Vector2f p1 = camera->project((R * v1 + t).eval(), inViewFlag);
        if (!inViewFlag)
            continue;
        Vector2f p2 = camera->project((R * v2 + t).eval(), inViewFlag);
        if (!inViewFlag)
            continue;

        cv::line(image, cv::Point2f(p1.x(), p1.y()), cv::Point2f(p2.x(), p2.y()),
                 cv::Scalar(255, 0, 0), 2);
    }
}


#include "objectmodel.h"
#include <cmath>
#include <climits>
#include <limits>

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

    /*const Vectors3d &v = box.vertices();
    for (size_t i = 0; i < box.polygons().size(); ++i)
    {
        const Polygon & polygon = box.polygons()[i];
        const VectorXi &vi = polygon.vertexIndices;
        Vector3d n = (v[vi[2]] - v[vi[1]]).cross(v[vi[0]] - v[vi[1]]).normalized();
        double nd = polygon.normal.dot(n);
        double dd = polygon.normal.dot(v[vi[3]] - v[vi[0]]);
        assert(fabs(nd - 1.0) < std::numeric_limits<double>::epsilon());
        assert(fabs(dd) < std::numeric_limits<double>::epsilon());
    }*/

    return box;
}

const Vectors3d & ObjectModel::vertices() const
{
    return m_vertices;
}

const ObjectModel::Polygons & ObjectModel::polygons() const
{
    return m_polygons;
}


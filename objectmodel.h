#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <vector>
#include <memory>

#include <Eigen/Eigen>

class PinholeCamera;

using VectorsXi = std::vector<Eigen::VectorXi, Eigen::aligned_allocator<Eigen::VectorXi>>;
using Vectors2d = std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>>;
using Vectors3d = std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>>;

class ObjectModel
{
public:
    struct Polygon
    {
         Eigen::VectorXi vertexIndices;
         Eigen::Vector3d normal;
    };
    using Polygons = std::vector<Polygon, Eigen::aligned_allocator<Polygon>>;

    static ObjectModel createBox(const Eigen::Vector3d & size = Eigen::Vector3d(1.0, 1.0, 1.0));

    const Vectors3d & vertices() const;
    const Polygons & polygons() const;

    Vectors3d getControlPoints(const std::shared_ptr<PinholeCamera> & camera,
                               float controlPixelDistance,
                               const Eigen::Matrix3d & R,
                               const Eigen::Vector3d & t) const;

private:
    ObjectModel();

    Vectors3d m_vertices;
    Polygons m_polygons;
};


#endif // OBJECTMODEL_H

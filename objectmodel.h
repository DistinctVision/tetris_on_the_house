#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <vector>
#include <memory>
#include <utility>
#include <tuple>

#include <Eigen/Eigen>

#include <opencv2/core.hpp>

class PinholeCamera;

using VectorsXi = std::vector<Eigen::VectorXi, Eigen::aligned_allocator<Eigen::VectorXi>>;
using Vectors2f = std::vector<Eigen::Vector2f, Eigen::aligned_allocator<Eigen::Vector2f>>;
using Vectors3f = std::vector<Eigen::Vector3f, Eigen::aligned_allocator<Eigen::Vector3f>>;
using Vectors2d = std::vector<Eigen::Vector2d, Eigen::aligned_allocator<Eigen::Vector2d>>;
using Vectors3d = std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d>>;

class ObjectModel
{
public:
    struct less_pair_i
    {
        bool operator () (const std::pair<int, int> & lhs, const std::pair<int, int> & rhs) const
        {
            return (lhs.first < rhs.first) ? true : ((lhs.first == rhs.first) && (lhs.second < rhs.second));
        }
    };

    struct Polygon
    {
         Eigen::VectorXi vertexIndices;
         Eigen::Vector3d normal;
    };
    using Polygons = std::vector<Polygon, Eigen::aligned_allocator<Polygon>>;

    static ObjectModel createBox(const Eigen::Vector3d & size = Eigen::Vector3d(1.0, 1.0, 1.0));
    static ObjectModel createCubikRurbik();

    const Vectors3d & vertices() const;
    const Polygons & polygons() const;

    std::tuple<Vectors3d, Vectors2f>
    getControlPoints(const std::shared_ptr<PinholeCamera> & camera,
                     float controlPixelDistance,
                     const Eigen::Matrix3d & R,
                     const Eigen::Vector3d & t) const;

    void draw(const cv::Mat & image,
              const std::shared_ptr<PinholeCamera> & camera,
              const Eigen::Matrix3d & R, const Eigen::Vector3d & t) const;

private:
    ObjectModel();

    Vectors3d m_vertices;
    Polygons m_polygons;
};

#endif // OBJECTMODEL_H
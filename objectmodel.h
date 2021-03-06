#ifndef OBJECTMODEL_H
#define OBJECTMODEL_H

#include <vector>
#include <set>
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

    using FlagsVector = Eigen::Matrix<uchar, Eigen::Dynamic, 1>;

    struct Polygon
    {
         Eigen::VectorXi vertexIndices;
         Eigen::Vector3f normal;
    };
    using Polygons = std::vector<Polygon, Eigen::aligned_allocator<Polygon>>;

    static ObjectModel createBox(const Eigen::Vector3f & size = Eigen::Vector3f(1.0f, 1.0f, 1.0f));
    static ObjectModel createCubikRubik(float border = 0.15f);
    static ObjectModel createHouse();

    static ObjectModel mirroredX(const ObjectModel & model);

    ObjectModel & merge(const ObjectModel & model);

    const Vectors3f & vertices() const;
    const Polygons & polygons() const;

    Vectors3f getControlPoints(const std::shared_ptr<PinholeCamera> & camera,
                               float controlPixelDistance,
                               const Eigen::Matrix3f & R,
                               const Eigen::Vector3f & t) const;

    std::tuple<Vectors3f, Vectors2f> getControlAndImagePoints(const std::shared_ptr<PinholeCamera> & camera,
                                                              float controlPixelDistance,
                                                              const Eigen::Matrix3f & R, const Eigen::Vector3f & t) const;

    void draw(const cv::Mat & image,
              const std::shared_ptr<PinholeCamera> & camera,
              const Eigen::Matrix3f & R, const Eigen::Vector3f & t) const;

private:
    ObjectModel();

    Vectors3f m_vertices;
    Polygons m_polygons;

    std::set<std::pair<int, int>> m_disabledEdges;
};

#endif // OBJECTMODEL_H

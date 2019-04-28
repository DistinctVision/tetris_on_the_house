#include "distancemapgenerator.h"

#include <list>

using namespace std;
using namespace sonar;

const Point2i DistanceMapGenerator::nearPixels[] = {
    Point2i(-1, -1),
    Point2i(0, -1),
    Point2i(1, -1),
    Point2i(1, 0),
    Point2i(1, 1),
    Point2i(0, 1),
    Point2i(-1, 1),
    Point2i(-1, 0)
};

const float DistanceMapGenerator::nearPixels_score[] = {
    1.41421356f,
    1.0f,
    1.41421356f,
    1.0f,
    1.41421356f,
    1.0f,
    1.41421356f,
    1.0f
};

DistanceMapGenerator::DistanceMapGenerator()
{

}

ConstImage<DistanceMapGenerator::Link> DistanceMapGenerator::compute(const ImageRef<uchar> & binaryImage)
{
    if (m_map.size() != binaryImage.size())
        m_map = Image<Link>(binaryImage.size());

    list<Point2i> activePoints;
    m_map.fill({ -1.0f, Point2i(-1, -1) });

    Point2i p(0, 0);
    {
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 3, 6))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
        for (p.x = 1; p.x < binaryImage.width() - 1; ++p.x)
        {
            if (binaryImage(p))
            {
                if (_existNullNeighbors(binaryImage, p, 3, 0))
                {
                    activePoints.push_back(p);
                    m_map(p) = Link{ 0.0f, p };
                }
            }
        }
        p.x = binaryImage.width() - 1;
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 5, 0))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
    }
    for (p.y = 1; p.y < binaryImage.height() - 1; ++p.y)
    {
        p.x = 0;
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 1, 6))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
        for (p.x = 1; p.x < binaryImage.width() - 1; ++p.x)
        {
            if (binaryImage(p))
            {
                if (_existNullNeighbors(binaryImage, p, 0, 0))
                {
                    activePoints.push_back(p);
                    m_map(p) = Link{ 0.0f, p };
                }
            }
        }
        p.x = binaryImage.width() - 1;
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 5, 2))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
    }
    {
        p.y = binaryImage.height() - 1;
        p.x = 0;
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 1, 4))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
        for (p.x = 1; p.x < binaryImage.width() - 1; ++p.x)
        {
            if (binaryImage(p))
            {
                if (_existNullNeighbors(binaryImage, p, 7, 4))
                {
                    activePoints.push_back(p);
                    m_map(p) = Link{ 0.0f, p };
                }
            }
        }
        p.x = binaryImage.width() - 1;
        if (binaryImage(p))
        {
            if (_existNullNeighbors(binaryImage, p, 7, 2))
            {
                activePoints.push_back(p);
                m_map(p) = Link{ 0.0f, p };
            }
        }
    }

    list<Point2i> segmentsPoints;
    while (!activePoints.empty())
    {
        Point2i cur_p = activePoints.front();
        activePoints.pop_front();
        Link & cur_link = m_map(cur_p);
        bool isEnd = true;
        for (int i = 0; i < 8; ++i)
        {
            Point2i n_p = cur_p + nearPixels[i];
            if (!m_map.pointInImage(n_p))
                continue;
            if (binaryImage(n_p) == 0)
                continue;
            Link & n_link = m_map(n_p);
            float d = cur_link.d + nearPixels_score[i];
            if (n_link.d < 0.0f)
            {
                n_link.d = d;
                n_link.source = cur_link.source;
                activePoints.push_back(n_p);
                isEnd = false;
            }
            else if (d < n_link.d)
            {
                n_link.d = d;
                n_link.source = cur_link.source;
                isEnd = false;
            }
            else if (n_link.d > cur_link.d)
            {
                isEnd = false;
            }
        }
        if (isEnd)
            segmentsPoints.push_back(cur_p);
    }
    m_map.fill(Link{ -1.0f, Point2i(-1, -1) });
    activePoints = move(segmentsPoints);
    for (const Point2i &p: activePoints)
        m_map(p) = Link{ 0.0f, p };

    while (!activePoints.empty())
    {
        Point2i cur_p = activePoints.front();
        activePoints.pop_front();
        Link & cur_link = m_map(cur_p);
        for (int i = 0; i < 8; ++i)
        {
            Point2i n_p = cur_p + nearPixels[i];
            if (!m_map.pointInImage(n_p))
                continue;
            Link & n_link = m_map(n_p);
            float d = cur_link.d + nearPixels_score[i];
            if (n_link.d < 0.0f)
            {
                n_link.d = d;
                n_link.source = cur_link.source;
                activePoints.push_back(n_p);
            }
            else if (d < n_link.d)
            {
                n_link.d = d;
                n_link.source = cur_link.source;
            }
        }
    }

    return m_map;
}

bool DistanceMapGenerator::_existNullNeighbors(const ImageRef<uchar> & binaryImage,
                                                    const Point2i & p,
                                                    int begin_index, int end_index) const
{
    int i = begin_index;
    do
    {
        if (binaryImage(p + nearPixels[i]) == 0)
            return true;
        i = (i + 1) % 8;
    } while (i != end_index);
    return false;
}

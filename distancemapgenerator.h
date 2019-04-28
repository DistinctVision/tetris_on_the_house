#ifndef DISTANCEMAPGENERATOR_H
#define DISTANCEMAPGENERATOR_H

#include <sonar/General/Point2.h>
#include <sonar/General/Image.h>

class DistanceMapGenerator
{
public:
    struct Link
    {
        float d;
        sonar::Point2i source;
    };

    static const sonar::Point2i nearPixels[8];
    static const float nearPixels_score[8];

    DistanceMapGenerator();

    sonar::ConstImage<Link> compute(const sonar::ImageRef<uchar> & binaryImage);

private:
    sonar::Image<Link> m_map;

    inline bool _existNullNeighbors(const sonar::ImageRef<uchar> & binaryImage,
                                    const sonar::Point2i &p,
                                    int begin_index, int end_index) const;
};

#endif // DISTANCEMAPGENERATOR_H

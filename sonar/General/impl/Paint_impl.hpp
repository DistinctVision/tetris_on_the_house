/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_PAINT_IMPL_HPP
#define SONAR_PAINT_IMPL_HPP

namespace sonar {

namespace paint {

template < typename Type >
void drawLine(const Image<Type> & image, float x1, float y1, float x2, float y2, const Type & color)
{
    bool steep = false;
    if (std::abs(x2 - x1) < std::abs(y2 - y1)) {
        std::swap(x1, y1);
        std::swap(x2, y2);
        steep = true;
    }
    if (x1 > x2) {
        std::swap(x1, x2);
        std::swap(y1, y2);
    }
    float dx = x2 - x1;
    float dy = y2 - y1;
    int y_step = (y2 > y1) ? 1 : -1;
    int dx_i = (int)dx;
    int dy_i = (int)dy;
    int derror2 = std::abs(dy_i) * 2;
    int error2 = 0;
    int y = (int)y1;
    if (steep) {
        for (int x = (int)x1; x <= (int)x2; x++) {
            if (image.pointInImage(y, x))
                image(y, x) = color;
            error2 += derror2;

            if (error2 > dx) {
                y += y_step;
                error2 -= dx_i * 2;
            }
        }
    } else {
        for (int x = (int)x1; x <= (int)x2; x++) {
            if (image.pointInImage(x, y))
                image(x, y) = color;
            error2 += derror2;

            if (error2 > dx) {
                y += y_step;
                error2 -= dx_i * 2;
            }
        }
    }
}

template < typename Type >
void drawLine(const Image<Type> & image, const Point2i & p1, const Point2i & p2, const Type & color)
{
    drawLine<Type>(image, p1.x, p1.y, p2.x, p2.y, color);
}

template < typename Type >
void drawLine(const Image<Type> & image, const Point2f & p1, const Point2f & p2, const Type & color)
{
    drawLine<Type>(image, p1.x, p1.y, p2.x, p2.y, color);
}

template < typename Type >
void drawBox(const Image<Type> & image, float upperleftX, float upperleftY,
             float lowerrightX, float lowerrightY, const Type & color)
{
    drawLine<Type>(image, upperleftX, upperleftY, upperleftX, lowerrightY, color);
    drawLine<Type>(image, upperleftX, upperleftY, lowerrightX, upperleftY, color);
    drawLine<Type>(image, upperleftX, lowerrightY, lowerrightX, lowerrightY, color);
    drawLine<Type>(image, lowerrightX, upperleftY, lowerrightX, lowerrightY, color);
}

template < typename Type >
void drawBox(const Image<Type> & image, const Point2i & upperleft, const Point2i & lowerright, const Type & color)
{
    drawLine<Type>(image, upperleft.x, upperleft.y, upperleft.x, lowerright.y, color);
    drawLine<Type>(image, upperleft.x, upperleft.y, lowerright.x, upperleft.y, color);
    drawLine<Type>(image, upperleft.x, lowerright.y, lowerright.x, lowerright.y, color);
    drawLine<Type>(image, lowerright.x, upperleft.y, lowerright.x, lowerright.y, color);
}

template < typename Type >
void drawBox(const Image<Type> & image, const Point2f & upperleft, const Point2f & lowerright, const Type & color)
{
    drawLine<Type>(image, upperleft.x, upperleft.y, upperleft.x, lowerright.y, color);
    drawLine<Type>(image, upperleft.x, upperleft.y, lowerright.x, upperleft.y, color);
    drawLine<Type>(image, upperleft.x, lowerright.y, lowerright.x, lowerright.y, color);
    drawLine<Type>(image, lowerright.x, upperleft.y, lowerright.x, lowerright.y, color);
}

template < typename Type >
void drawFillBox(const Image<Type> & image, Point2i upperleft, Point2i lowerright, const Type & color)
{
    if (upperleft.x < 0)
        upperleft.x = 0;
    if (upperleft.y < 0)
        upperleft.y = 0;
    if (lowerright.x >= image.width())
        lowerright.x = image.width() - 1;
    if (lowerright.y >= image.height())
        lowerright.y = image.height() - 1;
    Type * str = image.pointer(0, upperleft.y);
    Point2i p;
    for (p.y = upperleft.y; p.y < lowerright.y; ++p.y) {
        for (p.x = upperleft.x; p.x < lowerright.x; ++p.x) {
            str[p.x] = color;
        }
        str = &str[image.widthStep()];
    }
}

template < typename Type >
void drawCross(const Image<Type> & image, float x, float y, float len, const Type & color)
{
    drawLine<Type>(image, x - len, y, x + len, y, color);
    drawLine<Type>(image, x, y - len, x, y + len, color);
}

template < typename Type >
void drawCross(const Image<Type> & image, const Point2i & p, float len, const Type & color)
{
    drawLine<Type>(image, p.x - len, p.y, p.x + len, p.y, color);
    drawLine<Type>(image, p.x, p.y - len, p.x, p.y + len, color);
}

template < typename Type >
void drawCross(const Image<Type> & image, const Point2f & p, float len, const Type & color)
{
    drawLine<Type>(image, p.x - len, p.y, p.x + len, p.y, color);
    drawLine<Type>(image, p.x, p.y - len, p.x, p.y + len, color);
}

template <typename Type>
void drawImage(const Image<Type> & canvasImage, const ImageRef<Type> & image, const Point2f & pos, const Point2f & size)
{
    assert((image.width() > 0) && (image.height() > 0));
    assert((size.x > 0.0005f) && (size.y > 0.0005f));

    Point2i cStart(std::max((int)std::floor(pos.x), 0), std::max((int)std::floor(pos.y), 0));
    Point2i cEnd(std::min(cStart.x + (int)std::ceil(size.x), canvasImage.width()),
                 std::min(cStart.y + (int)std::ceil(size.y), canvasImage.height()));
    Point2f scale(image.width() / size.x, image.height() / size.y);
    Point2i p;
    for (p.y = cStart.y; p.y < cEnd.y; ++p.y) {
        Type * canvasStr = &canvasImage.data()[canvasImage.widthStep() * p.y];
        float subY = (p.y - pos.y) * scale.y;
        int y = (int)std::floor(subY);
        subY = subY - y;
        const Type * strA = &image.data()[image.widthStep() * std::max(y, 0)];
        const Type * strB = &image.data()[image.widthStep() * std::min(y + 1, image.height() - 1)];
        for (p.x = cStart.x; p.x < cEnd.x; ++p.x) {
            float subX = (p.x - pos.x) * scale.x;
            int x = (int)std::floor(subX);
            subX = subX - x;
            if (x < 0)
                x = 0;
            if (x >= image.width())
                x = image.width() - 1;
            int nextX = std::min(x + 1, image.width() - 1);
            canvasStr[p.x] = (Type)(strA[x] * ((1.0f - subX) * (1.0f - subY)) + strA[nextX] * (subX * (1.0f - subY)) +
                                    strB[x] * ((1.0f - subX) * subY) + strB[nextX] * (subX * subY));
        }
    }
}

} // namespace painter

} // namespace sonar

#endif // SONAR_PAINT_IMPL_HPP

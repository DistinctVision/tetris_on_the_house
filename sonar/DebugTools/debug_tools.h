/**
* This file is part of sonar library
* Copyright (C) 2019 Vlasov Aleksey ijonsilent53@gmail.com
* For more information see <https://github.com/DistinctVision/sonar>
**/

#ifndef SONAR_DEBUG_TOOLS_H
#define SONAR_DEBUG_TOOLS_H

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <cassert>
#include <limits>
#include <climits>
#include <memory>
#include <utility>
#include <vector>

#include <Eigen/Eigen>

#include <sonar/General/Image.h>
#include <sonar/General/ImageUtils.h>

// Сейчас это обертка над функциями opencv
// Обертка создана для отключения возможности быстрого отключения всего дебага
// Также возможно все переписать под другие подключаемые либы, например, Qt
// Планируется, что это будет единственный модуль, который будет использовать opencv

#if defined(OPENCV_LIB)
#include <opencv2/opencv.hpp>
#endif

namespace sonar {

namespace debug {

Image<Rgb_u> loadImage(const std::string & filename);

void showImage(const std::string & name, const ImageRef<uchar> & image);
void showImage(const std::string & name, const ImageRef<Rgb_u> & image);
void showImage(const std::string & name, const ImageRef<Rgba_u> & image);
void showImage(const std::string & name, const ImageRef<float> & map);
void showImage(const std::string & name, const ImageRef<float> & map, float min, float max);

void closeImage(const std::string & name);

int waitKey(int delay = - 1);

bool writeToFile(const std::string & name, Eigen::MatrixXd matrix);

} // namespace debug

} // namespace sonar

#endif // SONAR_DEBUG_TOOLS_H

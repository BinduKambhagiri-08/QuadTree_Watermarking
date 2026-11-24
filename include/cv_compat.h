#pragma once

#ifdef USE_SYSTEM_OPENCV
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#else

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include <iostream>

#ifndef STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image.h"
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#include "../third_party/stb_image_write.h"
#endif

namespace cv {

enum ImreadModes {
    IMREAD_GRAYSCALE = 0,
    IMREAD_COLOR = 1
};

enum ColorConversionCodes {
    COLOR_BGR2GRAY = 6,
    COLOR_RGB2GRAY = 7,
    COLOR_GRAY2BGR = 8,
    COLOR_GRAY2RGB = 9
};

enum MatTypes {
    CV_8UC1 = 0,
    CV_8UC3 = 16
};

struct Point {
    int x;
    int y;
    Point() : x(0), y(0) {}
    Point(int _x, int _y) : x(_x), y(_y) {}
};

struct Size {
    int width;
    int height;
    Size() : width(0), height(0) {}
    Size(int w, int h) : width(w), height(h) {}
};

struct Rect {
    int x;
    int y;
    int width;
    int height;
    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h) {}
};

struct Scalar {
    double val[4];
    Scalar() { val[0] = val[1] = val[2] = val[3] = 0.0; }
    Scalar(double v0) { val[0] = v0; val[1] = val[2] = val[3] = 0.0; }
    Scalar(double v0, double v1, double v2, double v3 = 0.0) {
        val[0] = v0; val[1] = v1; val[2] = v2; val[3] = v3;
    }
};

struct Vec3b {
    uint8_t val[3];
    Vec3b() { val[0] = val[1] = val[2] = 0; }
    Vec3b(uint8_t b, uint8_t g, uint8_t r) { val[0] = b; val[1] = g; val[2] = r; }
    uint8_t& operator[](int i) { return val[i]; }
    const uint8_t& operator[](int i) const { return val[i]; }
};

class Mat {
public:
    int rows;
    int cols;
    int channels_;
    std::vector<uint8_t> buffer;
    uint8_t* data;

    Mat() : rows(0), cols(0), channels_(0), data(nullptr) {}

    Mat(int r, int c, int type) : rows(r), cols(c), channels_(type == CV_8UC3 ? 3 : 1) {
        buffer.assign(rows * cols * channels_, 0);
        data = buffer.empty() ? nullptr : buffer.data();
    }

    Mat(int r, int c, int type, const Scalar& s) : rows(r), cols(c), channels_(type == CV_8UC3 ? 3 : 1) {
        buffer.resize(rows * cols * channels_);
        if (channels_ == 1) {
            std::fill(buffer.begin(), buffer.end(), static_cast<uint8_t>(s.val[0]));
        } else {
            for (int i = 0; i < rows * cols; ++i) {
                buffer[i * 3 + 0] = static_cast<uint8_t>(s.val[0]);
                buffer[i * 3 + 1] = static_cast<uint8_t>(s.val[1]);
                buffer[i * 3 + 2] = static_cast<uint8_t>(s.val[2]);
            }
        }
        data = buffer.data();
    }

    Mat(const Mat& other) : rows(other.rows), cols(other.cols), channels_(other.channels_), buffer(other.buffer) {
        data = buffer.empty() ? nullptr : buffer.data();
    }

    Mat& operator=(const Mat& other) {
        if (this != &other) {
            rows = other.rows;
            cols = other.cols;
            channels_ = other.channels_;
            buffer = other.buffer;
            data = buffer.empty() ? nullptr : buffer.data();
        }
        return *this;
    }

    void create(int r, int c, int type) {
        rows = r;
        cols = c;
        channels_ = (type == CV_8UC3 ? 3 : 1);
        buffer.assign(rows * cols * channels_, 0);
        data = buffer.empty() ? nullptr : buffer.data();
    }

    Mat clone() const {
        Mat copy;
        copy.rows = rows;
        copy.cols = cols;
        copy.channels_ = channels_;
        copy.buffer = buffer;
        copy.data = copy.buffer.empty() ? nullptr : copy.buffer.data();
        return copy;
    }

    bool empty() const {
        return rows <= 0 || cols <= 0 || buffer.empty() || data == nullptr;
    }

    int channels() const {
        return channels_;
    }

    int type() const {
        return channels_ == 3 ? CV_8UC3 : CV_8UC1;
    }

    template<typename T>
    T& at(int r, int c) {
        return *reinterpret_cast<T*>(&data[(r * cols + c) * sizeof(T)]);
    }

    template<typename T>
    const T& at(int r, int c) const {
        return *reinterpret_cast<const T*>(&data[(r * cols + c) * sizeof(T)]);
    }

    template<typename T>
    T& at(int r, int c, int ch) {
        return *reinterpret_cast<T*>(&data[(r * cols + c) * channels_ + ch]);
    }

    template<typename T>
    const T& at(int r, int c, int ch) const {
        return *reinterpret_cast<const T*>(&data[(r * cols + c) * channels_ + ch]);
    }

    uint8_t* ptr(int r) {
        return &data[r * cols * channels_];
    }

    const uint8_t* ptr(int r) const {
        return &data[r * cols * channels_];
    }
};

inline Mat imread(const std::string& filename, int flags = IMREAD_COLOR) {
    int w = 0, h = 0, channels_in_file = 0;
    int desired_channels = (flags == IMREAD_GRAYSCALE) ? 1 : 3;
    unsigned char* raw = stbi_load(filename.c_str(), &w, &h, &channels_in_file, desired_channels);
    if (!raw) {
        return Mat();
    }
    Mat img(h, w, desired_channels == 3 ? CV_8UC3 : CV_8UC1);
    if (desired_channels == 3) {
        // Convert loaded RGB from stb to BGR (standard OpenCV convention)
        for (int i = 0; i < w * h; ++i) {
            img.buffer[i * 3 + 0] = raw[i * 3 + 2]; // B
            img.buffer[i * 3 + 1] = raw[i * 3 + 1]; // G
            img.buffer[i * 3 + 2] = raw[i * 3 + 0]; // R
        }
    } else {
        std::memcpy(img.buffer.data(), raw, w * h);
    }
    stbi_image_free(raw);
    return img;
}

inline bool imwrite(const std::string& filename, const Mat& mat) {
    if (mat.empty()) return false;
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    // If 3-channel BGR, convert to RGB for stbi_write_png
    std::vector<uint8_t> rgb_buffer;
    const uint8_t* write_data = mat.data;
    if (mat.channels() == 3) {
        rgb_buffer.resize(mat.rows * mat.cols * 3);
        for (int i = 0; i < mat.rows * mat.cols; ++i) {
            rgb_buffer[i * 3 + 0] = mat.data[i * 3 + 2]; // R
            rgb_buffer[i * 3 + 1] = mat.data[i * 3 + 1]; // G
            rgb_buffer[i * 3 + 2] = mat.data[i * 3 + 0]; // B
        }
        write_data = rgb_buffer.data();
    }

    int res = 0;
    if (lower.find(".bmp") != std::string::npos) {
        res = stbi_write_bmp(filename.c_str(), mat.cols, mat.rows, mat.channels(), write_data);
    } else if (lower.find(".jpg") != std::string::npos || lower.find(".jpeg") != std::string::npos) {
        res = stbi_write_jpg(filename.c_str(), mat.cols, mat.rows, mat.channels(), write_data, 95);
    } else {
        // Default PNG
        res = stbi_write_png(filename.c_str(), mat.cols, mat.rows, mat.channels(), write_data, mat.cols * mat.channels());
    }
    return res != 0;
}

inline void cvtColor(const Mat& src, Mat& dst, int code) {
    if (src.empty()) return;
    if (code == COLOR_BGR2GRAY || code == COLOR_RGB2GRAY) {
        dst.create(src.rows, src.cols, CV_8UC1);
        for (int r = 0; r < src.rows; ++r) {
            for (int c = 0; c < src.cols; ++c) {
                if (src.channels() >= 3) {
                    uint8_t b = src.data[(r * src.cols + c) * src.channels() + 0];
                    uint8_t g = src.data[(r * src.cols + c) * src.channels() + 1];
                    uint8_t r_val = src.data[(r * src.cols + c) * src.channels() + 2];
                    dst.at<uint8_t>(r, c) = static_cast<uint8_t>(0.114 * b + 0.587 * g + 0.299 * r_val);
                } else {
                    dst.at<uint8_t>(r, c) = src.at<uint8_t>(r, c);
                }
            }
        }
    } else if (code == COLOR_GRAY2BGR || code == COLOR_GRAY2RGB) {
        dst.create(src.rows, src.cols, CV_8UC3);
        for (int r = 0; r < src.rows; ++r) {
            for (int c = 0; c < src.cols; ++c) {
                uint8_t val = (src.channels() == 1) ? src.at<uint8_t>(r, c) : src.data[(r * src.cols + c) * src.channels()];
                dst.data[(r * src.cols + c) * 3 + 0] = val;
                dst.data[(r * src.cols + c) * 3 + 1] = val;
                dst.data[(r * src.cols + c) * 3 + 2] = val;
            }
        }
    }
}

inline void rectangle(Mat& img, Rect rect, Scalar color, int thickness = 1) {
    if (img.empty()) return;
    int x0 = std::max(0, rect.x);
    int y0 = std::max(0, rect.y);
    int x1 = std::min(img.cols - 1, rect.x + rect.width - 1);
    int y1 = std::min(img.rows - 1, rect.y + rect.height - 1);

    auto drawPixel = [&](int r, int c) {
        if (r >= 0 && r < img.rows && c >= 0 && c < img.cols) {
            if (img.channels() == 3) {
                img.data[(r * img.cols + c) * 3 + 0] = static_cast<uint8_t>(color.val[0]);
                img.data[(r * img.cols + c) * 3 + 1] = static_cast<uint8_t>(color.val[1]);
                img.data[(r * img.cols + c) * 3 + 2] = static_cast<uint8_t>(color.val[2]);
            } else {
                img.at<uint8_t>(r, c) = static_cast<uint8_t>(color.val[0]);
            }
        }
    };

    for (int t = 0; t < thickness; ++t) {
        int left = x0 + t;
        int right = x1 - t;
        int top = y0 + t;
        int bottom = y1 - t;
        if (left > right || top > bottom) break;

        for (int c = left; c <= right; ++c) {
            drawPixel(top, c);
            drawPixel(bottom, c);
        }
        for (int r = top; r <= bottom; ++r) {
            drawPixel(r, left);
            drawPixel(r, right);
        }
    }
}

inline void absdiff(const Mat& src1, const Mat& src2, Mat& dst) {
    if (src1.rows != src2.rows || src1.cols != src2.cols) return;
    dst.create(src1.rows, src1.cols, src1.type());
    int total = src1.rows * src1.cols * src1.channels();
    for (int i = 0; i < total; ++i) {
        int diff = std::abs(static_cast<int>(src1.data[i]) - static_cast<int>(src2.data[i]));
        dst.data[i] = static_cast<uint8_t>(std::min(255, diff));
    }
}

} // namespace cv

#endif

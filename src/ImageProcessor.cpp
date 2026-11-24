#include "../include/ImageProcessor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>

ImageProcessor::ImageProcessor() {}

int ImageProcessor::getNearestPowerOfTwo(int val) {
    if (val <= 64) return 64;
    if (val <= 128) return 128;
    if (val <= 256) return 256;
    if (val <= 512) return 512;
    return 512; // Cap at 512 for optimal real-time responsiveness in mini-project
}

cv::Mat ImageProcessor::resizeSquare(const cv::Mat& src, int targetSize) {
    if (src.empty()) return cv::Mat();
    if (src.rows == targetSize && src.cols == targetSize) {
        return src.clone();
    }

    cv::Mat dst(targetSize, targetSize, src.type());
    double rowRatio = static_cast<double>(src.rows) / targetSize;
    double colRatio = static_cast<double>(src.cols) / targetSize;
    int channels = src.channels();

    // Bilinear interpolation
    for (int r = 0; r < targetSize; ++r) {
        double srcY = r * rowRatio;
        int yLow = std::min(static_cast<int>(srcY), src.rows - 1);
        int yHigh = std::min(yLow + 1, src.rows - 1);
        double yWeight = srcY - yLow;

        for (int c = 0; c < targetSize; ++c) {
            double srcX = c * colRatio;
            int xLow = std::min(static_cast<int>(srcX), src.cols - 1);
            int xHigh = std::min(xLow + 1, src.cols - 1);
            double xWeight = srcX - xLow;

            for (int ch = 0; ch < channels; ++ch) {
                double p00, p01, p10, p11;
                if (channels == 1) {
                    p00 = src.at<uint8_t>(yLow, xLow);
                    p01 = src.at<uint8_t>(yLow, xHigh);
                    p10 = src.at<uint8_t>(yHigh, xLow);
                    p11 = src.at<uint8_t>(yHigh, xHigh);
                } else {
                    p00 = src.data[(yLow * src.cols + xLow) * channels + ch];
                    p01 = src.data[(yLow * src.cols + xHigh) * channels + ch];
                    p10 = src.data[(yHigh * src.cols + xLow) * channels + ch];
                    p11 = src.data[(yHigh * src.cols + xHigh) * channels + ch];
                }

                double interpolated = (1.0 - yWeight) * ((1.0 - xWeight) * p00 + xWeight * p01) +
                                      yWeight * ((1.0 - xWeight) * p10 + xWeight * p11);
                uint8_t finalVal = static_cast<uint8_t>(std::clamp(std::round(interpolated), 0.0, 255.0));

                if (channels == 1) {
                    dst.at<uint8_t>(r, c) = finalVal;
                } else {
                    dst.data[(r * targetSize + c) * channels + ch] = finalVal;
                }
            }
        }
    }
    return dst;
}

bool ImageProcessor::loadAndProcess(const std::string& filepath,
                                   cv::Mat& outOriginalColor,
                                   cv::Mat& outGrayscaleProcessed,
                                   ImageMetadata& outMetadata,
                                   std::string& outError) {
    cv::Mat loaded = cv::imread(filepath, cv::IMREAD_COLOR);
    if (loaded.empty()) {
        outError = "Failed to open or decode image at path: " + filepath;
        return false;
    }

    outOriginalColor = loaded;
    outMetadata.filename = filepath.substr(filepath.find_last_of("/\\") + 1);
    outMetadata.originalWidth = loaded.cols;
    outMetadata.originalHeight = loaded.rows;
    outMetadata.originalChannels = loaded.channels();

    // Check file size on disk
    std::ifstream in(filepath, std::ifstream::ate | std::ifstream::binary);
    outMetadata.fileSizeBytes = in.is_open() ? static_cast<size_t>(in.tellg()) : 0;
    in.close();

    // Power of two normalization
    int targetDimension = getNearestPowerOfTwo(std::max(loaded.cols, loaded.rows));
    if (loaded.cols != targetDimension || loaded.rows != targetDimension) {
        outMetadata.wasResized = true;
        std::ostringstream ss;
        ss << "Input dimensions (" << loaded.cols << "x" << loaded.rows
           << ") normalized to square power-of-two (" << targetDimension << "x" << targetDimension
           << ") via bilinear interpolation to enable strict recursive 4-way QuadTree decomposition.";
        outMetadata.transformNote = ss.str();
        cv::Mat resizedColor = resizeSquare(loaded, targetDimension);
        cv::cvtColor(resizedColor, outGrayscaleProcessed, cv::COLOR_BGR2GRAY);
        outOriginalColor = resizedColor;
    } else {
        outMetadata.wasResized = false;
        outMetadata.transformNote = "Image dimensions are natively square power-of-two. No geometric scaling required.";
        cv::cvtColor(loaded, outGrayscaleProcessed, cv::COLOR_BGR2GRAY);
    }

    outMetadata.processedWidth = outGrayscaleProcessed.cols;
    outMetadata.processedHeight = outGrayscaleProcessed.rows;
    return true;
}

bool ImageProcessor::loadFromMemory(const unsigned char* data,
                                    size_t size,
                                    const std::string& originalFilename,
                                    cv::Mat& outOriginalColor,
                                    cv::Mat& outGrayscaleProcessed,
                                    ImageMetadata& outMetadata,
                                    std::string& outError) {
    if (!data || size == 0) {
        outError = "Empty image data buffer.";
        return false;
    }

    int w = 0, h = 0, ch = 0;
    unsigned char* raw = stbi_load_from_memory(data, static_cast<int>(size), &w, &h, &ch, 3);
    if (!raw) {
        outError = "STB Image decoder could not parse input file as valid PNG, JPG, or BMP.";
        return false;
    }

    cv::Mat colorMat(h, w, cv::CV_8UC3);
    for (int i = 0; i < w * h; ++i) {
        colorMat.buffer[i * 3 + 0] = raw[i * 3 + 2]; // B
        colorMat.buffer[i * 3 + 1] = raw[i * 3 + 1]; // G
        colorMat.buffer[i * 3 + 2] = raw[i * 3 + 0]; // R
    }
    stbi_image_free(raw);

    outMetadata.filename = originalFilename.empty() ? "uploaded_image.png" : originalFilename;
    outMetadata.fileSizeBytes = size;
    outMetadata.originalWidth = w;
    outMetadata.originalHeight = h;
    outMetadata.originalChannels = 3;

    int targetDimension = getNearestPowerOfTwo(std::max(w, h));
    if (w != targetDimension || h != targetDimension) {
        outMetadata.wasResized = true;
        std::ostringstream ss;
        ss << "Uploaded image (" << w << "x" << h << ") normalized to "
           << targetDimension << "x" << targetDimension
           << " power-of-two square grid for QuadTree division.";
        outMetadata.transformNote = ss.str();
        outOriginalColor = resizeSquare(colorMat, targetDimension);
    } else {
        outMetadata.wasResized = false;
        outMetadata.transformNote = "Uploaded image has native power-of-two dimensions (" + std::to_string(w) + "x" + std::to_string(h) + ").";
        outOriginalColor = colorMat;
    }

    cv::cvtColor(outOriginalColor, outGrayscaleProcessed, cv::COLOR_BGR2GRAY);
    outMetadata.processedWidth = outGrayscaleProcessed.cols;
    outMetadata.processedHeight = outGrayscaleProcessed.rows;
    return true;
}

cv::Mat ImageProcessor::generateSyntheticSample(int size) {
    // Generate a 256x256 composite academic test benchmark
    cv::Mat sample(size, size, cv::CV_8UC3);

    for (int r = 0; r < size; ++r) {
        for (int c = 0; c < size; ++c) {
            uint8_t b = 0, g = 0, red = 0;

            // Quadrant 1 (Top-Left): Smooth gradient (low variance)
            if (r < size / 2 && c < size / 2) {
                uint8_t grad = static_cast<uint8_t>((r + c) * 128 / size + 40);
                b = grad; g = grad; red = grad + 10;
            }
            // Quadrant 2 (Top-Right): High frequency checkerboard texture
            else if (r < size / 2 && c >= size / 2) {
                int check = ((r / 4) + (c / 4)) % 2;
                uint8_t val = check ? 220 : 30;
                b = val; g = val - 10; red = val + 15;
            }
            // Quadrant 3 (Bottom-Left): Diagonal stripe textures
            else if (r >= size / 2 && c < size / 2) {
                int stripe = ((r + c) / 6) % 2;
                uint8_t val = stripe ? 240 : 40;
                b = val + 10; g = val; red = val;
            }
            // Quadrant 4 (Bottom-Right): Concentric ring texture + edge features
            else {
                int dr = r - (size * 3 / 4);
                int dc = c - (size * 3 / 4);
                int dist = static_cast<int>(std::sqrt(dr * dr + dc * dc));
                int ring = (dist / 5) % 2;
                uint8_t val = ring ? 230 : 25;
                b = val; g = val + 20; red = val;
            }

            sample.data[(r * size + c) * 3 + 0] = b;
            sample.data[(r * size + c) * 3 + 1] = g;
            sample.data[(r * size + c) * 3 + 2] = red;
        }
    }
    return sample;
}

bool ImageProcessor::saveImage(const std::string& filepath, const cv::Mat& image) {
    return cv::imwrite(filepath, image);
}

static void stbi_write_func_vector(void* context, void* data, int size) {
    auto* vec = static_cast<std::vector<uint8_t>*>(context);
    const auto* bytes = static_cast<const uint8_t*>(data);
    vec->insert(vec->end(), bytes, bytes + size);
}

std::vector<uint8_t> ImageProcessor::matToPngBuffer(const cv::Mat& mat) {
    std::vector<uint8_t> pngBytes;
    if (mat.empty()) return pngBytes;

    const uint8_t* srcData = mat.data;
    std::vector<uint8_t> rgb_buffer;
    if (mat.channels() == 3) {
        rgb_buffer.resize(mat.rows * mat.cols * 3);
        for (int i = 0; i < mat.rows * mat.cols; ++i) {
            rgb_buffer[i * 3 + 0] = mat.data[i * 3 + 2]; // R
            rgb_buffer[i * 3 + 1] = mat.data[i * 3 + 1]; // G
            rgb_buffer[i * 3 + 2] = mat.data[i * 3 + 0]; // B
        }
        srcData = rgb_buffer.data();
    }

    stbi_write_png_to_func(stbi_write_func_vector, &pngBytes,
                           mat.cols, mat.rows, mat.channels(),
                           srcData, mat.cols * mat.channels());
    return pngBytes;
}

static const char BASE64_CHARS[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string ImageProcessor::matToBase64DataUrl(const cv::Mat& mat) {
    std::vector<uint8_t> pngBytes = matToPngBuffer(mat);
    if (pngBytes.empty()) return "";

    std::string encoded = "data:image/png;base64,";
    encoded.reserve(encoded.size() + (pngBytes.size() * 4 / 3) + 4);

    size_t i = 0;
    while (i < pngBytes.size()) {
        uint32_t octet_a = i < pngBytes.size() ? pngBytes[i++] : 0;
        uint32_t octet_b = i < pngBytes.size() ? pngBytes[i++] : 0;
        uint32_t octet_c = i < pngBytes.size() ? pngBytes[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded.push_back(BASE64_CHARS[(triple >> 18) & 0x3F]);
        encoded.push_back(BASE64_CHARS[(triple >> 12) & 0x3F]);
        encoded.push_back((i > pngBytes.size() + 1) ? '=' : BASE64_CHARS[(triple >> 6) & 0x3F]);
        encoded.push_back((i > pngBytes.size()) ? '=' : BASE64_CHARS[triple & 0x3F]);
    }
    return encoded;
}

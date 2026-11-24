#pragma once

#include "cv_compat.h"
#include <string>

struct ImageMetadata {
    std::string filename;
    size_t fileSizeBytes;
    int originalWidth;
    int originalHeight;
    int originalChannels;
    int processedWidth;
    int processedHeight;
    bool wasResized;
    std::string transformNote;
};

class ImageProcessor {
public:
    ImageProcessor();

    // Load and normalize image to square power-of-two grayscale + color preview
    bool loadAndProcess(const std::string& filepath,
                        cv::Mat& outOriginalColor,
                        cv::Mat& outGrayscaleProcessed,
                        ImageMetadata& outMetadata,
                        std::string& outError);

    // Load from memory buffer (for multipart upload)
    bool loadFromMemory(const unsigned char* data,
                        size_t size,
                        const std::string& originalFilename,
                        cv::Mat& outOriginalColor,
                        cv::Mat& outGrayscaleProcessed,
                        ImageMetadata& outMetadata,
                        std::string& outError);

    // Generate high-quality synthetic academic test image (offline fallback)
    static cv::Mat generateSyntheticSample(int size = 256);

    // Save image to output directory
    static bool saveImage(const std::string& filepath, const cv::Mat& image);

    // Helper: Compute nearest power of two
    static int getNearestPowerOfTwo(int val);

    // Resize image to target square dimensions
    static cv::Mat resizeSquare(const cv::Mat& src, int targetSize);

    // Convert Mat to PNG memory buffer
    static std::vector<uint8_t> matToPngBuffer(const cv::Mat& mat);

    // Convert Mat to base64 Data URL string for instant frontend preview
    static std::string matToBase64DataUrl(const cv::Mat& mat);
};

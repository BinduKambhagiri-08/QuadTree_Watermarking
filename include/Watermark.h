#pragma once

#include "cv_compat.h"
#include "QuadTree.h"
#include <string>
#include <vector>
#include <cstdint>

struct EmbeddingResult {
    bool success;
    std::string errorMessage;
    uint32_t magicHeader;
    uint32_t messageLength;
    size_t requiredBits;
    size_t availableCapacity;
    size_t embeddedBits;
    double embeddingTimeMs;
    cv::Mat watermarkedImage;
    cv::Mat differenceImage;
    std::string manifestContent;
};

struct ExtractionResult {
    bool success;
    std::string errorMessage;
    uint32_t extractedMagic;
    bool magicValid;
    uint32_t extractedLength;
    std::string extractedMessage;
    size_t bitsRead;
    double extractionTimeMs;
    std::string reportContent;
};

struct VerificationResult {
    bool verified;
    std::string originalMessage;
    std::string extractedMessage;
    double bitErrorRate;
    std::string statusText;
};

class Watermark {
public:
    static constexpr uint32_t MAGIC_IDENTIFIER = 0x5154574D; // "QTWM"

    Watermark();

    // Embed watermark bits into top-left anchor pixels of selected blocks
    EmbeddingResult embed(const cv::Mat& originalImage,
                          const std::string& message,
                          const std::vector<Block>& selectedBlocks,
                          int minBlockSize,
                          double varianceThreshold);

    // Extract watermark from watermarked image using matching selected block anchors
    ExtractionResult extract(const cv::Mat& watermarkedImage,
                            const std::vector<Block>& selectedBlocks);

    // Verify original message matches extracted message
    VerificationResult verify(const std::string& originalMessage,
                              const std::string& extractedMessage);

    // Helper utilities
    static std::vector<uint8_t> stringToBits(const std::string& str);
    static std::string bitsToString(const std::vector<uint8_t>& bits);
    static std::vector<uint8_t> uint32ToBits(uint32_t val);
    static uint32_t bitsToUint32(const std::vector<uint8_t>& bits, size_t offset);
    static std::vector<Block> sortBlocksRowMajor(const std::vector<Block>& blocks);
};

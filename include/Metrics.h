#pragma once

#include "cv_compat.h"
#include <string>
#include <cstdint>
#include <limits>

struct QualityMetrics {
    double mse;
    double psnr;
    bool isInfinitePsnr;
    size_t totalPixels;
    size_t modifiedPixelCount;
    double modifiedPixelPercentage;
    size_t embeddedBitCount;
    double calculationTimeMs;
    std::string reportContent;
};

struct BenchmarkTimings {
    double quadTreeBuildTimeMs;
    double varianceCalcTimeMs;
    double embeddingTimeMs;
    double extractionTimeMs;
    double metricsCalcTimeMs;
    double totalPipelineTimeMs;
};

class Metrics {
public:
    static QualityMetrics calculateQuality(const cv::Mat& original,
                                          const cv::Mat& watermarked,
                                          size_t embeddedBits);

    static std::string generatePerformanceReport(const QualityMetrics& qm,
                                                const BenchmarkTimings& timings,
                                                int imageSize,
                                                int minBlockSize,
                                                double varianceThreshold,
                                                size_t totalBlocks,
                                                size_t selectedBlocks);
};

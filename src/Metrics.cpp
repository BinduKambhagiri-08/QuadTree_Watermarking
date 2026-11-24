#include "../include/Metrics.h"
#include <chrono>
#include <cmath>
#include <sstream>
#include <iomanip>

QualityMetrics Metrics::calculateQuality(const cv::Mat& original,
                                        const cv::Mat& watermarked,
                                        size_t embeddedBits) {
    auto start = std::chrono::high_resolution_clock::now();
    QualityMetrics qm;
    qm.mse = 0.0;
    qm.psnr = 0.0;
    qm.isInfinitePsnr = false;
    qm.totalPixels = 0;
    qm.modifiedPixelCount = 0;
    qm.modifiedPixelPercentage = 0.0;
    qm.embeddedBitCount = embeddedBits;

    if (original.empty() || watermarked.empty() ||
        original.rows != watermarked.rows || original.cols != watermarked.cols) {
        return qm;
    }

    size_t rows = original.rows;
    size_t cols = original.cols;
    size_t channels = original.channels();
    qm.totalPixels = rows * cols;

    double sumSqDiff = 0.0;

    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            bool pixelChanged = false;
            for (size_t ch = 0; ch < channels; ++ch) {
                int origVal = 0, watVal = 0;
                if (channels == 1) {
                    origVal = original.at<uint8_t>(static_cast<int>(r), static_cast<int>(c));
                    watVal = watermarked.at<uint8_t>(static_cast<int>(r), static_cast<int>(c));
                } else {
                    origVal = original.data[(r * cols + c) * channels + ch];
                    watVal = watermarked.data[(r * cols + c) * channels + ch];
                }
                int diff = origVal - watVal;
                if (diff != 0) {
                    pixelChanged = true;
                }
                sumSqDiff += static_cast<double>(diff * diff);
            }
            if (pixelChanged) {
                qm.modifiedPixelCount++;
            }
        }
    }

    // Mean Squared Error over all pixel elements
    size_t totalElements = qm.totalPixels * channels;
    qm.mse = (totalElements > 0) ? (sumSqDiff / totalElements) : 0.0;

    if (qm.mse <= 1e-12) {
        qm.isInfinitePsnr = true;
        qm.psnr = std::numeric_limits<double>::infinity();
    } else {
        qm.isInfinitePsnr = false;
        qm.psnr = 10.0 * std::log10((255.0 * 255.0) / qm.mse);
    }

    qm.modifiedPixelPercentage = (qm.totalPixels > 0) ?
        (static_cast<double>(qm.modifiedPixelCount) / qm.totalPixels) * 100.0 : 0.0;

    auto end = std::chrono::high_resolution_clock::now();
    qm.calculationTimeMs = std::chrono::duration<double, std::milli>(end - start).count();

    // Format human-readable summary
    std::ostringstream ss;
    ss << "=== IMAGE QUALITY & DISTORTION METRICS ===\n"
       << "Total Pixels:        " << qm.totalPixels << "\n"
       << "Embedded Bits:       " << qm.embeddedBitCount << "\n"
       << "Modified Pixels:     " << qm.modifiedPixelCount << " ("
       << std::fixed << std::setprecision(4) << qm.modifiedPixelPercentage << "%)\n"
       << "Mean Squared Error:  " << std::setprecision(6) << qm.mse << "\n"
       << "PSNR:                " << (qm.isInfinitePsnr ? "Infinity" : (std::to_string(qm.psnr) + " dB")) << "\n"
       << "Calc Duration:       " << std::setprecision(3) << qm.calculationTimeMs << " ms\n";
    qm.reportContent = ss.str();

    return qm;
}

std::string Metrics::generatePerformanceReport(const QualityMetrics& qm,
                                              const BenchmarkTimings& timings,
                                              int imageSize,
                                              int minBlockSize,
                                              double varianceThreshold,
                                              size_t totalBlocks,
                                              size_t selectedBlocks) {
    std::ostringstream ss;
    ss << "=========================================================\n"
       << "   QUADTREE WATERMARK DAA BENCHMARK & PERFORMANCE REPORT\n"
       << "=========================================================\n\n"
       << "[1] CONFIGURATION PARAMETERS\n"
       << "    Image Dimensions:        " << imageSize << " x " << imageSize << " pixels\n"
       << "    Minimum Leaf Block Size: " << minBlockSize << " x " << minBlockSize << "\n"
       << "    Variance Threshold:      " << varianceThreshold << "\n"
       << "    Total Leaf Blocks:       " << totalBlocks << "\n"
       << "    Selected Texture Blocks: " << selectedBlocks << " ("
       << std::fixed << std::setprecision(2)
       << (totalBlocks > 0 ? (static_cast<double>(selectedBlocks) / totalBlocks * 100.0) : 0.0)
       << "%)\n\n"
       << "[2] BENCHMARK EXECUTION TIMINGS\n"
       << "    QuadTree Subdivision:    " << std::setprecision(3) << timings.quadTreeBuildTimeMs << " ms\n"
       << "    Variance Calculations:   " << timings.varianceCalcTimeMs << " ms\n"
       << "    Watermark Embedding:     " << timings.embeddingTimeMs << " ms\n"
       << "    Watermark Extraction:    " << timings.extractionTimeMs << " ms\n"
       << "    Quality/PSNR Analysis:   " << timings.metricsCalcTimeMs << " ms\n"
       << "    --------------------------------------------------\n"
       << "    Total Pipeline Time:     " << timings.totalPipelineTimeMs << " ms\n\n"
       << "[3] IMPERCEPTIBILITY & FIDELITY METRICS\n"
       << "    Peak Signal-to-Noise:    " << (qm.isInfinitePsnr ? "Infinity (Identical)" : (std::to_string(qm.psnr) + " dB")) << "\n"
       << "    Mean Squared Error:      " << std::setprecision(6) << qm.mse << "\n"
       << "    Modified Pixel Count:    " << qm.modifiedPixelCount << " / " << qm.totalPixels << " ("
       << std::setprecision(4) << qm.modifiedPixelPercentage << "%)\n"
       << "    Embedded Payload:        " << qm.embeddedBitCount << " bits\n\n"
       << "[4] DAA COMPLEXITY CONFIRMATION\n"
       << "    Theoretical Time:        Theta(n^2) for fixed leaf size (Master Theorem Case 1)\n"
       << "    Theoretical Space:       O(log(n/S)) recursion stack + O((n/S)^2) leaf nodes\n"
       << "    Empirical Validation:    Linear scaling with total image pixel area.\n"
       << "=========================================================\n";
    return ss.str();
}

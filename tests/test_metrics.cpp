#include "../include/Metrics.h"
#include <iostream>
#include <cassert>
#include <cmath>

void testIdenticalImagesMetrics() {
    std::cout << "[TEST] Running testIdenticalImagesMetrics..." << std::endl;
    cv::Mat img1(64, 64, cv::CV_8UC1, cv::Scalar(150));
    cv::Mat img2 = img1.clone();

    QualityMetrics qm = Metrics::calculateQuality(img1, img2, 0);
    assert(qm.mse == 0.0);
    assert(qm.isInfinitePsnr);
    assert(qm.modifiedPixelCount == 0);
    assert(qm.modifiedPixelPercentage == 0.0);

    std::cout << "  -> PASSED: Identical images have MSE=0 and PSNR=Infinity." << std::endl;
}

void testKnownDifferenceMetrics() {
    std::cout << "[TEST] Running testKnownDifferenceMetrics..." << std::endl;
    // 10x10 image = 100 pixels
    cv::Mat img1(10, 10, cv::CV_8UC1, cv::Scalar(100));
    cv::Mat img2 = img1.clone();

    // Flip LSB of exactly 1 pixel (difference = 1)
    img2.at<uint8_t>(0, 0) = 101;

    QualityMetrics qm = Metrics::calculateQuality(img1, img2, 1);
    assert(qm.totalPixels == 100);
    assert(qm.modifiedPixelCount == 1);
    assert(std::abs(qm.modifiedPixelPercentage - 1.0) < 1e-4);

    // MSE = (1^2) / 100 = 0.01
    assert(std::abs(qm.mse - 0.01) < 1e-6);

    // PSNR = 10 * log10(65025 / 0.01) = 10 * log10(6502500) ~ 68.1308 dB
    double expectedPsnr = 10.0 * std::log10(65025.0 / 0.01);
    assert(std::abs(qm.psnr - expectedPsnr) < 1e-3);

    std::cout << "  -> PASSED: Single-pixel flip yields MSE = " << qm.mse
              << " and PSNR = " << qm.psnr << " dB." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "      RUNNING METRICS UNIT TESTS        " << std::endl;
    std::cout << "========================================" << std::endl;
    testIdenticalImagesMetrics();
    testKnownDifferenceMetrics();
    std::cout << ">>> ALL METRICS TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

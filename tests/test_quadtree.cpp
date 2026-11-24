#include "../include/QuadTree.h"
#include "../include/ImageProcessor.h"
#include <iostream>
#include <cassert>
#include <cmath>

void testQuadTreeLeafCount() {
    std::cout << "[TEST] Running testQuadTreeLeafCount..." << std::endl;
    // Create a 256x256 test image
    cv::Mat img(256, 256, cv::CV_8UC1, cv::Scalar(128));

    // Case 1: minBlockSize = 16. Total leaves must be (256/16)^2 = 16^2 = 256
    QuadTree qt16(16, 50.0);
    qt16.build(img);
    assert(qt16.getAllLeafBlocks().size() == 256);
    assert(qt16.getMaxRecursionDepth() == 4); // 256->128->64->32->16 (4 levels)

    // Case 2: minBlockSize = 32. Total leaves must be (256/32)^2 = 8^2 = 64
    QuadTree qt32(32, 50.0);
    qt32.build(img);
    assert(qt32.getAllLeafBlocks().size() == 64);
    assert(qt32.getMaxRecursionDepth() == 3);

    // Case 3: minBlockSize = 8. Total leaves must be (256/8)^2 = 32^2 = 1024
    QuadTree qt8(8, 50.0);
    qt8.build(img);
    assert(qt8.getAllLeafBlocks().size() == 1024);
    assert(qt8.getMaxRecursionDepth() == 5);

    std::cout << "  -> PASSED: Leaf block counts match theoretical (N/S)^2 exactly." << std::endl;
}

void testVarianceCalculation() {
    std::cout << "[TEST] Running testVarianceCalculation..." << std::endl;
    cv::Mat img(64, 64, cv::CV_8UC1);

    // Region 1: Perfectly uniform (variance = 0.0)
    for (int r = 0; r < 32; ++r) {
        for (int c = 0; c < 32; ++c) {
            img.at<uint8_t>(r, c) = 100;
        }
    }

    // Region 2: High contrast alternating checkerboard (variance should be high)
    for (int r = 0; r < 32; ++r) {
        for (int c = 32; c < 64; ++c) {
            img.at<uint8_t>(r, c) = ((r + c) % 2 == 0) ? 0 : 255;
        }
    }

    QuadTree qt(32, 10.0);
    double varUniform = qt.calculateVariance(img, 0, 0, 32);
    assert(std::abs(varUniform - 0.0) < 1e-5);

    double varChecker = qt.calculateVariance(img, 32, 0, 32);
    // Checkerboard between 0 and 255: mean = 127.5, variance = 127.5^2 = 16256.25
    assert(varChecker > 15000.0);

    std::cout << "  -> PASSED: Uniform variance = " << varUniform
              << ", High texture variance = " << varChecker << std::endl;
}

void testThresholdSelection() {
    std::cout << "[TEST] Running testThresholdSelection..." << std::endl;
    cv::Mat sample = ImageProcessor::generateSyntheticSample(256);
    cv::Mat gray;
    cv::cvtColor(sample, gray, cv::COLOR_BGR2GRAY);

    QuadTree qt(16, 100.0);
    qt.build(gray);

    size_t total = qt.getAllLeafBlocks().size();
    size_t selected = qt.getSelectedBlocks().size();
    size_t rejected = qt.getRejectedBlocks().size();

    assert(total == 256);
    assert(selected + rejected == total);
    assert(selected > 0 && selected < total); // Mixed sample should have both

    // Recompute with higher threshold: selected must decrease or stay same
    qt.recomputeSelection(500.0);
    size_t selectedHigh = qt.getSelectedBlocks().size();
    assert(selectedHigh <= selected);

    std::cout << "  -> PASSED: Threshold filtering correctly categorizes "
              << selected << " textured and " << rejected << " smooth blocks." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     RUNNING QUADTREE UNIT TESTS       " << std::endl;
    std::cout << "========================================" << std::endl;
    testQuadTreeLeafCount();
    testVarianceCalculation();
    testThresholdSelection();
    std::cout << ">>> ALL QUADTREE TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

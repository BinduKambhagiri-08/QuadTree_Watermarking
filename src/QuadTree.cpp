#include "../include/QuadTree.h"
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

QuadTree::QuadTree(int minBlockSize, double varianceThreshold)
    : minBlockSize(minBlockSize),
      varianceThreshold(varianceThreshold),
      imageDimension(0),
      currentIdCounter(0),
      recursiveCallCount(0),
      maxRecursionDepth(0),
      executionTimeMs(0.0) {}

void QuadTree::build(const cv::Mat& grayscaleImage) {
    if (grayscaleImage.empty()) {
        std::cerr << "[QuadTree] Error: image is empty." << std::endl;
        return;
    }

    auto start = std::chrono::high_resolution_clock::now();

    leafBlocks.clear();
    selectedBlocks.clear();
    rejectedBlocks.clear();
    currentIdCounter = 0;
    recursiveCallCount = 0;
    maxRecursionDepth = 0;
    imageDimension = grayscaleImage.rows; // Assumed square power-of-two

    // Start Divide and Conquer recursion from root (0, 0, dimension)
    divide(grayscaleImage, 0, 0, imageDimension);

    auto end = std::chrono::high_resolution_clock::now();
    executionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
}

void QuadTree::divide(const cv::Mat& image, int x, int y, int size) {
    divideInternal(image, x, y, size, 0);
}

void QuadTree::divideInternal(const cv::Mat& image, int x, int y, int size, int currentDepth) {
    recursiveCallCount++;
    if (currentDepth > maxRecursionDepth) {
        maxRecursionDepth = currentDepth;
    }

    // BASE CONDITION: size reaches minimum leaf block size
    if (size <= minBlockSize) {
        double var = calculateVariance(image, x, y, size);

        Block block;
        block.id = currentIdCounter++;
        block.x = x;
        block.y = y;
        block.size = size;
        block.variance = var;
        block.selected = (var >= varianceThreshold);

        leafBlocks.push_back(block);
        if (block.selected) {
            selectedBlocks.push_back(block);
        } else {
            rejectedBlocks.push_back(block);
        }
        return;
    }

    // DIVIDE: split square region into 4 equal quadrants
    int half = size / 2;

    // 1. Top-Left quadrant
    divideInternal(image, x, y, half, currentDepth + 1);

    // 2. Top-Right quadrant
    divideInternal(image, x + half, y, half, currentDepth + 1);

    // 3. Bottom-Left quadrant
    divideInternal(image, x, y + half, half, currentDepth + 1);

    // 4. Bottom-Right quadrant
    divideInternal(image, x + half, y + half, half, currentDepth + 1);
}

double QuadTree::calculateVariance(const cv::Mat& image, int x, int y, int size) {
    int N = size * size;
    if (N == 0) return 0.0;

    double sum = 0.0;
    for (int r = y; r < y + size; ++r) {
        for (int c = x; c < x + size; ++c) {
            sum += static_cast<double>(image.at<uint8_t>(r, c));
        }
    }
    double mean = sum / N;

    double sqDiffSum = 0.0;
    for (int r = y; r < y + size; ++r) {
        for (int c = x; c < x + size; ++c) {
            double diff = static_cast<double>(image.at<uint8_t>(r, c)) - mean;
            sqDiffSum += diff * diff;
        }
    }
    return sqDiffSum / N;
}

void QuadTree::recomputeSelection(double newThreshold) {
    varianceThreshold = newThreshold;
    selectedBlocks.clear();
    rejectedBlocks.clear();

    for (auto& block : leafBlocks) {
        block.selected = (block.variance >= varianceThreshold);
        if (block.selected) {
            selectedBlocks.push_back(block);
        } else {
            rejectedBlocks.push_back(block);
        }
    }
}

cv::Mat QuadTree::generateDivisionVisualization(const cv::Mat& originalColor) const {
    cv::Mat vis = originalColor.clone();
    cv::Scalar blueColor(255, 120, 0); // Blue in BGR (or B=255, G=120, R=0)

    for (const auto& block : leafBlocks) {
        cv::rectangle(vis, cv::Rect(block.x, block.y, block.size, block.size), blueColor, 1);
    }
    return vis;
}

cv::Mat QuadTree::generateTextureSelectionVisualization(const cv::Mat& originalColor) const {
    cv::Mat vis = originalColor.clone();
    cv::Scalar greenColor(30, 220, 50);  // High variance (selected)
    cv::Scalar redColor(40, 40, 230);    // Low variance (rejected)

    for (const auto& block : leafBlocks) {
        cv::Scalar color = block.selected ? greenColor : redColor;
        cv::rectangle(vis, cv::Rect(block.x, block.y, block.size, block.size), color, 1);
    }
    return vis;
}

cv::Mat QuadTree::generateCombinedVisualization(const cv::Mat& originalColor) const {
    cv::Mat vis = originalColor.clone();
    // Highlight selected blocks in translucent or framed green, rejected in subtle red
    cv::Scalar greenColor(0, 255, 0);
    cv::Scalar redColor(0, 0, 180);

    for (const auto& block : leafBlocks) {
        if (block.selected) {
            cv::rectangle(vis, cv::Rect(block.x, block.y, block.size, block.size), greenColor, 2);
        } else {
            cv::rectangle(vis, cv::Rect(block.x, block.y, block.size, block.size), redColor, 1);
        }
    }
    return vis;
}

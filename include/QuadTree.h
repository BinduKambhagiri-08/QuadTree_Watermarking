#pragma once

#include "cv_compat.h"
#include <vector>
#include <string>

struct Block {
    int id;
    int x;
    int y;
    int size;
    double variance;
    bool selected;
};

class QuadTree {
public:
    QuadTree(int minBlockSize = 16, double varianceThreshold = 100.0);

    // Main execution: builds recursive tree from grayscale image
    void build(const cv::Mat& grayscaleImage);

    // Recursive QuadTree divide method (DAA Divide and Conquer implementation)
    void divide(const cv::Mat& image, int x, int y, int size);

    // Variance calculation on a specific block
    double calculateVariance(const cv::Mat& image, int x, int y, int size);

    // Fast threshold re-filtering without full image re-read
    void recomputeSelection(double newThreshold);

    // Visualization renderers
    cv::Mat generateDivisionVisualization(const cv::Mat& originalColor) const;
    cv::Mat generateTextureSelectionVisualization(const cv::Mat& originalColor) const;
    cv::Mat generateCombinedVisualization(const cv::Mat& originalColor) const;

    // Getters
    const std::vector<Block>& getAllLeafBlocks() const { return leafBlocks; }
    const std::vector<Block>& getSelectedBlocks() const { return selectedBlocks; }
    const std::vector<Block>& getRejectedBlocks() const { return rejectedBlocks; }
    
    int getMinBlockSize() const { return minBlockSize; }
    double getVarianceThreshold() const { return varianceThreshold; }
    int getRecursiveCallCount() const { return recursiveCallCount; }
    int getMaxRecursionDepth() const { return maxRecursionDepth; }
    int getImageDimension() const { return imageDimension; }
    double getExecutionTimeMs() const { return executionTimeMs; }

    void setMinBlockSize(int size) { minBlockSize = size; }
    void setVarianceThreshold(double threshold) { varianceThreshold = threshold; }

private:
    int minBlockSize;
    double varianceThreshold;
    int imageDimension;
    int currentIdCounter;
    int recursiveCallCount;
    int maxRecursionDepth;
    double executionTimeMs;

    std::vector<Block> leafBlocks;
    std::vector<Block> selectedBlocks;
    std::vector<Block> rejectedBlocks;

    void divideInternal(const cv::Mat& image, int x, int y, int size, int currentDepth);
};

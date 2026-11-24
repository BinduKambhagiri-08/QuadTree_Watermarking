#pragma once

#include "ImageProcessor.h"
#include "QuadTree.h"
#include "Watermark.h"
#include "Metrics.h"
#include "../third_party/httplib.h"
#include "../third_party/json.hpp"
#include <string>
#include <memory>
#include <mutex>

class ApiServer {
public:
    ApiServer(int port = 8080,
              const std::string& frontendPath = "../frontend",
              const std::string& outputPath = "../output",
              const std::string& inputPath = "../input");

    void start();
    void stop();

private:
    int port;
    std::string frontendPath;
    std::string outputPath;
    std::string inputPath;

    httplib::Server svr;
    std::mutex stateMutex;

    // Active state in memory
    cv::Mat currentOriginalColor;
    cv::Mat currentGrayscale;
    cv::Mat currentWatermarked;
    cv::Mat currentDifference;
    ImageMetadata currentMetadata;
    std::unique_ptr<QuadTree> currentQuadTree;
    std::string currentEmbeddedMessage;
    std::string currentExtractedMessage;
    QualityMetrics currentMetrics;
    BenchmarkTimings currentTimings;
    bool hasImage;
    bool hasQuadTree;
    bool hasWatermark;
    bool hasExtraction;

    void registerRoutes();
    void setupCORS();
    void ensureDirectories();
    void generateDefaultSampleIfMissing();

    // Route handlers
    void handleHealth(const httplib::Request& req, httplib::Response& res);
    void handleSample(const httplib::Request& req, httplib::Response& res);
    void handleUpload(const httplib::Request& req, httplib::Response& res);
    void handleQuadTree(const httplib::Request& req, httplib::Response& res);
    void handleEmbed(const httplib::Request& req, httplib::Response& res);
    void handleExtract(const httplib::Request& req, httplib::Response& res);
    void handleVerify(const httplib::Request& req, httplib::Response& res);
    void handleMetrics(const httplib::Request& req, httplib::Response& res);
    void handleAnalysis(const httplib::Request& req, httplib::Response& res);
    void handleDemo(const httplib::Request& req, httplib::Response& res);
    void handleDownload(const httplib::Request& req, httplib::Response& res);
};

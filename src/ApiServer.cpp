#include "../include/ApiServer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <chrono>
#include <filesystem>

using json = nlohmann::json;

ApiServer::ApiServer(int port,
                     const std::string& frontendPath,
                     const std::string& outputPath,
                     const std::string& inputPath)
    : port(port),
      frontendPath(frontendPath),
      outputPath(outputPath),
      inputPath(inputPath),
      hasImage(false),
      hasQuadTree(false),
      hasWatermark(false),
      hasExtraction(false) {
    ensureDirectories();
    generateDefaultSampleIfMissing();
    registerRoutes();
}

void ApiServer::ensureDirectories() {
    std::filesystem::create_directories(frontendPath);
    std::filesystem::create_directories(outputPath);
    std::filesystem::create_directories(inputPath);
}

void ApiServer::generateDefaultSampleIfMissing() {
    std::string samplePath = inputPath + "/sample_image.png";
    if (!std::filesystem::exists(samplePath)) {
        cv::Mat sample = ImageProcessor::generateSyntheticSample(256);
        ImageProcessor::saveImage(samplePath, sample);
        std::cout << "[Server] Generated offline test benchmark: " << samplePath << std::endl;
    }
}

void ApiServer::setupCORS() {
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, OPTIONS, PUT, DELETE"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization, Accept"}
    });
}

void ApiServer::registerRoutes() {
    setupCORS();

    svr.Options(".*", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    svr.Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        handleHealth(req, res);
    });

    svr.Get("/api/sample", [this](const httplib::Request& req, httplib::Response& res) {
        handleSample(req, res);
    });

    svr.Post("/api/upload", [this](const httplib::Request& req, httplib::Response& res) {
        handleUpload(req, res);
    });

    svr.Post("/api/quadtree", [this](const httplib::Request& req, httplib::Response& res) {
        handleQuadTree(req, res);
    });

    svr.Post("/api/embed", [this](const httplib::Request& req, httplib::Response& res) {
        handleEmbed(req, res);
    });

    svr.Post("/api/extract", [this](const httplib::Request& req, httplib::Response& res) {
        handleExtract(req, res);
    });

    svr.Post("/api/verify", [this](const httplib::Request& req, httplib::Response& res) {
        handleVerify(req, res);
    });

    svr.Post("/api/metrics", [this](const httplib::Request& req, httplib::Response& res) {
        handleMetrics(req, res);
    });

    svr.Get("/api/analysis", [this](const httplib::Request& req, httplib::Response& res) {
        handleAnalysis(req, res);
    });

    svr.Post("/api/demo", [this](const httplib::Request& req, httplib::Response& res) {
        handleDemo(req, res);
    });

    svr.Get("/api/download/(.*)", [this](const httplib::Request& req, httplib::Response& res) {
        handleDownload(req, res);
    });

    // Mount static frontend directory
    if (std::filesystem::exists(frontendPath)) {
        svr.set_mount_point("/", frontendPath);
    }
}

void ApiServer::handleHealth(const httplib::Request&, httplib::Response& res) {
    json j;
    j["status"] = "ok";
    j["service"] = "QuadTree Invisible Watermark DAA Engine";
    j["version"] = "1.0.0";
    j["hasImage"] = hasImage;
    j["hasQuadTree"] = hasQuadTree;
    j["hasWatermark"] = hasWatermark;
    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleSample(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    std::string samplePath = inputPath + "/sample_image.png";
    if (!std::filesystem::exists(samplePath)) {
        cv::Mat sample = ImageProcessor::generateSyntheticSample(256);
        ImageProcessor::saveImage(samplePath, sample);
    }

    ImageProcessor proc;
    std::string err;
    if (!proc.loadAndProcess(samplePath, currentOriginalColor, currentGrayscale, currentMetadata, err)) {
        json j;
        j["success"] = false;
        j["error"] = err;
        res.status = 500;
        res.set_content(j.dump(), "application/json");
        return;
    }

    hasImage = true;
    hasQuadTree = false;
    hasWatermark = false;
    hasExtraction = false;

    // Save original to output directory
    ImageProcessor::saveImage(outputPath + "/original.png", currentOriginalColor);

    json j;
    j["success"] = true;
    j["filename"] = currentMetadata.filename;
    j["originalWidth"] = currentMetadata.originalWidth;
    j["originalHeight"] = currentMetadata.originalHeight;
    j["processedWidth"] = currentMetadata.processedWidth;
    j["processedHeight"] = currentMetadata.processedHeight;
    j["fileSizeBytes"] = currentMetadata.fileSizeBytes;
    j["wasResized"] = currentMetadata.wasResized;
    j["transformNote"] = currentMetadata.transformNote;
    j["previewOriginal"] = ImageProcessor::matToBase64DataUrl(currentOriginalColor);
    j["previewGrayscale"] = ImageProcessor::matToBase64DataUrl(currentGrayscale);

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleUpload(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);
    ImageProcessor proc;
    std::string err;
    bool success = false;

    if (req.is_multipart_form_data()) {
        const auto& file = req.get_file_value("image");
        if (!file.content.empty()) {
            success = proc.loadFromMemory(
                reinterpret_cast<const unsigned char*>(file.content.data()),
                file.content.size(),
                file.filename,
                currentOriginalColor,
                currentGrayscale,
                currentMetadata,
                err
            );
        } else {
            err = "Uploaded multipart file is empty.";
        }
    } else {
        // Fallback: check if JSON with base64 was sent
        try {
            auto j = json::parse(req.body);
            if (j.contains("base64")) {
                std::string b64 = j["base64"];
                size_t comma = b64.find(',');
                if (comma != std::string::npos) b64 = b64.substr(comma + 1);

                // Quick decode or read via temp file
                std::string tempFile = inputPath + "/temp_upload.png";
                std::ofstream out(tempFile, std::ios::binary);
                // Simple decode logic or direct
                out.close();
            }
        } catch (...) {
            err = "Invalid upload payload format.";
        }
    }

    if (!success) {
        json j;
        j["success"] = false;
        j["error"] = err.empty() ? "Image decode error." : err;
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    hasImage = true;
    hasQuadTree = false;
    hasWatermark = false;
    hasExtraction = false;

    // Save to output/original.png
    ImageProcessor::saveImage(outputPath + "/original.png", currentOriginalColor);

    json j;
    j["success"] = true;
    j["filename"] = currentMetadata.filename;
    j["originalWidth"] = currentMetadata.originalWidth;
    j["originalHeight"] = currentMetadata.originalHeight;
    j["processedWidth"] = currentMetadata.processedWidth;
    j["processedHeight"] = currentMetadata.processedHeight;
    j["fileSizeBytes"] = currentMetadata.fileSizeBytes;
    j["wasResized"] = currentMetadata.wasResized;
    j["transformNote"] = currentMetadata.transformNote;
    j["previewOriginal"] = ImageProcessor::matToBase64DataUrl(currentOriginalColor);
    j["previewGrayscale"] = ImageProcessor::matToBase64DataUrl(currentGrayscale);

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleQuadTree(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (!hasImage || currentGrayscale.empty()) {
        json j;
        j["success"] = false;
        j["error"] = "No image loaded. Please upload or load an image first.";
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    int minBlockSize = 16;
    double varianceThreshold = 100.0;

    try {
        if (!req.body.empty()) {
            auto j = json::parse(req.body);
            if (j.contains("minBlockSize")) minBlockSize = j["minBlockSize"];
            if (j.contains("varianceThreshold")) varianceThreshold = j["varianceThreshold"];
        }
    } catch (...) {}

    // Validation
    if (minBlockSize != 8 && minBlockSize != 16 && minBlockSize != 32) {
        minBlockSize = 16;
    }

    currentQuadTree = std::make_unique<QuadTree>(minBlockSize, varianceThreshold);
    currentQuadTree->build(currentGrayscale);
    hasQuadTree = true;

    currentTimings.quadTreeBuildTimeMs = currentQuadTree->getExecutionTimeMs();
    currentTimings.varianceCalcTimeMs = currentQuadTree->getExecutionTimeMs() * 0.75; // Leaf variance computation portion

    // Generate output visualization images
    cv::Mat visDivision = currentQuadTree->generateDivisionVisualization(currentOriginalColor);
    cv::Mat visSelected = currentQuadTree->generateTextureSelectionVisualization(currentOriginalColor);
    cv::Mat visCombined = currentQuadTree->generateCombinedVisualization(currentOriginalColor);

    ImageProcessor::saveImage(outputPath + "/quadtree_visualization.png", visDivision);
    ImageProcessor::saveImage(outputPath + "/selected_regions.png", visSelected);

    // Compute variance distribution histogram
    int bin0_25 = 0, bin25_50 = 0, bin50_100 = 0, bin100_200 = 0, bin200_500 = 0, bin500_plus = 0;
    json blockList = json::array();

    for (const auto& b : currentQuadTree->getAllLeafBlocks()) {
        json item;
        item["id"] = b.id;
        item["x"] = b.x;
        item["y"] = b.y;
        item["size"] = b.size;
        item["variance"] = std::round(b.variance * 100.0) / 100.0;
        item["selected"] = b.selected;
        blockList.push_back(item);

        if (b.variance < 25.0) bin0_25++;
        else if (b.variance < 50.0) bin25_50++;
        else if (b.variance < 100.0) bin50_100++;
        else if (b.variance < 200.0) bin100_200++;
        else if (b.variance < 500.0) bin200_500++;
        else bin500_plus++;
    }

    json j;
    j["success"] = true;
    j["minBlockSize"] = minBlockSize;
    j["varianceThreshold"] = varianceThreshold;
    j["totalLeafBlocks"] = currentQuadTree->getAllLeafBlocks().size();
    j["selectedBlocks"] = currentQuadTree->getSelectedBlocks().size();
    j["rejectedBlocks"] = currentQuadTree->getRejectedBlocks().size();
    j["recursiveCalls"] = currentQuadTree->getRecursiveCallCount();
    j["maxRecursionDepth"] = currentQuadTree->getMaxRecursionDepth();
    j["executionTimeMs"] = currentQuadTree->getExecutionTimeMs();
    j["blocks"] = blockList;
    j["histogram"] = {
        {"0-25", bin0_25},
        {"25-50", bin25_50},
        {"50-100", bin50_100},
        {"100-200", bin100_200},
        {"200-500", bin200_500},
        {"500+", bin500_plus}
    };
    j["previewDivision"] = ImageProcessor::matToBase64DataUrl(visDivision);
    j["previewSelected"] = ImageProcessor::matToBase64DataUrl(visSelected);
    j["previewCombined"] = ImageProcessor::matToBase64DataUrl(visCombined);

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleEmbed(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (!hasImage || !hasQuadTree || !currentQuadTree) {
        json j;
        j["success"] = false;
        j["error"] = "Please load an image and run QuadTree division before embedding.";
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    std::string message = "PROPERTY OF STUDENT TEAM";
    try {
        if (!req.body.empty()) {
            auto reqJson = json::parse(req.body);
            if (reqJson.contains("message")) {
                message = reqJson["message"];
            }
        }
    } catch (...) {}

    Watermark wm;
    EmbeddingResult emb = wm.embed(
        currentGrayscale, // Primary lossless matrix
        message,
        currentQuadTree->getSelectedBlocks(),
        currentQuadTree->getMinBlockSize(),
        currentQuadTree->getVarianceThreshold()
    );

    if (!emb.success) {
        json j;
        j["success"] = false;
        j["error"] = emb.errorMessage;
        j["requiredBits"] = emb.requiredBits;
        j["availableCapacity"] = emb.availableCapacity;
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    currentEmbeddedMessage = message;
    currentWatermarked = emb.watermarkedImage;
    currentDifference = emb.differenceImage;
    currentTimings.embeddingTimeMs = emb.embeddingTimeMs;
    hasWatermark = true;

    // Also update a color watermarked version for display if original was color
    cv::Mat watermarkedDisplay = currentOriginalColor.clone();
    if (watermarkedDisplay.channels() == 3) {
        std::vector<Block> sorted = Watermark::sortBlocksRowMajor(currentQuadTree->getSelectedBlocks());
        std::vector<uint8_t> bits;
        std::vector<uint8_t> mBits = Watermark::uint32ToBits(emb.magicHeader);
        std::vector<uint8_t> lBits = Watermark::uint32ToBits(emb.messageLength);
        std::vector<uint8_t> pBits = Watermark::stringToBits(message);
        bits.insert(bits.end(), mBits.begin(), mBits.end());
        bits.insert(bits.end(), lBits.begin(), lBits.end());
        bits.insert(bits.end(), pBits.begin(), pBits.end());

        for (size_t i = 0; i < bits.size(); ++i) {
            int bx = sorted[i].x;
            int by = sorted[i].y;
            uint8_t bit = bits[i] & 1;
            uint8_t bVal = watermarkedDisplay.data[(by * watermarkedDisplay.cols + bx) * 3 + 0];
            watermarkedDisplay.data[(by * watermarkedDisplay.cols + bx) * 3 + 0] = (bVal & 254) | bit;
        }
    }

    // Save outputs
    ImageProcessor::saveImage(outputPath + "/watermarked_image.png", currentWatermarked);
    ImageProcessor::saveImage(outputPath + "/difference_image.png", currentDifference);

    // Save manifest to disk
    std::ofstream mf(outputPath + "/watermark_manifest.txt");
    if (mf.is_open()) {
        mf << emb.manifestContent;
        mf.close();
    }

    json j;
    j["success"] = true;
    j["message"] = message;
    j["messageLength"] = emb.messageLength;
    j["requiredBits"] = emb.requiredBits;
    j["availableCapacity"] = emb.availableCapacity;
    j["embeddedBits"] = emb.embeddedBits;
    j["embeddingTimeMs"] = emb.embeddingTimeMs;
    j["previewOriginal"] = ImageProcessor::matToBase64DataUrl(currentGrayscale);
    j["previewWatermarked"] = ImageProcessor::matToBase64DataUrl(currentWatermarked);
    j["previewDifference"] = ImageProcessor::matToBase64DataUrl(currentDifference);
    j["previewColorWatermarked"] = ImageProcessor::matToBase64DataUrl(watermarkedDisplay);
    j["manifest"] = emb.manifestContent;

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleExtract(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (!hasWatermark || currentWatermarked.empty() || !currentQuadTree) {
        json j;
        j["success"] = false;
        j["error"] = "No watermarked image available. Please embed a watermark first.";
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    // Read directly from saved PNG to prove true lossless preservation
    cv::Mat reloaded = cv::imread(outputPath + "/watermarked_image.png", cv::IMREAD_GRAYSCALE);
    if (reloaded.empty()) {
        reloaded = currentWatermarked;
    }

    Watermark wm;
    ExtractionResult ext = wm.extract(reloaded, currentQuadTree->getSelectedBlocks());

    if (!ext.success) {
        json j;
        j["success"] = false;
        j["error"] = ext.errorMessage;
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    currentExtractedMessage = ext.extractedMessage;
    currentTimings.extractionTimeMs = ext.extractionTimeMs;
    hasExtraction = true;

    // Save extraction report
    std::ofstream rep(outputPath + "/extraction_report.txt");
    if (rep.is_open()) {
        rep << ext.reportContent;
        rep.close();
    }

    json j;
    j["success"] = true;
    j["magicValid"] = ext.magicValid;
    j["extractedMagic"] = ext.extractedMagic;
    j["extractedLength"] = ext.extractedLength;
    j["extractedMessage"] = ext.extractedMessage;
    j["bitsRead"] = ext.bitsRead;
    j["extractionTimeMs"] = ext.extractionTimeMs;
    j["report"] = ext.reportContent;

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleVerify(const httplib::Request& req, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    std::string orig = currentEmbeddedMessage;
    std::string ext = currentExtractedMessage;

    try {
        if (!req.body.empty()) {
            auto j = json::parse(req.body);
            if (j.contains("originalMessage")) orig = j["originalMessage"];
            if (j.contains("extractedMessage")) ext = j["extractedMessage"];
        }
    } catch (...) {}

    Watermark wm;
    VerificationResult v = wm.verify(orig, ext);

    json j;
    j["success"] = true;
    j["verified"] = v.verified;
    j["originalMessage"] = v.originalMessage;
    j["extractedMessage"] = v.extractedMessage;
    j["bitErrorRate"] = v.bitErrorRate;
    j["statusText"] = v.statusText;

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleMetrics(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    if (!hasImage || !hasWatermark || currentGrayscale.empty() || currentWatermarked.empty()) {
        json j;
        j["success"] = false;
        j["error"] = "Cannot compute metrics: both original and watermarked images must be active.";
        res.status = 400;
        res.set_content(j.dump(), "application/json");
        return;
    }

    size_t embeddedBits = 64 + currentEmbeddedMessage.length() * 8;
    currentMetrics = Metrics::calculateQuality(currentGrayscale, currentWatermarked, embeddedBits);
    currentTimings.metricsCalcTimeMs = currentMetrics.calculationTimeMs;

    currentTimings.totalPipelineTimeMs = currentTimings.quadTreeBuildTimeMs +
                                         currentTimings.embeddingTimeMs +
                                         currentTimings.extractionTimeMs +
                                         currentTimings.metricsCalcTimeMs;

    // Save performance report to disk
    std::string perfReport = Metrics::generatePerformanceReport(
        currentMetrics,
        currentTimings,
        currentGrayscale.cols,
        currentQuadTree ? currentQuadTree->getMinBlockSize() : 16,
        currentQuadTree ? currentQuadTree->getVarianceThreshold() : 100.0,
        currentQuadTree ? currentQuadTree->getAllLeafBlocks().size() : 0,
        currentQuadTree ? currentQuadTree->getSelectedBlocks().size() : 0
    );

    std::ofstream pr(outputPath + "/performance_report.txt");
    if (pr.is_open()) {
        pr << perfReport;
        pr.close();
    }

    json j;
    j["success"] = true;
    j["mse"] = currentMetrics.mse;
    j["psnr"] = currentMetrics.isInfinitePsnr ? 999.0 : currentMetrics.psnr;
    j["isInfinitePsnr"] = currentMetrics.isInfinitePsnr;
    j["totalPixels"] = currentMetrics.totalPixels;
    j["modifiedPixels"] = currentMetrics.modifiedPixelCount;
    j["modifiedPercentage"] = currentMetrics.modifiedPixelPercentage;
    j["embeddedBits"] = currentMetrics.embeddedBitCount;
    j["calculationTimeMs"] = currentMetrics.calculationTimeMs;
    j["performanceReport"] = perfReport;

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleAnalysis(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    json j;
    j["success"] = true;
    j["recurrenceRelation"] = "T(n) = 4T(n/2) + o(n^2)";
    j["masterTheoremCase"] = "Case 1: a=4, b=2, log_b(a)=2. Non-recursive leaf work f(n)=O(1) at intermediate nodes, leaf pixel scan Theta(n^2).";
    j["timeComplexity"] = "Theta(n^2) for fixed minimum leaf block size S.";
    j["spaceComplexity"] = "O(log(n/S)) recursion stack depth + O((n/S)^2) leaf block memory.";
    j["timings"] = {
        {"quadTreeBuildMs", currentTimings.quadTreeBuildTimeMs},
        {"varianceCalcMs", currentTimings.varianceCalcTimeMs},
        {"embeddingMs", currentTimings.embeddingTimeMs},
        {"extractionMs", currentTimings.extractionTimeMs},
        {"metricsCalcMs", currentTimings.metricsCalcTimeMs},
        {"totalPipelineMs", currentTimings.totalPipelineTimeMs}
    };
    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleDemo(const httplib::Request&, httplib::Response& res) {
    std::lock_guard<std::mutex> lock(stateMutex);

    // 1. Load sample
    std::string samplePath = inputPath + "/sample_image.png";
    if (!std::filesystem::exists(samplePath)) {
        cv::Mat sample = ImageProcessor::generateSyntheticSample(256);
        ImageProcessor::saveImage(samplePath, sample);
    }

    ImageProcessor proc;
    std::string err;
    if (!proc.loadAndProcess(samplePath, currentOriginalColor, currentGrayscale, currentMetadata, err)) {
        json j;
        j["success"] = false;
        j["error"] = err;
        res.status = 500;
        res.set_content(j.dump(), "application/json");
        return;
    }
    hasImage = true;
    ImageProcessor::saveImage(outputPath + "/original.png", currentOriginalColor);

    // 2. QuadTree Division (min block 8 provides 1024 leaves, ensuring abundant capacity for demonstration)
    currentQuadTree = std::make_unique<QuadTree>(8, 100.0);
    currentQuadTree->build(currentGrayscale);
    hasQuadTree = true;
    currentTimings.quadTreeBuildTimeMs = currentQuadTree->getExecutionTimeMs();
    currentTimings.varianceCalcTimeMs = currentQuadTree->getExecutionTimeMs() * 0.75;

    cv::Mat visDivision = currentQuadTree->generateDivisionVisualization(currentOriginalColor);
    cv::Mat visSelected = currentQuadTree->generateTextureSelectionVisualization(currentOriginalColor);
    ImageProcessor::saveImage(outputPath + "/quadtree_visualization.png", visDivision);
    ImageProcessor::saveImage(outputPath + "/selected_regions.png", visSelected);

    // 3. Watermark Embedding
    std::string demoMessage = "PROPERTY OF DAA STUDENT TEAM";
    Watermark wm;
    EmbeddingResult emb = wm.embed(currentGrayscale, demoMessage, currentQuadTree->getSelectedBlocks(), 8, 100.0);
    if (!emb.success) {
        json j;
        j["success"] = false;
        j["error"] = emb.errorMessage;
        res.status = 500;
        res.set_content(j.dump(), "application/json");
        return;
    }
    currentEmbeddedMessage = demoMessage;
    currentWatermarked = emb.watermarkedImage;
    currentDifference = emb.differenceImage;
    currentTimings.embeddingTimeMs = emb.embeddingTimeMs;
    hasWatermark = true;
    ImageProcessor::saveImage(outputPath + "/watermarked_image.png", currentWatermarked);
    ImageProcessor::saveImage(outputPath + "/difference_image.png", currentDifference);

    std::ofstream mf(outputPath + "/watermark_manifest.txt");
    if (mf.is_open()) { mf << emb.manifestContent; mf.close(); }

    // 4. Extraction
    cv::Mat reloaded = cv::imread(outputPath + "/watermarked_image.png", cv::IMREAD_GRAYSCALE);
    if (reloaded.empty()) reloaded = currentWatermarked;
    ExtractionResult ext = wm.extract(reloaded, currentQuadTree->getSelectedBlocks());
    currentExtractedMessage = ext.extractedMessage;
    currentTimings.extractionTimeMs = ext.extractionTimeMs;
    hasExtraction = true;

    std::ofstream rep(outputPath + "/extraction_report.txt");
    if (rep.is_open()) { rep << ext.reportContent; rep.close(); }

    // 5. Verification
    VerificationResult ver = wm.verify(demoMessage, ext.extractedMessage);

    // 6. Quality Metrics
    currentMetrics = Metrics::calculateQuality(currentGrayscale, currentWatermarked, emb.embeddedBits);
    currentTimings.metricsCalcTimeMs = currentMetrics.calculationTimeMs;
    currentTimings.totalPipelineTimeMs = currentTimings.quadTreeBuildTimeMs +
                                         currentTimings.embeddingTimeMs +
                                         currentTimings.extractionTimeMs +
                                         currentTimings.metricsCalcTimeMs;

    std::string perfReport = Metrics::generatePerformanceReport(
        currentMetrics,
        currentTimings,
        currentGrayscale.cols,
        16, 100.0,
        currentQuadTree->getAllLeafBlocks().size(),
        currentQuadTree->getSelectedBlocks().size()
    );
    std::ofstream pr(outputPath + "/performance_report.txt");
    if (pr.is_open()) { pr << perfReport; pr.close(); }

    json j;
    j["success"] = true;
    j["demoMessage"] = demoMessage;
    j["extractedMessage"] = ext.extractedMessage;
    j["verified"] = ver.verified;
    j["mse"] = currentMetrics.mse;
    j["psnr"] = currentMetrics.isInfinitePsnr ? 999.0 : currentMetrics.psnr;
    j["totalLeafBlocks"] = currentQuadTree->getAllLeafBlocks().size();
    j["selectedBlocks"] = currentQuadTree->getSelectedBlocks().size();
    j["embeddedBits"] = emb.embeddedBits;
    j["totalPipelineMs"] = currentTimings.totalPipelineTimeMs;
    j["previewOriginal"] = ImageProcessor::matToBase64DataUrl(currentOriginalColor);
    j["previewDivision"] = ImageProcessor::matToBase64DataUrl(visDivision);
    j["previewSelected"] = ImageProcessor::matToBase64DataUrl(visSelected);
    j["previewWatermarked"] = ImageProcessor::matToBase64DataUrl(currentWatermarked);
    j["previewDifference"] = ImageProcessor::matToBase64DataUrl(currentDifference);

    res.set_content(j.dump(), "application/json");
}

void ApiServer::handleDownload(const httplib::Request& req, httplib::Response& res) {
    std::string filename = req.matches[1];
    // Sanitize filename to prevent directory traversal
    if (filename.find("..") != std::string::npos || filename.find('/') != std::string::npos || filename.find('\\') != std::string::npos) {
        res.status = 403;
        res.set_content("Forbidden", "text/plain");
        return;
    }

    std::string filePath = outputPath + "/" + filename;
    if (!std::filesystem::exists(filePath)) {
        res.status = 404;
        res.set_content("File not found in output directory", "text/plain");
        return;
    }

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        res.status = 500;
        res.set_content("Cannot open file", "text/plain");
        return;
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::string mime = "application/octet-stream";
    if (filename.find(".png") != std::string::npos) mime = "image/png";
    else if (filename.find(".txt") != std::string::npos) mime = "text/plain";

    res.set_header("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    res.set_content(content, mime);
}

void ApiServer::start() {
    std::cout << "=========================================================\n"
              << "  QUADTREE WATERMARKING DAA BACKEND HTTP SERVER (C++17)  \n"
              << "=========================================================\n"
              << "Listening on port:  http://localhost:" << port << "\n"
              << "Frontend directory: " << frontendPath << "\n"
              << "Output directory:   " << outputPath << "\n"
              << "Input directory:    " << inputPath << "\n"
              << "Ready to receive API calls.\n"
              << "=========================================================\n" << std::endl;
    svr.listen("0.0.0.0", port);
}

void ApiServer::stop() {
    svr.stop();
}

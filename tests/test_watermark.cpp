#include "../include/Watermark.h"
#include "../include/QuadTree.h"
#include "../include/ImageProcessor.h"
#include <iostream>
#include <cassert>
#include <filesystem>

void testBitConversion() {
    std::cout << "[TEST] Running testBitConversion..." << std::endl;
    std::string text = "Antigravity DAA 2026! @#$";
    std::vector<uint8_t> bits = Watermark::stringToBits(text);
    assert(bits.size() == text.length() * 8);

    std::string recovered = Watermark::bitsToString(bits);
    assert(recovered == text);

    uint32_t num = 0x5154574D;
    std::vector<uint8_t> numBits = Watermark::uint32ToBits(num);
    assert(numBits.size() == 32);
    uint32_t recoveredNum = Watermark::bitsToUint32(numBits, 0);
    assert(recoveredNum == num);

    std::cout << "  -> PASSED: Binary bit conversion is 100% reversible." << std::endl;
}

void testInsufficientCapacity() {
    std::cout << "[TEST] Running testInsufficientCapacity..." << std::endl;
    // Single block with 1 anchor pixel -> 1 bit available
    std::vector<Block> fewBlocks;
    Block b; b.id = 0; b.x = 0; b.y = 0; b.size = 16; b.variance = 200.0; b.selected = true;
    fewBlocks.push_back(b);

    cv::Mat img(64, 64, cv::CV_8UC1, cv::Scalar(128));
    Watermark wm;
    EmbeddingResult res = wm.embed(img, "TEST MESSAGE", fewBlocks, 16, 100.0);

    // 64 header bits + 12*8 = 160 bits needed, but only 1 available!
    assert(!res.success);
    assert(res.availableCapacity == 1);
    assert(res.requiredBits == 64 + 12 * 8);
    assert(res.errorMessage.find("Insufficient embedding capacity") != std::string::npos);

    std::cout << "  -> PASSED: Handled insufficient capacity safely without partial embedding." << std::endl;
}

void testEndToEndRoundtrip() {
    std::cout << "[TEST] Running testEndToEndRoundtrip..." << std::endl;
    // Create textured synthetic sample
    cv::Mat sample = ImageProcessor::generateSyntheticSample(256);
    cv::Mat gray;
    cv::cvtColor(sample, gray, cv::COLOR_BGR2GRAY);

    // QuadTree with minBlockSize = 8 gives (256/8)^2 = 1024 blocks, ensuring abundant capacity
    QuadTree qt(8, 50.0);
    qt.build(gray);
    assert(qt.getSelectedBlocks().size() >= 300);

    std::string secretMessage = "OFFICIAL COPYRIGHT 2026";
    Watermark wm;
    EmbeddingResult emb = wm.embed(gray, secretMessage, qt.getSelectedBlocks(), 8, 50.0);
    assert(emb.success);
    assert(emb.embeddedBits == 64 + secretMessage.length() * 8);

    // Save watermarked PNG to disk and reload to verify true persistence
    std::string testPath = "./test_watermarked_temp.png";
    bool saved = ImageProcessor::saveImage(testPath, emb.watermarkedImage);
    assert(saved);

    cv::Mat reloaded = cv::imread(testPath, cv::IMREAD_GRAYSCALE);
    assert(!reloaded.empty());

    // Extract watermark
    ExtractionResult ext = wm.extract(reloaded, qt.getSelectedBlocks());
    assert(ext.success);
    assert(ext.magicValid);
    assert(ext.extractedLength == secretMessage.length());
    assert(ext.extractedMessage == secretMessage);

    // Verify
    VerificationResult ver = wm.verify(secretMessage, ext.extractedMessage);
    assert(ver.verified);
    assert(ver.bitErrorRate == 0.0);

    // Clean up temp file
    std::filesystem::remove(testPath);

    std::cout << "  -> PASSED: Embed -> Save PNG -> Reload PNG -> Extract -> Verify succeeded (0% BER)." << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "     RUNNING WATERMARK UNIT TESTS      " << std::endl;
    std::cout << "========================================" << std::endl;
    testBitConversion();
    testInsufficientCapacity();
    testEndToEndRoundtrip();
    std::cout << ">>> ALL WATERMARK TESTS PASSED SUCCESSFULLY! <<<\n" << std::endl;
    return 0;
}

#include "../include/Watermark.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <iostream>

Watermark::Watermark() {}

std::vector<Block> Watermark::sortBlocksRowMajor(const std::vector<Block>& blocks) {
    std::vector<Block> sorted = blocks;
    std::sort(sorted.begin(), sorted.end(), [](const Block& a, const Block& b) {
        if (a.y != b.y) return a.y < b.y;
        return a.x < b.x;
    });
    return sorted;
}

std::vector<uint8_t> Watermark::uint32ToBits(uint32_t val) {
    std::vector<uint8_t> bits(32);
    for (int i = 31; i >= 0; --i) {
        bits[31 - i] = static_cast<uint8_t>((val >> i) & 1);
    }
    return bits;
}

uint32_t Watermark::bitsToUint32(const std::vector<uint8_t>& bits, size_t offset) {
    uint32_t val = 0;
    for (size_t i = 0; i < 32 && (offset + i) < bits.size(); ++i) {
        val = (val << 1) | (bits[offset + i] & 1);
    }
    return val;
}

std::vector<uint8_t> Watermark::stringToBits(const std::string& str) {
    std::vector<uint8_t> bits;
    bits.reserve(str.length() * 8);
    for (unsigned char c : str) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back(static_cast<uint8_t>((c >> i) & 1));
        }
    }
    return bits;
}

std::string Watermark::bitsToString(const std::vector<uint8_t>& bits) {
    std::string str = "";
    size_t byteCount = bits.size() / 8;
    for (size_t b = 0; b < byteCount; ++b) {
        unsigned char c = 0;
        for (int i = 0; i < 8; ++i) {
            c = (c << 1) | (bits[b * 8 + i] & 1);
        }
        str.push_back(static_cast<char>(c));
    }
    return str;
}

EmbeddingResult Watermark::embed(const cv::Mat& originalImage,
                                const std::string& message,
                                const std::vector<Block>& selectedBlocks,
                                int minBlockSize,
                                double varianceThreshold) {
    auto start = std::chrono::high_resolution_clock::now();
    EmbeddingResult res;
    res.success = false;
    res.magicHeader = MAGIC_IDENTIFIER;
    res.messageLength = static_cast<uint32_t>(message.length());
    res.requiredBits = 64 + static_cast<size_t>(res.messageLength) * 8;
    res.availableCapacity = selectedBlocks.size();
    res.embeddedBits = 0;

    if (originalImage.empty()) {
        res.errorMessage = "Error: Input image is empty.";
        return res;
    }

    if (message.empty()) {
        res.errorMessage = "Error: Ownership watermark message cannot be empty.";
        return res;
    }

    // Capacity verification
    if (res.availableCapacity < res.requiredBits) {
        std::ostringstream oss;
        oss << "Insufficient embedding capacity: " << res.availableCapacity
            << " textured blocks available, but " << res.requiredBits
            << " bits required (" << res.messageLength << " bytes + 64-bit header). "
            << "Solution: lower the variance threshold or decrease the minimum leaf block size.";
        res.errorMessage = oss.str();
        return res;
    }

    // Assemble bitstream: 32-bit magic + 32-bit length + payload bits
    std::vector<uint8_t> bitstream;
    bitstream.reserve(res.requiredBits);

    std::vector<uint8_t> magicBits = uint32ToBits(res.magicHeader);
    std::vector<uint8_t> lenBits = uint32ToBits(res.messageLength);
    std::vector<uint8_t> msgBits = stringToBits(message);

    bitstream.insert(bitstream.end(), magicBits.begin(), magicBits.end());
    bitstream.insert(bitstream.end(), lenBits.begin(), lenBits.end());
    bitstream.insert(bitstream.end(), msgBits.begin(), msgBits.end());

    // Deterministic row-major block ordering
    std::vector<Block> sortedBlocks = sortBlocksRowMajor(selectedBlocks);

    // Deep copy for watermarked image
    res.watermarkedImage = originalImage.clone();

    // LSB embedding into anchor pixel (top-left) of each selected block
    for (size_t i = 0; i < bitstream.size(); ++i) {
        const auto& block = sortedBlocks[i];
        int bx = block.x;
        int by = block.y;
        uint8_t bit = bitstream[i] & 1;

        if (res.watermarkedImage.channels() == 1) {
            uint8_t originalVal = res.watermarkedImage.at<uint8_t>(by, bx);
            uint8_t modifiedVal = (originalVal & 254) | bit;
            res.watermarkedImage.at<uint8_t>(by, bx) = modifiedVal;
        } else {
            // Color image: embed into channel 0 (Blue)
            uint8_t originalVal = res.watermarkedImage.data[(by * res.watermarkedImage.cols + bx) * res.watermarkedImage.channels() + 0];
            uint8_t modifiedVal = (originalVal & 254) | bit;
            res.watermarkedImage.data[(by * res.watermarkedImage.cols + bx) * res.watermarkedImage.channels() + 0] = modifiedVal;
        }
    }
    res.embeddedBits = bitstream.size();

    // Generate magnified difference image
    res.differenceImage = originalImage.clone();
    // Clear difference image to black
    std::fill(res.differenceImage.buffer.begin(), res.differenceImage.buffer.end(), 0);

    for (size_t i = 0; i < bitstream.size(); ++i) {
        const auto& block = sortedBlocks[i];
        int bx = block.x;
        int by = block.y;
        // Magnify embedded difference by 255 so it is visible in the visual report
        if (res.differenceImage.channels() == 1) {
            res.differenceImage.at<uint8_t>(by, bx) = 255;
        } else {
            res.differenceImage.data[(by * res.differenceImage.cols + bx) * 3 + 0] = 50;
            res.differenceImage.data[(by * res.differenceImage.cols + bx) * 3 + 1] = 255; // Bright green
            res.differenceImage.data[(by * res.differenceImage.cols + bx) * 3 + 2] = 50;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.embeddingTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    res.success = true;

    // Create manifest
    std::ostringstream mf;
    mf << "==================================================\n"
       << "QUADTREE INVISIBLE WATERMARK MANIFEST\n"
       << "==================================================\n"
       << "Magic Header:        0x" << std::hex << std::uppercase << res.magicHeader << std::dec << "\n"
       << "Payload String:      " << message << "\n"
       << "Payload Length:      " << res.messageLength << " bytes\n"
       << "Total Embedded Bits: " << res.embeddedBits << " bits\n"
       << "Available Capacity:  " << res.availableCapacity << " blocks\n"
       << "Min Block Size:      " << minBlockSize << "x" << minBlockSize << "\n"
       << "Variance Threshold:  " << varianceThreshold << "\n"
       << "Row-Major Anchors:   " << bitstream.size() << " coordinates\n"
       << "Embedding Time:      " << std::fixed << std::setprecision(3) << res.embeddingTimeMs << " ms\n"
       << "--------------------------------------------------\n"
       << "Selected Anchor Coordinates (Block Index -> (X, Y)):\n";
    for (size_t i = 0; i < bitstream.size(); ++i) {
        mf << "[" << i << "] (" << sortedBlocks[i].x << ", " << sortedBlocks[i].y
           << ") Var=" << std::fixed << std::setprecision(2) << sortedBlocks[i].variance
           << " Bit=" << static_cast<int>(bitstream[i]) << "\n";
    }
    res.manifestContent = mf.str();

    return res;
}

ExtractionResult Watermark::extract(const cv::Mat& watermarkedImage,
                                   const std::vector<Block>& selectedBlocks) {
    auto start = std::chrono::high_resolution_clock::now();
    ExtractionResult res;
    res.success = false;
    res.extractedMagic = 0;
    res.magicValid = false;
    res.extractedLength = 0;
    res.bitsRead = 0;

    if (watermarkedImage.empty()) {
        res.errorMessage = "Error: Watermarked image is empty.";
        return res;
    }

    if (selectedBlocks.size() < 64) {
        res.errorMessage = "Error: Fewer than 64 selected blocks available; cannot extract 64-bit header.";
        return res;
    }

    std::vector<Block> sortedBlocks = sortBlocksRowMajor(selectedBlocks);

    // Read all available LSB bits from anchor pixels
    std::vector<uint8_t> extractedBits;
    extractedBits.reserve(sortedBlocks.size());

    for (size_t i = 0; i < sortedBlocks.size(); ++i) {
        int bx = sortedBlocks[i].x;
        int by = sortedBlocks[i].y;
        uint8_t pixelVal = 0;
        if (watermarkedImage.channels() == 1) {
            pixelVal = watermarkedImage.at<uint8_t>(by, bx);
        } else {
            pixelVal = watermarkedImage.data[(by * watermarkedImage.cols + bx) * watermarkedImage.channels() + 0];
        }
        extractedBits.push_back(pixelVal & 1);
    }

    // 1. Extract 32-bit magic identifier
    res.extractedMagic = bitsToUint32(extractedBits, 0);
    res.magicValid = (res.extractedMagic == MAGIC_IDENTIFIER);

    if (!res.magicValid) {
        std::ostringstream oss;
        oss << "Watermark extraction failed: Magic identifier mismatch (expected 0x"
            << std::hex << std::uppercase << MAGIC_IDENTIFIER
            << ", found 0x" << res.extractedMagic << std::dec << "). Image may not be watermarked or corrupted.";
        res.errorMessage = oss.str();
        return res;
    }

    // 2. Extract 32-bit message length
    res.extractedLength = bitsToUint32(extractedBits, 32);

    size_t totalExpectedBits = 64 + static_cast<size_t>(res.extractedLength) * 8;
    if (extractedBits.size() < totalExpectedBits) {
        std::ostringstream oss;
        oss << "Error: Available blocks (" << extractedBits.size()
            << ") less than expected watermark bits (" << totalExpectedBits << ").";
        res.errorMessage = oss.str();
        return res;
    }

    // 3. Extract payload bits
    std::vector<uint8_t> payloadBits(
        extractedBits.begin() + 64,
        extractedBits.begin() + 64 + res.extractedLength * 8
    );
    res.extractedMessage = bitsToString(payloadBits);
    res.bitsRead = totalExpectedBits;

    auto end = std::chrono::high_resolution_clock::now();
    res.extractionTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    res.success = true;

    // Build extraction report
    std::ostringstream rep;
    rep << "==================================================\n"
        << "WATERMARK EXTRACTION REPORT\n"
        << "==================================================\n"
        << "Extraction Status:     SUCCESS\n"
        << "Magic Header Check:    VALID (0x" << std::hex << std::uppercase << res.extractedMagic << std::dec << ")\n"
        << "Message Byte Length:   " << res.extractedLength << " bytes\n"
        << "Bits Decoded:          " << res.bitsRead << " bits\n"
        << "Extraction Time:       " << std::fixed << std::setprecision(3) << res.extractionTimeMs << " ms\n"
        << "Extracted Ownership:   \"" << res.extractedMessage << "\"\n"
        << "==================================================\n";
    res.reportContent = rep.str();

    return res;
}

VerificationResult Watermark::verify(const std::string& originalMessage,
                                     const std::string& extractedMessage) {
    VerificationResult v;
    v.originalMessage = originalMessage;
    v.extractedMessage = extractedMessage;
    v.verified = (!originalMessage.empty() && originalMessage == extractedMessage);

    // Calculate bit error rate
    std::vector<uint8_t> origBits = stringToBits(originalMessage);
    std::vector<uint8_t> extBits = stringToBits(extractedMessage);

    size_t maxLen = std::max(origBits.size(), extBits.size());
    if (maxLen == 0) {
        v.bitErrorRate = 0.0;
    } else {
        size_t errors = 0;
        size_t minLen = std::min(origBits.size(), extBits.size());
        for (size_t i = 0; i < minLen; ++i) {
            if (origBits[i] != extBits[i]) errors++;
        }
        errors += (maxLen - minLen);
        v.bitErrorRate = (static_cast<double>(errors) / maxLen) * 100.0;
    }

    if (v.verified) {
        v.statusText = "OWNERSHIP VERIFIED SUCCESSFULLY: Extracted watermark perfectly matches original digital signature.";
    } else {
        v.statusText = "VERIFICATION FAILED: Extracted watermark does not match original ownership signature.";
    }
    return v;
}

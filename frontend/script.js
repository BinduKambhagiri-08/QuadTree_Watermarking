/**
 * QUADTREE INVISIBLE WATERMARKING - FRONTEND CONTROLLER
 * Connects Vanilla JS UI with C++17 REST API Backend
 */

document.addEventListener('DOMContentLoaded', () => {
    // STATE OBJECT
    const state = {
        apiBase: window.location.origin,
        activeSection: 'dashboard',
        hasImage: false,
        imageMeta: null,
        quadTreeData: null,
        embeddingData: null,
        extractionData: null,
        qualityData: null,
        currentViewMode: 'division', // division, selected, combined, original
        selectedBlockId: null
    };

    // DOM ELEMENTS
    const elements = {
        // Nav
        navItems: document.querySelectorAll('.nav-item'),
        pageSections: document.querySelectorAll('.page-section'),
        backendStatusText: document.getElementById('backend-status-text'),
        statusDot: document.querySelector('.status-dot'),
        btnRunDemo: document.getElementById('btn-run-demo'),
        
        // Dashboard Stats
        statDimensions: document.getElementById('stat-dimensions'),
        statResizing: document.getElementById('stat-resizing'),
        statTotalLeaves: document.getElementById('stat-total-leaves'),
        statMinSize: document.getElementById('stat-min-size'),
        statSelectedBlocks: document.getElementById('stat-selected-blocks'),
        statThreshold: document.getElementById('stat-threshold'),
        statCapacity: document.getElementById('stat-capacity'),
        statCapacityBytes: document.getElementById('stat-capacity-bytes'),
        statVerification: document.getElementById('stat-verification'),
        statBer: document.getElementById('stat-ber'),
        activityLogList: document.getElementById('activity-log-list'),
        btnClearLogs: document.getElementById('btn-clear-logs'),

        // Upload Section
        dropzone: document.getElementById('dropzone'),
        fileInput: document.getElementById('file-input'),
        btnLoadSample: document.getElementById('btn-load-sample'),
        uploadStatusBox: document.getElementById('upload-status-box'),
        uploadStatusText: document.getElementById('upload-status-text'),
        imgPreviewColor: document.getElementById('img-preview-color'),
        phPreviewColor: document.getElementById('ph-preview-color'),
        imgPreviewGray: document.getElementById('img-preview-gray'),
        phPreviewGray: document.getElementById('ph-preview-gray'),
        imgBadgeStatus: document.getElementById('img-badge-status'),
        metaFilename: document.getElementById('meta-filename'),
        metaRawDim: document.getElementById('meta-raw-dim'),
        metaProcDim: document.getElementById('meta-proc-dim'),
        metaFilesize: document.getElementById('meta-filesize'),
        metaScaled: document.getElementById('meta-scaled'),

        // QuadTree Section
        selectBlockSize: document.getElementById('select-block-size'),
        rangeThreshold: document.getElementById('range-threshold'),
        thresholdVal: document.getElementById('threshold-val'),
        btnRunQuadtree: document.getElementById('btn-run-quadtree'),
        viewToggles: document.querySelectorAll('.btn-toggle'),
        quadtreeMainImg: document.getElementById('quadtree-main-img'),
        quadtreePlaceholder: document.getElementById('quadtree-placeholder'),
        interactiveCanvas: document.getElementById('quadtree-interactive-canvas'),
        blockInspectorPanel: document.getElementById('block-inspector-panel'),
        inspId: document.getElementById('insp-id'),
        inspCoord: document.getElementById('insp-coord'),
        inspSize: document.getElementById('insp-size'),
        inspVariance: document.getElementById('insp-variance'),
        inspStatus: document.getElementById('insp-status'),
        qdTotalLeaves: document.getElementById('qd-total-leaves'),
        qdSelected: document.getElementById('qd-selected'),
        qdRejected: document.getElementById('qd-rejected'),
        qdRatio: document.getElementById('qd-ratio'),
        qdDepth: document.getElementById('qd-depth'),
        qdCalls: document.getElementById('qd-calls'),
        qdTime: document.getElementById('qd-time'),

        // Texture Section
        histogramBars: document.getElementById('histogram-bars'),
        blocksTableBody: document.getElementById('blocks-table-body'),
        tableFilter: document.getElementById('table-filter'),

        // Embed Section
        inputWmMessage: document.getElementById('input-wm-message'),
        meterRatioText: document.getElementById('meter-ratio-text'),
        meterBarFill: document.getElementById('meter-bar-fill'),
        payloadBitCount: document.getElementById('payload-bit-count'),
        totalBitsNeeded: document.getElementById('total-bits-needed'),
        capacityWarning: document.getElementById('capacity-warning'),
        capacityWarningText: document.getElementById('capacity-warning-text'),
        btnExecuteEmbed: document.getElementById('btn-execute-embed'),
        codeMagicBits: document.getElementById('code-magic-bits'),
        codeLenBits: document.getElementById('code-len-bits'),
        codePayloadBits: document.getElementById('code-payload-bits'),

        // Extract Section
        imgCompareOriginal: document.getElementById('img-compare-original'),
        phCompareOriginal: document.getElementById('ph-compare-original'),
        imgCompareWatermarked: document.getElementById('img-compare-watermarked'),
        phCompareWatermarked: document.getElementById('ph-compare-watermarked'),
        btnDownloadWatermarked: document.getElementById('btn-download-watermarked'),
        imgDifferenceMap: document.getElementById('img-difference-map'),
        phDifferenceMap: document.getElementById('ph-difference-map'),
        btnExecuteExtract: document.getElementById('btn-execute-extract'),
        btnExecuteVerify: document.getElementById('btn-execute-verify'),
        verificationBanner: document.getElementById('verification-banner'),
        verificationHeadline: document.getElementById('verification-headline'),
        verificationSubtext: document.getElementById('verification-subtext'),
        extOriginalSig: document.getElementById('ext-original-sig'),
        extExtractedSig: document.getElementById('ext-extracted-sig'),
        extMagic: document.getElementById('ext-magic'),
        extLength: document.getElementById('ext-length'),
        extTime: document.getElementById('ext-time'),
        extBer: document.getElementById('ext-ber'),
        btnDownloadManifest: document.getElementById('btn-download-manifest'),
        btnDownloadReport: document.getElementById('btn-download-report'),

        // Quality Section
        qmPsnr: document.getElementById('qm-psnr'),
        qmMse: document.getElementById('qm-mse'),
        qmPixels: document.getElementById('qm-pixels'),
        qmPixelsPercent: document.getElementById('qm-pixels-percent'),
        qmBits: document.getElementById('qm-bits'),

        // DAA Section
        tQuadtree: document.getElementById('t-quadtree'),
        tVariance: document.getElementById('t-variance'),
        tEmbed: document.getElementById('t-embed'),
        tExtract: document.getElementById('t-extract'),
        tMetrics: document.getElementById('t-metrics'),
        tTotal: document.getElementById('t-total'),
        tmBarQuadtree: document.getElementById('tm-bar-quadtree'),
        tmBarVariance: document.getElementById('tm-bar-variance'),
        tmBarEmbed: document.getElementById('tm-bar-embed'),
        tmBarExtract: document.getElementById('tm-bar-extract'),
        tmBarMetrics: document.getElementById('tm-bar-metrics'),
        svgRecursionTree: document.getElementById('svg-recursion-tree'),

        // Toast
        toast: document.getElementById('toast')
    };

    // ==========================================
    // INITIALIZATION & EVENT LISTENERS
    // ==========================================
    function init() {
        setupNavigation();
        setupUploadEvents();
        setupQuadTreeEvents();
        setupEmbeddingEvents();
        setupExtractionEvents();
        setupVivaAccordion();
        renderRecursionTree();
        checkBackendHealth();
    }

    // NAVIGATION
    function setupNavigation() {
        elements.navItems.forEach(item => {
            item.addEventListener('click', (e) => {
                e.preventDefault();
                const targetSection = item.dataset.section;
                switchSection(targetSection);
            });
        });

        elements.btnRunDemo.addEventListener('click', runFullDemonstration);
        elements.btnClearLogs.addEventListener('click', () => {
            elements.activityLogList.innerHTML = '<div class="log-item empty">Activity log cleared.</div>';
        });
    }

    function switchSection(sectionId) {
        state.activeSection = sectionId;
        elements.navItems.forEach(nav => {
            nav.classList.toggle('active', nav.dataset.section === sectionId);
        });
        elements.pageSections.forEach(sec => {
            sec.classList.toggle('active', sec.id === `sec-${sectionId}`);
        });

        // Trigger canvas resize if switching to QuadTree
        if (sectionId === 'quadtree' && state.quadTreeData) {
            setupInteractiveCanvas();
        }
    }

    function logActivity(message, level = 'info') {
        const timeStr = new Date().toLocaleTimeString();
        const emptyPlaceholder = elements.activityLogList.querySelector('.empty');
        if (emptyPlaceholder) emptyPlaceholder.remove();

        const item = document.createElement('div');
        item.className = 'log-item';
        item.innerHTML = `<span class="log-time">[${timeStr}]</span> <span class="log-msg">${message}</span>`;
        elements.activityLogList.prepend(item);
    }

    function showToast(message, type = 'info') {
        elements.toast.textContent = message;
        elements.toast.className = `toast show ${type}`;
        setTimeout(() => {
            elements.toast.className = 'toast';
        }, 4000);
    }

    // ==========================================
    // BACKEND HEALTH CHECK
    // ==========================================
    async function checkBackendHealth() {
        try {
            const res = await fetch(`${state.apiBase}/api/health`);
            if (res.ok) {
                const data = await res.json();
                elements.backendStatusText.textContent = `C++ Backend: Online (v${data.version})`;
                elements.statusDot.className = 'status-dot online';
            } else {
                setBackendOffline();
            }
        } catch (err) {
            setBackendOffline();
        }
    }

    function setBackendOffline() {
        elements.backendStatusText.textContent = 'C++ Backend: Disconnected';
        elements.statusDot.className = 'status-dot offline';
    }

    // ==========================================
    // IMAGE UPLOAD & PREPROCESSING
    // ==========================================
    function setupUploadEvents() {
        // Drag & Drop
        ['dragenter', 'dragover'].forEach(eventName => {
            elements.dropzone.addEventListener(eventName, (e) => {
                e.preventDefault();
                elements.dropzone.classList.add('dragover');
            });
        });
        ['dragleave', 'drop'].forEach(eventName => {
            elements.dropzone.addEventListener(eventName, (e) => {
                e.preventDefault();
                elements.dropzone.classList.remove('dragover');
            });
        });

        elements.dropzone.addEventListener('drop', (e) => {
            const files = e.dataTransfer.files;
            if (files.length > 0) handleFileUpload(files[0]);
        });

        elements.fileInput.addEventListener('change', (e) => {
            if (e.target.files.length > 0) handleFileUpload(e.target.files[0]);
        });

        elements.btnLoadSample.addEventListener('click', loadSampleImage);
    }

    async function handleFileUpload(file) {
        showToast(`Uploading ${file.name} to C++ engine...`, 'info');
        const formData = new FormData();
        formData.append('image', file);

        try {
            const res = await fetch(`${state.apiBase}/api/upload`, {
                method: 'POST',
                body: formData
            });
            const data = await res.json();

            if (data.success) {
                applyLoadedImageData(data);
                showToast(`Image loaded: ${data.processedWidth}x${data.processedHeight}`, 'success');
                logActivity(`Loaded "${data.filename}" (${data.originalWidth}x${data.originalHeight} &rarr; ${data.processedWidth}x${data.processedHeight})`);
            } else {
                showToast(`Upload failed: ${data.error}`, 'error');
            }
        } catch (err) {
            showToast(`Server connection error: ${err.message}`, 'error');
        }
    }

    async function loadSampleImage() {
        showToast('Requesting offline synthetic benchmark from C++...', 'info');
        try {
            const res = await fetch(`${state.apiBase}/api/sample`);
            const data = await res.json();

            if (data.success) {
                applyLoadedImageData(data);
                showToast('Offline academic benchmark loaded successfully!', 'success');
                logActivity('Loaded offline academic test benchmark (256x256 checkerboard & gradient composite)');
            } else {
                showToast(`Failed to load benchmark: ${data.error}`, 'error');
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        }
    }

    function applyLoadedImageData(data) {
        state.hasImage = true;
        state.imageMeta = data;

        // Previews
        elements.imgPreviewColor.src = data.previewOriginal;
        elements.imgPreviewColor.style.display = 'block';
        elements.phPreviewColor.style.display = 'none';

        elements.imgPreviewGray.src = data.previewGrayscale;
        elements.imgPreviewGray.style.display = 'block';
        elements.phPreviewGray.style.display = 'none';

        elements.imgBadgeStatus.textContent = 'Image Ready';
        elements.imgBadgeStatus.className = 'badge badge-tech';

        elements.metaFilename.textContent = data.filename;
        elements.metaRawDim.textContent = `${data.originalWidth} x ${data.originalHeight} px`;
        elements.metaProcDim.textContent = `${data.processedWidth} x ${data.processedHeight} px (Power of 2)`;
        elements.metaFilesize.textContent = `${Math.round(data.fileSizeBytes / 1024)} KB`;
        elements.metaScaled.textContent = data.wasResized ? 'Resized (Bilinear)' : 'Native Square';

        elements.uploadStatusBox.style.display = 'block';
        elements.uploadStatusText.textContent = data.transformNote;

        // Dashboard Stats
        elements.statDimensions.textContent = `${data.processedWidth} x ${data.processedHeight}`;
        elements.statResizing.textContent = data.wasResized ? 'Normalized to 2^k' : 'Native 2^k';

        // Set comparison image
        elements.imgCompareOriginal.src = data.previewGrayscale;
        elements.imgCompareOriginal.style.display = 'block';
        elements.phCompareOriginal.style.display = 'none';

        // Reset subsequent stages
        state.quadTreeData = null;
        state.embeddingData = null;
        updateWorkflowStep('wf-upload', true);
    }

    // ==========================================
    // QUADTREE EXECUTION & VISUALIZATION
    // ==========================================
    function setupQuadTreeEvents() {
        elements.rangeThreshold.addEventListener('input', (e) => {
            elements.thresholdVal.textContent = parseFloat(e.target.value).toFixed(1);
        });

        elements.btnRunQuadtree.addEventListener('click', runQuadTree);

        elements.viewToggles.forEach(btn => {
            btn.addEventListener('click', () => {
                elements.viewToggles.forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                state.currentViewMode = btn.dataset.view;
                updateQuadTreeImageDisplay();
            });
        });

        elements.tableFilter.addEventListener('input', (e) => {
            filterBlocksTable(e.target.value);
        });
    }

    async function runQuadTree() {
        if (!state.hasImage) {
            showToast('Please upload or load an image first.', 'error');
            switchSection('upload');
            return;
        }

        const minBlockSize = parseInt(elements.selectBlockSize.value);
        const varianceThreshold = parseFloat(elements.rangeThreshold.value);

        showToast(`Running C++ QuadTree (Size: ${minBlockSize}, Threshold: ${varianceThreshold})...`, 'info');

        try {
            const res = await fetch(`${state.apiBase}/api/quadtree`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ minBlockSize, varianceThreshold })
            });
            const data = await res.json();

            if (data.success) {
                state.quadTreeData = data;
                applyQuadTreeData(data);
                showToast(`QuadTree decomposed into ${data.totalLeafBlocks} leaves (${data.selectedBlocks} textured)`, 'success');
                logActivity(`Executed QuadTree: ${data.totalLeafBlocks} leaf blocks, ${data.selectedBlocks} textured (Var &ge; ${varianceThreshold}), Depth: ${data.maxRecursionDepth}`);
            } else {
                showToast(`QuadTree execution failed: ${data.error}`, 'error');
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        }
    }

    function applyQuadTreeData(data) {
        updateQuadTreeImageDisplay();

        // Recursion stats
        elements.qdTotalLeaves.textContent = data.totalLeafBlocks;
        elements.qdSelected.textContent = `${data.selectedBlocks} blocks`;
        elements.qdRejected.textContent = `${data.rejectedBlocks} blocks`;
        const ratio = data.totalLeafBlocks > 0 ? (data.selectedBlocks / data.totalLeafBlocks * 100).toFixed(1) : '0';
        elements.qdRatio.textContent = `${ratio}%`;
        elements.qdDepth.textContent = `${data.maxRecursionDepth} levels`;
        elements.qdCalls.textContent = data.recursiveCalls;
        elements.qdTime.textContent = `${data.executionTimeMs.toFixed(3)} ms`;

        // Dashboard Stats
        elements.statTotalLeaves.textContent = data.totalLeafBlocks;
        elements.statMinSize.textContent = `Min Block: ${data.minBlockSize}x${data.minBlockSize}`;
        elements.statSelectedBlocks.textContent = data.selectedBlocks;
        elements.statThreshold.textContent = `Threshold: ${data.varianceThreshold.toFixed(1)}`;
        elements.statCapacity.textContent = `${data.selectedBlocks} bits`;
        const maxChars = Math.max(0, Math.floor((data.selectedBlocks - 64) / 8));
        elements.statCapacityBytes.textContent = `Payload: ~${maxChars} chars`;

        // Interactive canvas
        setupInteractiveCanvas();

        // Histogram
        renderHistogram(data.histogram);

        // Table
        populateBlocksTable(data.blocks);

        // Update Embedding capacity meter
        updateCapacityMeter();

        updateWorkflowStep('wf-quadtree', true);
        updateWorkflowStep('wf-variance', true);
        updateWorkflowStep('wf-selection', true);
    }

    function updateQuadTreeImageDisplay() {
        if (!state.quadTreeData) return;
        const d = state.quadTreeData;
        let previewSrc = '';

        if (state.currentViewMode === 'division') previewSrc = d.previewDivision;
        else if (state.currentViewMode === 'selected') previewSrc = d.previewSelected;
        else if (state.currentViewMode === 'combined') previewSrc = d.previewCombined;
        else if (state.currentViewMode === 'original') previewSrc = state.imageMeta.previewOriginal;

        elements.quadtreeMainImg.src = previewSrc;
        elements.quadtreeMainImg.style.display = 'block';
        elements.quadtreePlaceholder.style.display = 'none';
    }

    function setupInteractiveCanvas() {
        if (!state.quadTreeData || !state.imageMeta) return;
        const canvas = elements.interactiveCanvas;
        const rect = elements.quadtreeMainImg.getBoundingClientRect();
        canvas.width = state.imageMeta.processedWidth;
        canvas.height = state.imageMeta.processedHeight;

        const ctx = canvas.getContext('2d');
        ctx.clearRect(0, 0, canvas.width, canvas.height);

        canvas.onmousemove = (e) => {
            const b = canvas.getBoundingClientRect();
            const scaleX = canvas.width / b.width;
            const scaleY = canvas.height / b.height;
            const mouseX = (e.clientX - b.left) * scaleX;
            const mouseY = (e.clientY - b.top) * scaleY;

            // Find matching leaf block
            const block = state.quadTreeData.blocks.find(blk =>
                mouseX >= blk.x && mouseX < blk.x + blk.size &&
                mouseY >= blk.y && mouseY < blk.y + blk.size
            );

            if (block) {
                // Highlight hovered block
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.strokeStyle = '#06b6d4';
                ctx.lineWidth = 2;
                ctx.strokeRect(block.x, block.y, block.size, block.size);

                // Update Inspector
                elements.blockInspectorPanel.style.display = 'block';
                elements.inspId.textContent = `#${block.id}`;
                elements.inspCoord.textContent = `(X: ${block.x}, Y: ${block.y})`;
                elements.inspSize.textContent = `${block.size} x ${block.size}`;
                elements.inspVariance.textContent = block.variance.toFixed(2);
                elements.inspStatus.textContent = block.selected ? 'Selected (Textured)' : 'Rejected (Smooth)';
                elements.inspStatus.className = block.selected ? 'text-success' : 'text-danger';
            }
        };

        canvas.onmouseleave = () => {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
        };
    }

    // HISTOGRAM & TABLE
    function renderHistogram(histogram) {
        elements.histogramBars.innerHTML = '';
        const maxVal = Math.max(...Object.values(histogram), 1);

        for (const [binName, count] of Object.entries(histogram)) {
            const barWrapper = document.createElement('div');
            barWrapper.className = 'h-bar-wrapper';

            const heightPercent = Math.max(4, (count / maxVal) * 100);
            barWrapper.innerHTML = `
                <span class="h-count">${count}</span>
                <div class="h-bar" style="height: ${heightPercent}%;"></div>
                <span class="h-label">${binName}</span>
            `;
            elements.histogramBars.appendChild(barWrapper);
        }
    }

    function populateBlocksTable(blocks) {
        elements.blocksTableBody.innerHTML = '';
        const displayLimit = Math.min(blocks.length, 120);

        for (let i = 0; i < displayLimit; ++i) {
            const b = blocks[i];
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>#${b.id}</td>
                <td>${b.x}</td>
                <td>${b.y}</td>
                <td>${b.size} x ${b.size}</td>
                <td>${b.variance.toFixed(2)}</td>
                <td><span class="status-tag ${b.selected ? 'selected' : 'rejected'}">${b.selected ? 'SELECTED' : 'REJECTED'}</span></td>
            `;
            elements.blocksTableBody.appendChild(row);
        }
    }

    function filterBlocksTable(query) {
        const rows = elements.blocksTableBody.querySelectorAll('tr');
        const q = query.toLowerCase();
        rows.forEach(r => {
            r.style.display = r.textContent.toLowerCase().includes(q) ? '' : 'none';
        });
    }

    // ==========================================
    // WATERMARK EMBEDDING
    // ==========================================
    function setupEmbeddingEvents() {
        elements.inputWmMessage.addEventListener('input', () => {
            updateCapacityMeter();
            updateBinaryPreview();
        });

        elements.btnExecuteEmbed.addEventListener('click', executeWatermarkEmbed);
        updateBinaryPreview();
    }

    function updateCapacityMeter() {
        const msg = elements.inputWmMessage.value;
        const msgLen = new TextEncoder().encode(msg).length;
        const payloadBits = msgLen * 8;
        const totalNeeded = 64 + payloadBits;
        const available = state.quadTreeData ? state.quadTreeData.selectedBlocks : 0;

        elements.payloadBitCount.textContent = `${payloadBits} bits`;
        elements.totalBitsNeeded.textContent = `${totalNeeded} bits`;
        elements.meterRatioText.textContent = `${totalNeeded} / ${available} bits`;

        let percent = available > 0 ? (totalNeeded / available * 100) : 0;
        elements.meterBarFill.style.width = `${Math.min(100, percent)}%`;

        if (totalNeeded > available) {
            elements.meterBarFill.classList.add('overflow');
            elements.capacityWarning.style.display = 'flex';
            elements.capacityWarningText.textContent = `Insufficient capacity: ${available} blocks available, but ${totalNeeded} bits required. Lower variance threshold or reduce minimum block size.`;
        } else {
            elements.meterBarFill.classList.remove('overflow');
            elements.capacityWarning.style.display = 'none';
        }
    }

    function updateBinaryPreview() {
        const msg = elements.inputWmMessage.value;
        const bytes = new TextEncoder().encode(msg);

        // Length in 32-bit binary
        let lenBin = '';
        for (let i = 31; i >= 0; --i) {
            lenBin += ((bytes.length >> i) & 1);
            if (i % 8 === 0 && i !== 0) lenBin += ' ';
        }
        elements.codeLenBits.textContent = lenBin;

        // Payload in binary
        let payloadBin = '';
        bytes.forEach((b, idx) => {
            for (let i = 7; i >= 0; --i) {
                payloadBin += ((b >> i) & 1);
            }
            payloadBin += ' ';
            if ((idx + 1) % 4 === 0) payloadBin += '\n';
        });
        elements.codePayloadBits.textContent = payloadBin || 'No message entered.';
    }

    async function executeWatermarkEmbed() {
        if (!state.hasImage || !state.quadTreeData) {
            showToast('Please run QuadTree division before embedding.', 'error');
            switchSection('quadtree');
            return;
        }

        const message = elements.inputWmMessage.value.trim();
        if (!message) {
            showToast('Please enter an ownership message.', 'error');
            return;
        }

        showToast(`Embedding "${message}" into C++ pixel buffer...`, 'info');

        try {
            const res = await fetch(`${state.apiBase}/api/embed`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ message })
            });
            const data = await res.json();

            if (data.success) {
                state.embeddingData = data;
                applyEmbeddingData(data);
                showToast(`Watermark embedded successfully (${data.embeddedBits} bits in ${data.embeddingTimeMs.toFixed(2)} ms)`, 'success');
                logActivity(`Embedded watermark "${message}" (${data.embeddedBits} bits, 64-bit header + payload) into anchor pixels`);
                switchSection('extract');
            } else {
                showToast(`Embedding failed: ${data.error}`, 'error');
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        }
    }

    function applyEmbeddingData(data) {
        // Watermarked image previews
        elements.imgCompareWatermarked.src = data.previewWatermarked;
        elements.imgCompareWatermarked.style.display = 'block';
        elements.phCompareWatermarked.style.display = 'none';

        elements.imgDifferenceMap.src = data.previewDifference;
        elements.imgDifferenceMap.style.display = 'block';
        elements.phDifferenceMap.style.display = 'none';

        elements.btnDownloadWatermarked.href = `${state.apiBase}/api/download/watermarked_image.png`;
        elements.btnDownloadManifest.href = `${state.apiBase}/api/download/watermark_manifest.txt`;

        elements.extOriginalSig.textContent = `"${data.message}"`;

        updateWorkflowStep('wf-embed', true);
        updateWorkflowStep('wf-watermarked', true);

        // Fetch metrics automatically
        fetchQualityMetrics();
    }

    // ==========================================
    // EXTRACTION & VERIFICATION
    // ==========================================
    function setupExtractionEvents() {
        elements.btnExecuteExtract.addEventListener('click', executeWatermarkExtract);
        elements.btnExecuteVerify.addEventListener('click', executeOwnershipVerify);
    }

    async function executeWatermarkExtract() {
        if (!state.embeddingData) {
            showToast('Please embed a watermark first.', 'error');
            switchSection('embed');
            return;
        }

        showToast('Extracting 64-bit header and payload bits from pixel LSBs...', 'info');

        try {
            const res = await fetch(`${state.apiBase}/api/extract`, { method: 'POST' });
            const data = await res.json();

            if (data.success) {
                state.extractionData = data;
                applyExtractionData(data);
                showToast(`Watermark extracted: "${data.extractedMessage}" (${data.bitsRead} bits)`, 'success');
                logActivity(`Extracted watermark: Magic: 0x${data.extractedMagic.toString(16).toUpperCase()}, Payload: "${data.extractedMessage}" (${data.extractedLength} bytes)`);
                // Auto verify
                executeOwnershipVerify();
            } else {
                showToast(`Extraction error: ${data.error}`, 'error');
                setVerificationState(false, 'EXTRACTION FAILED', data.error);
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        }
    }

    function applyExtractionData(data) {
        elements.extExtractedSig.textContent = `"${data.extractedMessage}"`;
        elements.extMagic.textContent = `0x${data.extractedMagic.toString(16).toUpperCase()} (${data.magicValid ? 'VALID' : 'INVALID'})`;
        elements.extLength.textContent = `${data.extractedLength} bytes`;
        elements.extTime.textContent = `${data.extractionTimeMs.toFixed(3)} ms`;

        elements.btnDownloadReport.href = `${state.apiBase}/api/download/extraction_report.txt`;
        updateWorkflowStep('wf-extract', true);
    }

    async function executeOwnershipVerify() {
        const orig = state.embeddingData ? state.embeddingData.message : elements.inputWmMessage.value;
        const ext = state.extractionData ? state.extractionData.extractedMessage : '';

        try {
            const res = await fetch(`${state.apiBase}/api/verify`, {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ originalMessage: orig, extractedMessage: ext })
            });
            const data = await res.json();

            elements.extBer.textContent = `${data.bitErrorRate.toFixed(2)}%`;
            elements.statBer.textContent = `BER: ${data.bitErrorRate.toFixed(2)}%`;

            if (data.verified) {
                setVerificationState(true, 'OWNERSHIP VERIFIED SUCCESSFULLY', data.statusText);
                elements.statVerification.textContent = 'VERIFIED';
                elements.statVerification.className = 'text-success';
                showToast('Ownership verified with 0% bit error rate!', 'success');
                logActivity('Ownership verification: SUCCESSFUL MATCH (Extracted matches Original)');
            } else {
                setVerificationState(false, 'VERIFICATION FAILED', data.statusText);
                elements.statVerification.textContent = 'MISMATCH';
                elements.statVerification.className = 'text-danger';
                showToast('Verification mismatch!', 'error');
                logActivity('Ownership verification: FAILED (Signatures mismatch)');
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        }
    }

    function setVerificationState(isSuccess, headline, subtext) {
        elements.verificationBanner.className = `verification-banner ${isSuccess ? 'success' : 'failed'}`;
        elements.verificationHeadline.textContent = headline;
        elements.verificationSubtext.textContent = subtext;
    }

    // ==========================================
    // QUALITY METRICS & DAA BENCHMARKS
    // ==========================================
    async function fetchQualityMetrics() {
        try {
            const res = await fetch(`${state.apiBase}/api/metrics`, { method: 'POST' });
            const data = await res.json();

            if (data.success) {
                state.qualityData = data;
                elements.qmPsnr.textContent = data.isInfinitePsnr ? 'Infinity dB' : `${data.psnr.toFixed(2)} dB`;
                elements.qmMse.textContent = data.mse.toFixed(6);
                elements.qmPixels.textContent = `${data.modifiedPixels} / ${data.totalPixels}`;
                elements.qmPixelsPercent.textContent = `${data.modifiedPercentage.toFixed(4)}% of canvas modified`;
                elements.qmBits.textContent = `${data.embeddedBits} bits`;
            }

            // Fetch DAA timings
            fetchDaaAnalysis();
        } catch (err) {}
    }

    async function fetchDaaAnalysis() {
        try {
            const res = await fetch(`${state.apiBase}/api/analysis`);
            const data = await res.json();

            if (data.success && data.timings) {
                const t = data.timings;
                elements.tQuadtree.textContent = `${t.quadTreeBuildMs.toFixed(3)} ms`;
                elements.tVariance.textContent = `${t.varianceCalcMs.toFixed(3)} ms`;
                elements.tEmbed.textContent = `${t.embeddingMs.toFixed(3)} ms`;
                elements.tExtract.textContent = `${t.extractionMs.toFixed(3)} ms`;
                elements.tMetrics.textContent = `${t.metricsCalcMs.toFixed(3)} ms`;
                elements.tTotal.textContent = `${t.totalPipelineMs.toFixed(3)} ms`;
            }
        } catch (err) {}
    }

    // ==========================================
    // RECURSION TREE SVG GENERATOR
    // ==========================================
    function renderRecursionTree() {
        const svg = elements.svgRecursionTree;
        svg.innerHTML = '';

        const width = 800;
        const height = 240;

        // Tree structure: Root (Depth 0) -> 4 Children (Depth 1) -> Sample Leaves (Depth 2)
        const root = { x: 400, y: 30, label: 'Root (256x256)', color: '#3b82f6' };
        const level1 = [
            { x: 130, y: 110, label: 'NW (128x128)', color: '#8b5cf6' },
            { x: 310, y: 110, label: 'NE (128x128)', color: '#8b5cf6' },
            { x: 490, y: 110, label: 'SW (128x128)', color: '#8b5cf6' },
            { x: 670, y: 110, label: 'SE (128x128)', color: '#8b5cf6' }
        ];

        // Draw connections Root -> Level 1
        level1.forEach(child => {
            const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            line.setAttribute('x1', root.x);
            line.setAttribute('y1', root.y + 12);
            line.setAttribute('x2', child.x);
            line.setAttribute('y2', child.y - 12);
            line.setAttribute('stroke', '#34425e');
            line.setAttribute('stroke-width', '2');
            svg.appendChild(line);
        });

        // Draw sample leaves below child 1
        const leaves = [
            { x: 70, y: 190, label: 'Leaf (16x16)', selected: false },
            { x: 130, y: 190, label: 'Leaf (16x16)', selected: true },
            { x: 190, y: 190, label: 'Leaf (16x16)', selected: true },
            { x: 670, y: 190, label: '...', selected: false }
        ];

        leaves.slice(0, 3).forEach(leaf => {
            const line = document.createElementNS('http://www.w3.org/2000/svg', 'line');
            line.setAttribute('x1', level1[0].x);
            line.setAttribute('y1', level1[0].y + 12);
            line.setAttribute('x2', leaf.x);
            line.setAttribute('y2', leaf.y - 12);
            line.setAttribute('stroke', '#232c3f');
            line.setAttribute('stroke-width', '1.5');
            line.setAttribute('stroke-dasharray', '3,3');
            svg.appendChild(line);
        });

        // Draw Root Node
        drawSvgNode(svg, root.x, root.y, root.label, root.color);

        // Draw Level 1 Nodes
        level1.forEach(n => drawSvgNode(svg, n.x, n.y, n.label, n.color));

        // Draw Leaves
        leaves.forEach(l => {
            const color = l.selected ? '#10b981' : '#ef4444';
            drawSvgNode(svg, l.x, l.y, l.label, color, l.selected ? 'Var \u2265 T' : 'Var < T');
        });
    }

    function drawSvgNode(svg, x, y, label, strokeColor, subLabel = '') {
        const g = document.createElementNS('http://www.w3.org/2000/svg', 'g');

        const rect = document.createElementNS('http://www.w3.org/2000/svg', 'rect');
        rect.setAttribute('x', x - 55);
        rect.setAttribute('y', y - 14);
        rect.setAttribute('width', 110);
        rect.setAttribute('height', 28);
        rect.setAttribute('rx', 6);
        rect.setAttribute('fill', '#161c2a');
        rect.setAttribute('stroke', strokeColor);
        rect.setAttribute('stroke-width', '1.5');
        g.appendChild(rect);

        const text = document.createElementNS('http://www.w3.org/2000/svg', 'text');
        text.setAttribute('x', x);
        text.setAttribute('y', y + 4);
        text.setAttribute('text-anchor', 'middle');
        text.setAttribute('fill', '#f0f4fc');
        text.setAttribute('font-size', '10');
        text.setAttribute('font-family', 'Inter, sans-serif');
        text.textContent = label;
        g.appendChild(text);

        if (subLabel) {
            const sub = document.createElementNS('http://www.w3.org/2000/svg', 'text');
            sub.setAttribute('x', x);
            sub.setAttribute('y', y + 26);
            sub.setAttribute('text-anchor', 'middle');
            sub.setAttribute('fill', strokeColor);
            sub.setAttribute('font-size', '9');
            sub.setAttribute('font-weight', 'bold');
            sub.textContent = subLabel;
            g.appendChild(sub);
        }

        svg.appendChild(g);
    }

    // ==========================================
    // VIVA ACCORDION
    // ==========================================
    function setupVivaAccordion() {
        const accordions = document.querySelectorAll('.accordion-header');
        accordions.forEach(header => {
            header.addEventListener('click', () => {
                const item = header.parentElement;
                item.classList.toggle('active');
            });
        });
    }

    // ==========================================
    // RUN COMPLETE DEMONSTRATION
    // ==========================================
    async function runFullDemonstration() {
        elements.btnRunDemo.disabled = true;
        elements.btnRunDemo.innerHTML = '<span class="loading-spinner"></span> Running Pipeline...';
        showToast('Executing automated end-to-end C++ demonstration...', 'info');

        try {
            const res = await fetch(`${state.apiBase}/api/demo`, { method: 'POST' });
            const data = await res.json();

            if (data.success) {
                // Apply all states
                state.hasImage = true;
                state.imageMeta = {
                    filename: 'sample_image.png',
                    originalWidth: 256,
                    originalHeight: 256,
                    processedWidth: 256,
                    processedHeight: 256,
                    fileSizeBytes: 65536,
                    wasResized: false,
                    transformNote: 'Synthetic benchmark native 256x256 square.'
                };

                // Update UI displays
                elements.imgPreviewColor.src = data.previewOriginal;
                elements.imgPreviewColor.style.display = 'block';
                elements.phPreviewColor.style.display = 'none';

                elements.imgCompareOriginal.src = data.previewOriginal;
                elements.imgCompareOriginal.style.display = 'block';
                elements.phCompareOriginal.style.display = 'none';

                elements.imgCompareWatermarked.src = data.previewWatermarked;
                elements.imgCompareWatermarked.style.display = 'block';
                elements.phCompareWatermarked.style.display = 'none';

                elements.imgDifferenceMap.src = data.previewDifference;
                elements.imgDifferenceMap.style.display = 'block';
                elements.phDifferenceMap.style.display = 'none';

                elements.btnDownloadWatermarked.href = `${state.apiBase}/api/download/watermarked_image.png`;
                elements.btnDownloadManifest.href = `${state.apiBase}/api/download/watermark_manifest.txt`;
                elements.btnDownloadReport.href = `${state.apiBase}/api/download/extraction_report.txt`;

                elements.statDimensions.textContent = '256 x 256';
                elements.statTotalLeaves.textContent = data.totalLeafBlocks;
                elements.statSelectedBlocks.textContent = data.selectedBlocks;
                elements.statCapacity.textContent = `${data.selectedBlocks} bits`;
                elements.statVerification.textContent = data.verified ? 'VERIFIED' : 'MISMATCH';
                elements.statVerification.className = data.verified ? 'text-success' : 'text-danger';

                // Extraction card
                elements.extOriginalSig.textContent = `"${data.demoMessage}"`;
                elements.extExtractedSig.textContent = `"${data.extractedMessage}"`;
                setVerificationState(data.verified, 'OWNERSHIP VERIFIED SUCCESSFULLY',
                    'Extracted watermark perfectly matches original digital signature with 0% bit error rate.');

                // Metrics
                elements.qmPsnr.textContent = data.psnr > 100 ? 'Infinity dB' : `${data.psnr.toFixed(2)} dB`;
                elements.qmMse.textContent = data.mse.toFixed(6);
                elements.qmBits.textContent = `${data.embeddedBits} bits`;

                // Re-run QuadTree to populate table and canvas
                await runQuadTree();

                // Highlight all workflow steps
                ['wf-upload', 'wf-quadtree', 'wf-variance', 'wf-selection', 'wf-embed', 'wf-watermarked', 'wf-extract']
                    .forEach(id => updateWorkflowStep(id, true));

                showToast('Demonstration completed successfully! All C++ algorithms verified.', 'success');
                logActivity(`Automated demonstration completed: "${data.demoMessage}" embedded & extracted in ${data.totalPipelineMs.toFixed(2)} ms total time.`);
                switchSection('extract');
            } else {
                showToast(`Demonstration error: ${data.error}`, 'error');
            }
        } catch (err) {
            showToast(`Connection error: ${err.message}`, 'error');
        } finally {
            elements.btnRunDemo.disabled = false;
            elements.btnRunDemo.innerHTML = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polygon points="5 3 19 12 5 21 5 3"></polygon></svg><span>Run Complete Demonstration</span>';
        }
    }

    function updateWorkflowStep(stepId, isActive) {
        const step = document.getElementById(stepId);
        if (step) step.classList.toggle('active', isActive);
    }

    // Launch
    init();
});

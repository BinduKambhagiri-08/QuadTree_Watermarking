Write-Output ">>> Testing /api/health"
$health = Invoke-RestMethod -Uri "http://localhost:8080/api/health"
Write-Output "Health: $($health.status) v$($health.version)"

Write-Output "`n>>> Testing /api/sample"
$sampleRes = Invoke-RestMethod -Uri "http://localhost:8080/api/sample"
Write-Output "Sample loaded: $($sampleRes.filename), $($sampleRes.processedWidth)x$($sampleRes.processedHeight)"

Write-Output "`n>>> Testing /api/quadtree"
$qdBody = @{ minBlockSize = 16; varianceThreshold = 100.0 } | ConvertTo-Json
$qdRes = Invoke-RestMethod -Uri "http://localhost:8080/api/quadtree" -Method Post -Body $qdBody -ContentType "application/json"
Write-Output "QuadTree: leaves=$($qdRes.totalLeafBlocks), selected=$($qdRes.selectedBlocks), depth=$($qdRes.maxRecursionDepth), time=$($qdRes.executionTimeMs) ms"

Write-Output "`n>>> Testing /api/embed"
$embedBody = @{ message = "COPYRIGHT 2026" } | ConvertTo-Json
$embRes = Invoke-RestMethod -Uri "http://localhost:8080/api/embed" -Method Post -Body $embedBody -ContentType "application/json"
Write-Output "Embed: success=$($embRes.success), bits=$($embRes.embeddedBits), capacity=$($embRes.availableCapacity), time=$($embRes.embeddingTimeMs) ms"

Write-Output "`n>>> Testing /api/extract"
$extRes = Invoke-RestMethod -Uri "http://localhost:8080/api/extract" -Method Post
Write-Output "Extract: success=$($extRes.success), magicValid=$($extRes.magicValid), msg='$($extRes.extractedMessage)', time=$($extRes.extractionTimeMs) ms"

Write-Output "`n>>> Testing /api/verify"
$verBody = @{ originalMessage = "COPYRIGHT 2026"; extractedMessage = $extRes.extractedMessage } | ConvertTo-Json
$verRes = Invoke-RestMethod -Uri "http://localhost:8080/api/verify" -Method Post -Body $verBody -ContentType "application/json"
Write-Output "Verify: verified=$($verRes.verified), BER=$($verRes.bitErrorRate)%, text=$($verRes.statusText)"

Write-Output "`n>>> Testing /api/metrics"
$metRes = Invoke-RestMethod -Uri "http://localhost:8080/api/metrics" -Method Post
Write-Output "Metrics: MSE=$($metRes.mse), PSNR=$($metRes.psnr) dB, modifiedPixels=$($metRes.modifiedPixels), bits=$($metRes.embeddedBits)"

Write-Output "`n>>> Testing /api/demo"
$demoRes = Invoke-RestMethod -Uri "http://localhost:8080/api/demo" -Method Post
Write-Output "Demo: success=$($demoRes.success), verified=$($demoRes.verified), PSNR=$($demoRes.psnr) dB"

Write-Output "`n>>> Testing /api/analysis"
$anaRes = Invoke-RestMethod -Uri "http://localhost:8080/api/analysis"
Write-Output "Analysis: recurrence=$($anaRes.recurrenceRelation), timeComplexity=$($anaRes.timeComplexity)"

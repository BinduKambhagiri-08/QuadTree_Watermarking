# DAA Complexity Analysis & Master Theorem Proof

## 1. The Recurrence Relation

The QuadTree algorithm recursively divides an $n \times n$ 2D region into four sub-regions of size $(n/2) \times (n/2)$.

The recurrence relation can be formulated as:

$$T(n) = 4 T\left(\frac{n}{2}\right) + f(n)$$

Where:
- $a = 4$: Number of recursive subproblems generated at each step (Top-Left, Top-Right, Bottom-Left, Bottom-Right).
- $b = 2$: The scaling factor by which each dimension is halved ($n \to n/2$).
- $f(n)$: The non-recursive work performed at each intermediate internal node (calculating sub-block coordinate offsets and dispatching function calls).

---

## 2. Master Theorem Case 1 Derivation

The standard Master Theorem applies to recurrences of the form:
$$T(n) = a T\left(\frac{n}{b}\right) + f(n)$$

### Step 1: Compute Critical Exponent
$$n^{\log_b a} = n^{\log_2 4} = n^2$$

### Step 2: Evaluate Non-Recursive Work $f(n)$
At internal nodes, the QuadTree only splits coordinates (`half = size / 2`, calculating 4 points):
$$f(n) = O(1)$$

To apply Master Theorem Case 1, $f(n)$ must be polynomially smaller than $n^{\log_b a}$:
$$f(n) = O\left(n^{\log_b a - \epsilon}\right) \quad \text{for some constant } \epsilon > 0$$

Setting $\epsilon = 2$:
$$O(1) = O\left(n^{2 - 2}\right) = O(n^0) = O(1)$$

### Step 3: Conclusion by Master Theorem Case 1
Because $f(n)$ is polynomially dominated by the leaves, Master Theorem Case 1 states:
$$T(n) = \Theta\left(n^{\log_b a}\right) = \Theta(n^2)$$

---

## 3. Important Technical Clarification for Evaluators & Viva

In an academic examination, viva examiners frequently ask:
*"Does computing variance inside the QuadTree increase the complexity beyond $\Theta(n^2)$?"*

### Case A: Actual Project Implementation (Variance Only at Leaf Blocks)
In our implementation, variance is calculated **strictly at the leaf nodes** once recursion reaches $size = S$ (e.g., $16 \times 16$).
1. The leaf blocks form a non-overlapping partition covering the complete $N \times N$ image.
2. Every pixel in the image belongs to exactly one leaf block.
3. Therefore, calculating the variance across all leaf blocks examines each pixel exactly twice (once for sum/mean, once for squared deviations):
   $$\text{Total Leaf Pixel Operations} = 2 \times N^2 = \Theta(N^2)$$
4. The number of internal nodes in a 4-ary tree with $L$ leaves is:
   $$\text{Internal Nodes} = \frac{L - 1}{3} = \frac{(N/S)^2 - 1}{3} = O(N^2)$$
5. Each internal node performs $O(1)$ coordinate halving operations.
6. Combining internal work + leaf variance:
   $$T(N) = O(N^2) + \Theta(N^2) = \mathbf{\Theta(N^2)}$$

**Conclusion:** The total time complexity is strictly linear with respect to the total number of pixels, which is $\Theta(N^2)$ for an $N \times N$ canvas.

### Case B: If Variance Were Recomputed at Every Intermediate Node
If an implementation recomputed variance over all pixels at every recursive level (from $N \times N$, down to $(N/2) \times (N/2)$, etc.):
- Level 0: 1 node of size $N \times N \implies N^2$ operations.
- Level 1: 4 nodes of size $(N/2) \times (N/2) \implies 4 \times (N/2)^2 = N^2$ operations.
- Level $k$: $4^k$ nodes of size $(N/2^k)^2 \implies N^2$ operations.
- Total levels $= \log_2(N/S) + 1$.
- Total operations $= (\log_2(N/S) + 1) \times N^2 = \mathbf{\Theta(N^2 \log N)}$.

Our design deliberately avoids redundant intermediate calculations by executing variance strictly at the base case, maintaining optimal **$\Theta(N^2)$** efficiency.

---

## 4. Space Complexity Analysis

### 4.1 Recursion Call Stack Depth
At each recursive step, the block dimension is halved:
$$size_k = \frac{N}{2^k}$$

Recursion halts when $size_k = S$ (minimum block size):
$$\frac{N}{2^k} = S \implies 2^k = \frac{N}{S} \implies k = \log_2\left(\frac{N}{S}\right)$$

Since each stack frame consumes $O(1)$ memory for local scalar variables (`x, y, size, half`), the maximum call stack depth is:
$$\text{Stack Depth} = O\left(\log_2\left(\frac{N}{S}\right)\right) = \mathbf{O(\log N)}$$

For $N = 256$ and $S = 16$:
$$\text{Max Depth} = \log_2(256 / 16) = \log_2(16) = \mathbf{4 \text{ stack frames}}$$

### 4.2 Auxiliary Storage
- Leaf block storage: Stores $(N/S)^2$ block structs. For $256 \times 256$ with $S = 16$, this is $(16)^2 = 256$ structs $\approx$ a few kilobytes.
- Total auxiliary space: $O((N/S)^2) = \mathbf{O(N^2)}$ in the worst case where $S=1$.

---

## 5. Summary Table

| Phase / Operation | Algorithm Paradigm | Time Complexity | Auxiliary Space |
| :--- | :--- | :--- | :--- |
| **Image Loading & Grayscale** | Linear Pixel Scan | $\Theta(N^2)$ | $O(N^2)$ |
| **QuadTree Decomposition** | Divide & Conquer | $\Theta(N^2)$ | $O(\log(N/S))$ stack |
| **Variance Scanning** | Statistical Reduction | $\Theta(N^2)$ | $O(1)$ |
| **LSB Watermark Embedding** | Sequential Anchor Access | $O(B)$ where $B \le (N/S)^2$ | $O(B)$ |
| **LSB Watermark Extraction** | Sequential Anchor Access | $O(B)$ | $O(B)$ |
| **MSE & PSNR Computation** | Full Matrix Comparison | $\Theta(N^2)$ | $O(1)$ |

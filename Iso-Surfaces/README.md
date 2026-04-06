# Marching Cubes 體積渲染器 (Volume Renderer)

這是一個基於 **OpenGL 3.3+** 與 **Marching Cubes 演算法** 實作的體積資料視覺化工具。本專案能夠讀取 `.raw` 格式的醫療或科學掃描數據（如 Carp, foot, BluntFin），計算指定等值（Isovalue）的三角面網格，並提供即時的切片（Slicing）與光照渲染功能。

## 🌟 核心技術亮點

### 1. Marching Cubes 演算法實作
* **等值面提取**：完整實作了從 $256^3$ 網格單元中提取等值面的邏輯。
* **線性插值**：透過 `VertexInterp` 函數在網格邊緣進行線性插值，精確計算交點座標以獲得平滑的表面。
* **查表加速**：利用 `edgeTable` 與 `triTable` 快速檢索 256 種可能的立方體相交組態。

### 2. 即時裁切與切片系統 (Clipping & Slicing)
透過 Vertex Shader (`vex.vs`) 中的平面方程式 $ax + by + cz + d = 0$ 進行動態裁切：
* **Whole**: 顯示完整生成的等值面模型。
* **Cross**: 裁切並隱藏平面一側的模型。
* **Slice**: 僅顯示平面附近特定厚度內的薄層切片。

### 3. 視覺與光照效果
* **Phong Lighting Model**: 在 Fragment Shader (`fram.fs`) 實作環境光、漫反射與鏡面反射。
* **多層渲染**：支援同時生成多個不同 Isovalue 的模型，並透過 Alpha Blending 實現半透明疊加效果。

## UI 操作指南 (Dear ImGui)

### 生成模型 (重要：此步驟不會自動更新)
由於 Marching Cubes 運算量較大，系統採**手動觸發**機制：
1. **Isovalue**: 在輸入框設定想要提取的數值（例如 30 代表軟組織，較高數值代表骨骼）。
2. **Color Picker**: 選擇該等值面的顏色與透明度 (Alpha)。
3. **OK!**: 點擊按鈕後，程式才會計算並渲染出新的模型網格。

### 攝影機與環境控制
* **Camera Transform**: 支援 X/Y/Z 位移與 Pitch/Yaw/Roll 旋轉。
* **Plane Control**: 調整滑桿 `a, b, c` 改變切面方向，`d` 改變切面位置。
* **Histogram**: 顯示資料數值分布的直方圖，輔助選擇合適的 Isovalue。

## 開發環境
* **語言**: C++
* **圖形庫**: OpenGL 3.3 (GLAD / GLFW)
* **數學庫**: GLM
* **UI 庫**: Dear ImGui
* **資料格式**: 8-bit 無符號整數原始體積數據 (.raw)

---

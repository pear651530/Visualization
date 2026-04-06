# Marching Cubes 體積渲染器 (Volume Renderer)

這是一個基於 **OpenGL 3.3+** 與 **Marching Cubes 演算法** 實作的體積資料視覺化工具。本專案能夠讀取 `.raw` 格式的醫療或科學掃描數據，計算指定等值（Isovalue）的三角面網格，並提供即時的切片（Slicing）與光照渲染功能。

## 核心功能

* [cite_start]**Marching Cubes 演算法實作**：完整實作了從 $256^3$ 網格單元中提取等值面的邏輯 [cite: 92, 114]。
* [cite_start]**多層等值面渲染**：支援同時設定多個不同的等值與顏色，並處理半透明（Alpha Blending）混合效果 [cite: 114, 188]。
* **即時橫切面控制 (Clipping & Slicing)**：
    * **Whole**: 顯示完整模型。
    * [cite_start]**Cross**: 顯示特定平面下的截斷模型 [cite: 14, 137]。
    * [cite_start]**Slice**: 僅顯示特定平面附近的薄層切片 [cite: 13, 138]。
* [cite_start]**互動式 UI 介面**：整合 **Dear ImGui**，提供直方圖顯示、攝影機變換與平面參數調整 [cite: 134, 141, 146]。
* [cite_start]**Phong 光照模型**：於 Fragment Shader 實作環境光、漫反射與鏡面反射 [cite: 5, 7, 8]。

## 技術棧

* [cite_start]**Graphics API**: OpenGL 330 Core Profile [cite: 1, 10]
* [cite_start]**Windowing/Input**: GLFW & GLAD [cite: 16, 186]
* [cite_start]**Math**: GLM (OpenGL Mathematics) [cite: 16]
* [cite_start]**UI**: Dear ImGui [cite: 16]
* [cite_start]**Loader**: `stb_image` & `ifstream` (用於二進位原始數據讀取) [cite: 16, 192]

## 控制指南

### 攝影機操作 (Camera)
* [cite_start]**X / Y / Z**: 平移攝影機位置 [cite: 142, 144]。
* [cite_start]**Pitch / Yaw / Roll**: 旋轉攝影機視角方向 [cite: 146, 158]。
* [cite_start]**Reset**: 一鍵回復初始攝影機狀態 [cite: 166]。

### 平面切片控制 (Clipping)
* [cite_start]**a / b / c**: 調整切面法向量的方向 [cite: 135]。
* [cite_start]**d**: 調整平面的位移距離 $D$ [cite: 136]。
* [cite_start]**Mode 切換**: 選擇整體、截斷或薄層切片模式 [cite: 137, 139]。

### 等值面生成
1.  [cite_start]**Isovalue**: 輸入想要提取的數值（如 30, 110 等） [cite: 139]。
2.  [cite_start]**Color Picker**: 選擇該層面的顏色與透明度 [cite: 139]。
3.  [cite_start]**OK!**: 執行 Marching Cubes 運算並生成網格 [cite: 140]。

## 檔案結構

* [cite_start]`vex.vs`: 頂點著色器，處理座標變換與平面裁切計算 [cite: 10, 15]。
* [cite_start]`fram.fs`: 片段著色器，處理材質光照與透明度渲染 [cite: 1, 9]。
* [cite_start]`程式碼法二_自由.txt`: C++ 主程式，包含 Marching Cubes 核心邏輯、UI 邏輯與數據讀取 [cite: 16, 69]。

## 演算法細節

[cite_start]專案中的 `Polygonise` 函數會根據 8 個頂點相對於等值面的狀態，查詢 `edgeTable` 與 `triTable` [cite: 23, 27, 69]。若發生交會，則透過 `VertexInterp` 進行線性插值以精確計算交點座標：

[cite_start]$$p = p_1 + \frac{isolevel - val_{p1}}{val_{p2} - val_{p1}} \times (p_2 - p_1)$$ [cite: 22]

---
[cite_start]*本專案預設讀取 $256 \times 256 \times 256$ 的體積數據 [cite: 92]。*

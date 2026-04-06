# 2D 流場流線可視化系統 (2D Streamline Visualization)

這是一個基於 **OpenGL 3.3** 核心配置開發的即時流場視覺化工具。本專案能夠讀取 `.vec` 格式的 2D 向量場數據，利用數值積分演算法追蹤流線（Streamlines），並透過自定義的著色器管線，將流速映射為顏色與線條粗細，呈現流體動態效果。<br>
![](imgs/streamline1.png)<br>
![](imgs/streamline2.png)

## 核心技術亮點

### 1. 流線追蹤演算法 (RK2 Integration)
* **RK2 (Runge-Kutta 2nd Order)**：實作 RK2 數值積分法（Heun's Method），在離散的向量場中精確計算粒子運動軌跡。
* **雙線性內插 (Bilinear Interpolation)**：透過雙線性內插函式計算非整數座標點上的向量值，確保流線轉向平滑且符合物理連續性。
* **追蹤控制**：支援調整積分步長 `h`、起始取樣點間隔 `gap` 以及流線長度的上下限過濾機制，有效管理計算負載與畫面雜訊。

### 2. Geometry Shader 線條強化技術
* **動態線條生成**：不同於預設的簡單線段繪製，本專案在 `geometry.gs` 中接收 `lines` 頂點，計算其垂直法向量並動態生成 `triangle_strip`，將原始線段擴展為具備厚度的矩形帶（Band）。
* **多維視覺映射**：
    * **線條粗細**：根據流線追蹤的頂點進度動態映射寬度 `thickness`，創造出視覺上的導向感。
    * **顏色映射**：將流速（Speed）進行歸一化後，透過片段著色器從 1D 傳遞函數紋理採樣，呈現出從冷色到暖色的流速分佈。

### 3. 即時互動 UI 系統
* **Dear ImGui 整合**：
    * **數據管理**：可即時切換不同的向量場測試檔案（如 `step5_velocity.vec`、`test_unit.vec` 等）。
    * **參數調校**：動態調整全域線條粗細倍率、積分步長與起始點密度，所有更改均會即時重新計算並渲染。
    * **攝影機系統**：提供完整的視角平移（X/Y/Z）與六軸旋轉（Pitch/Yaw/Roll）控制。

## 操作指南

### 1. 流場參數控制
* **filename**: 從下拉選單切換不同的向量場數據集。
* **Line Limit**: 設定流線生成的最小長度（過濾雜訊）與最大點數（限制長度）。
* **thickness (+/-)**: 手動縮放所有流線的視覺粗細。
* **gap / h_gap**: 調整流線起始點的分佈密度以及數值積分的步進精細程度。

### 2. 攝影機與視角
* **Transform (X/Y/Z)**: 調整觀察位置，支援 2D 平面位移與縮放感。
* **Rotation**: 調整攝影機觀測角度（支援 Pitch, Yaw, Roll）。
* **Reset Camera**: 將所有視角參數恢復至初始配置。

## 開發環境
* **語言**: C++
* **圖形庫**: OpenGL 3.3 Core Profile (GLAD / GLFW)
* **數學庫**: GLM
* **UI 庫**: Dear ImGui
* **支援格式**: 2D Vector field data (.vec)

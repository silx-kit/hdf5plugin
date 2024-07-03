# Accuracy Measures in Decibel

Note: The reported accuracy difference between QccPack and Sam's implementation is due to
an arithmetic coding step that is employed by QccPack, but not in Sam's implementation.


## Turbulence 128-cubed and 256-cubed

*bpp*     |   *QccPack*    |     *Sam*     | | *QccPack*  | *Sam*
----------|----------------|---------------|-|------------|-------
*0.25*    |   **39.92**    |     **39.20** | | **40.45**  | **39.57**  
*0.5*     |   **43.54**    |     **42.68** | | **44.02**  | **43.08**
*1*       |   **48.43**    |     **47.27** | | **48.93**  | **47.65**
*2*       |   **55.49**    |     **54.04** | | **56.04**  | **54.46**
*4*       |   **67.89**    |     **65.73** | | **68.55**  | **66.16**



# Speed Measures in Millisecond

Note: the reported speed numbers for QccPack implementation include the following
steps of operations:
1. Wavelet Transform,
2. SPECK Encoding,  
3. Arithmetic Encoding,
4. Output bitstream to disk,
5. Input bitstream from disk,
6. Arithmetic Decoding,
7. SPECK Decoding, and
8. Inverse Wavelet Transform.

While for Sam's implementation, there are no arithmetic coding steps. I.e.,
there is no step 3 and 6.


Also note, these tests were performed on `cisl-vapor` with an Intel Core-i7 Skylake CPU.
The compiler used was `gcc-5.4` with `-O2` optimization.
The column titled "Sam" was the 1st version of implementation, while the column
titled "Sam Tuned" was after tuning with `VTune Profiler`.

## Turbulence 128-cubed and 256-cubed

*bpp*     |     *QccPack*     |    *Sam*    | *Sam Tuned* |  |    *QccPack*    |    *Sam*    | *Sam Tuned*
----------|-------------------|-------------|-------------|--|-----------------|-------------|-------------
*0.25*    |     **465**       |    **183**  |  **172**    |  |      **3991**   |    **1655** | **1708**               
*0.5*     |     **734**       |    **223**  |  **224**    |  |      **6112**   |    **2001** | **2040**               
*1*       |     **1206**      |    **296**  |  **266**    |  |      **10303**  |    **2684** | **2455**               
*2*       |     **2002**      |    **446**  |  **466**    |  |      **17304**  |    **4172** | **4064**               
*4*       |     **3490**      |    **707**  |  **719**    |  |      **30003**  |    **6647** | **6602**              

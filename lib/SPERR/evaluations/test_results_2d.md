# Accuracy Measures in Decibel

Note: The reported accuracy difference between QccPack and Sam's implementation is due to
an arithmetic coding step that is employed by QccPack, but not in Sam's implementation.

## Lena512 (2D)

*bpp*  |    *QccPack*    |  *Sam*
-------| --------------- | ----------
*0.25* |     **32.74**   |      **32.52**
*0.5*  |     **35.85**   |      **35.63**
*1*    |     **39.02**   |      **38.80**
*2*    |     **43.61**   |      **43.29**



## Turbulence1024 (2D)

*bpp*    |    *QccPack*    |     *Sam*
---------|-----------------|-----------
*0.25*   |     **34.90**   |     **34.63**
*0.5*    |     **38.80**   |     **38.47**
*1*      |     **43.89**   |     **43.44**
*2*      |     **50.90**   |     **50.37**



## Turbulence128 (2D)

*bpp*     |   *QccPack*    |     *Sam*
----------|----------------|------------
*0.25*    |   **33.46**    |     **33.21**
*0.5*     |   **37.77**    |     **37.50**
*1*       |   **42.79**    |     **42.49**
*2*       |   **49.53**    |     **49.17**



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

While for Sam's implementation, there are no arithmetic coding steps ( i.e., 3 and 6 ).


Also note, these tests were performed on Sam's Chromebox with an Intel Celeron 1.4Ghz Haswell CPU.
The compiler used was `gcc-9.2.1` with `-O2` optimization on.

## Turbulence1024 (2D)

*bpp*     |     *QccPack*     |    *Sam*
----------|-------------------|----------
*0.25*    |     **375**       |    **241**
*2*       |     **1711**      |    **561**
*4*       |     **3213**      |    **881**



## Turbulence128 (2D)

*bpp*      |      *QccPack*    |    *Sam*
-----------|-------------------|--------------
*0.25*     |      **6**        |    **3**
*2*        |      **25**       |    **8**
*4*        |      **36**       |    **12**

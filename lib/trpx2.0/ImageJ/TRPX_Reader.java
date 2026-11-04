import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileInputStream;
import java.io.FileNotFoundException; 
import java.io.IOException;
import java.math.BigInteger;
import java.nio.file.Paths;
import java.nio.file.Path;
import java.util.Arrays;
import java.util.Scanner;
import java.util.regex.Pattern;
import ij.IJ;
import ij.ImagePlus;
import ij.process.ImageProcessor;
import ij.gui.NewImage;
import ij.ImageStack;
import ij.plugin.PlugIn;
import ij.io.OpenDialog;

public class TRPX_Reader implements PlugIn {

    public void run(String arg) {

    
        OpenDialog dialog = new OpenDialog("Select a Terse/Prolix data file", null);
        String filePath = dialog.getPath();
        if (filePath == null) return;  
        if (!filePath.toLowerCase().endsWith(".trpx")) return;
        
		// Parameters required for unpacking the file
		long prolixBits = 0;
        long signed=1;
    	long block=0;
        long terseDataSize = 0;
        long imageSize = 0;
        long dataStartIndex;
        int dim0 = 0;
        int dim1 = 0;
        int nFrames = 1;
        long[] frameSizes = null;
        long[] metadataSizes = null;

		
    	try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
    		String line;
    		dataStartIndex = 0;
       		while ((line = br.readLine()) != null) {
       		    int indexTerse = line.indexOf("<Terse ");
           		if (indexTerse == -1) 
           			dataStartIndex += line.length() + 1;
				else {
                    int indexEndTag = line.indexOf("/>", indexTerse) + 2;
                   	if (indexEndTag != -1) {
               			dataStartIndex += indexEndTag;
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
			            	scanner.findWithinHorizon("<.*?prolix_bits=\"(\\d+)\".*?/>", 0);
               				prolixBits = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?signed=\"(\\d+)\".*?/>", 0);
               				signed = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?block=\"(\\d+)\".*?/>", 0);
               				block = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
              				scanner.findWithinHorizon("<.*?memory_size=\"(\\d+)\".*?/>", 0);
               				terseDataSize = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				scanner.findWithinHorizon("<.*?number_of_values=\"(\\d+)\".*?/>", 0);
               				imageSize = Long.parseLong(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
               				if (scanner.findWithinHorizon("<.*?number_of_frames=\"(\\d+)\".*?/>", 0) != null) 
                				nFrames = Integer.parseInt(scanner.match().group(1));
               			}	
               			try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
              				if (scanner.findWithinHorizon("<.*?dimensions=\"(\\d+)(?:\\s+(\\d+))?(?:\\s+(\\d+))?\".*?/>", 0) != null) {
               				    dim0 = Integer.parseInt(scanner.match().group(1));
   								dim1 = scanner.match().group(2) != null ? Integer.parseInt(scanner.match().group(2)) : dim1;
               				} else {
	                   			dim0 = (int) Math.sqrt(imageSize);
                   				dim1 = dim0;
       						}
           				}
           				// Parse memory_sizes_of_frames if present
           				try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
           					if (scanner.findWithinHorizon("<.*?memory_sizes_of_frames=\"([^\"]+)\".*?/>", 0) != null) {
           						String[] sizes = scanner.match().group(1).trim().split("\\s+");
           						frameSizes = new long[sizes.length];
           						for (int i = 0; i < sizes.length; i++) {
           							frameSizes[i] = Long.parseLong(sizes[i]);
           						}
           					}
           				}
           				// Parse metadata_string_sizes if present
           				try (Scanner scanner = new Scanner(line.substring(indexTerse, indexEndTag))) {
           					if (scanner.findWithinHorizon("<.*?metadata_string_sizes=\"([^\"]+)\".*?/>", 0) != null) {
           						String[] sizes = scanner.match().group(1).trim().split("\\s+");
           						metadataSizes = new long[sizes.length];
           						for (int i = 0; i < sizes.length; i++) {
           							metadataSizes[i] = Long.parseLong(sizes[i]);
           						}
           					}
           				}
               		}
            		break;
           		}
       		}
   		}
   		catch (FileNotFoundException e) {return;}
   		catch (IOException e) {return;}
   		
   	
		if (signed != 0 || prolixBits > 16) {
    		System.err.println("Error: Invalid .trpx images for this plugin: images must be unsigned 16 bit.");
    		System.err.println("Data of " + filePath + " are " + ((signed == 0) ? "un" : "") + "signed " + prolixBits + " bit");
	    	return; 
		}

		if (terseDataSize <= 0 || imageSize <= 0 || dim0 <= 0 || dim1 <= 0) {
			System.err.println("Error: Invalid parameters in .trpx file - data size: " + terseDataSize + 
							   ", image size: " + imageSize + ", dimensions: " + dim0 + "x" + dim1);
			return;
		}
		
		long totalMetadataSize = 0;
		if (metadataSizes != null) {
			for (long size : metadataSizes) {
				totalMetadataSize += size;
			}
		}
		
	
		System.out.println("TRPX File Parameters:");
		System.out.println("  Prolix bits: " + prolixBits);
		System.out.println("  Signed: " + signed);
		System.out.println("  Block size: " + block);
		System.out.println("  Image size: " + imageSize);
		System.out.println("  Dimensions: " + dim0 + "x" + dim1);
		System.out.println("  Frames: " + nFrames);
		System.out.println("  Terse data size: " + terseDataSize);
		System.out.println("  Data start index: " + dataStartIndex);
		System.out.println("  Total metadata size: " + totalMetadataSize);

   	
		ImagePlus imageStack = NewImage.createShortImage(filePath, dim0, dim1, nFrames, NewImage.FILL_BLACK);
		
		
		dTerseData = new byte[(int) (dataStartIndex + totalMetadataSize + terseDataSize + 2)];
    	Path path = Paths.get(filePath);
    	try (FileInputStream fileInputStream = new FileInputStream(path.toFile())) {
    		fileInputStream.read(dTerseData);
 		} 
		catch (IOException e) { return; }
   		

		dBitStart = (dataStartIndex + totalMetadataSize) * 8;
		

		long currentByteOffset = dataStartIndex + totalMetadataSize;
		
		for (int frameNumber = 1; frameNumber <= nFrames; ++frameNumber) {
			System.out.println("Processing frame " + frameNumber + " at bit position: " + dBitStart);
			
			// Detect compression mode 
			int compressionMode = detectCompressionMode();
			System.out.println("Frame " + frameNumber + " compression mode: " + 
				(compressionMode == MODE_SIGNED ? "Signed" : 
				 compressionMode == MODE_UNSIGNED ? "Unsigned" : "Small_unsigned"));
			
			imageStack.setSlice(frameNumber);
			ImageProcessor ip = imageStack.getProcessor();
			short[] pixels = (short[]) ip.getPixels();
			
			if (compressionMode == MODE_UNSIGNED) {
				decompressUnsignedFrame(pixels, (int)imageSize, (int)block, (int)prolixBits);
			} else if (compressionMode == MODE_SMALL_UNSIGNED) {
				decompressSmallUnsignedFrame(pixels, (int)imageSize, (int)block, (int)prolixBits);
			} else {
			
				decompressSignedFrame(pixels, (int)imageSize, (int)block);
			}
			
			// Set start of next frame using frame sizes from XML if available
			if (frameSizes != null && frameNumber < nFrames) {
				currentByteOffset += frameSizes[frameNumber - 1];
				dBitStart = currentByteOffset * 8;
				System.out.println("Next frame starts at byte " + currentByteOffset + " (bit " + dBitStart + ")");
			}
			
			
		}


        imageStack.show();
        IJ.run(imageStack, "Enhance Contrast", "saturated=0.35"); 
        imageStack.updateAndDraw();
	}

	private long dBitStart;
	private byte[] dTerseData;
	

	private static final int MODE_SIGNED = 0;
	private static final int MODE_UNSIGNED = 1;
	private static final int MODE_SMALL_UNSIGNED = 2;


	private int detectCompressionMode() {
		// Peek at the next 18 bits to check if it's a mode prefix
		long savedBitStart = dBitStart;
		int prefix = ToUnsignedInt(18);  
		
		System.out.println("Checking 18-bit value at bit " + savedBitStart + ": 0b" + 
			String.format("%18s", Integer.toBinaryString(prefix)).replace(' ', '0') + 
			" (decimal: " + prefix + ")");
		
		if (prefix == 0b111111111111111100) {  
			// Small_unsigned mode - prefix consumed, don't rewind
			System.out.println("Detected: Small_unsigned mode (prefix consumed)");
			return MODE_SMALL_UNSIGNED;
		} else if (prefix == 0b111111111111111000) {  
			// Unsigned mode - prefix consumed, don't rewind
			System.out.println("Detected: Unsigned mode (prefix consumed)");
			return MODE_UNSIGNED;
		} else {
			// Signed mode - NO prefix exists, so rewind to start of frame data
			System.out.println("Detected: Signed mode (no prefix, rewinding to bit " + savedBitStart + ")");
			dBitStart = savedBitStart;
			return MODE_SIGNED;
		}
	}

	
	private void decompressSignedFrame(short[] pixels, int imageSize, int block) {
		short significant_bits = 0;
		for (int from = 0; from < imageSize; from += block) {
			if (0 == ToShort(1)) 
				if (7 == (significant_bits = ToShort(3))) 
					if (10 == (significant_bits += ToShort(2))) 
						significant_bits += ToShort(6);
			int to = (int) Math.min(imageSize, from + block);
			if (significant_bits == 0) 
				Arrays.fill(pixels, from, to, (short)0);
			else 
				for (int j = from; j < to; ++j) 
					pixels[j] = ToShort(significant_bits);
		}
	}
	
	
	private void decompressUnsignedFrame(short[] pixels, int imageSize, int block, int prolixBits) {
		short significant_bits = 0;
		short masked_bits = 0;
		
		for (int from = 0; from < imageSize; from += block) {
			if (0 == ToShort(1)) 
				if (7 == (significant_bits = ToShort(3))) 
					if (10 == (significant_bits += ToShort(2))) 
						significant_bits += ToShort(6);
			
			int to = (int) Math.min(imageSize, from + block);
			
			if (significant_bits != prolixBits) {
				if (significant_bits == 0) 
					Arrays.fill(pixels, from, to, (short)0);
				else 
					for (int j = from; j < to; ++j) 
						pixels[j] = ToShort(significant_bits);
			} else {
				if (0 == ToShort(1)) 
					if (7 == (masked_bits = ToShort(3))) 
						if (10 == (masked_bits += ToShort(2))) 
							masked_bits += ToShort(6);
				

				for (int j = from; j < to; ++j) {
					int value = ToShort(masked_bits) - 1;
					pixels[j] = (short) Math.max(0, value); 
				}
			}
		}
	}

    private short ToShort(int s) { 
		int indx = (int) dBitStart >> 3;          
		
		if (indx + 2 >= dTerseData.length) {
			System.err.println("Warning: Trying to read beyond data bounds. Index: " + indx + 
							   ", Array length: " + dTerseData.length + ", Bit position: " + dBitStart);
			dBitStart += s;
			return 0;
		}
		
		int trpx = ((dTerseData[indx] & 0xFF) +
                	((dTerseData[1 + indx] & 0xFF) << 8) +
                    ((dTerseData[2 + indx] & 0xFF) << 16));
        short result = (short)((trpx >>> (dBitStart & 7)) & ((1 << s) - 1));
        dBitStart += s;
        return result;
      }
      
 
      private int ToUnsignedInt(long s) { 
		int indx = (int) dBitStart >> 3;          
		
		if (indx + 4 >= dTerseData.length) {
			System.err.println("Warning: Trying to read beyond data bounds for unsigned int. Index: " + indx + 
							   ", Array length: " + dTerseData.length + ", Bit position: " + dBitStart);
			dBitStart += s;
			return 0;
		}
		
		long trpx = ((dTerseData[indx] & 0xFFL) +
                	((dTerseData[1 + indx] & 0xFFL) << 8) +
                    ((dTerseData[2 + indx] & 0xFFL) << 16) +
                    ((dTerseData[3 + indx] & 0xFFL) << 24));
        int result = (int)((trpx >>> (dBitStart & 7)) & ((1L << s) - 1));
        dBitStart += s;
        return result;
      }
      
      private BigInteger ToBigInteger(long numBits) {
		if (numBits <= 0) return BigInteger.ZERO;
		if (numBits <= 32) return BigInteger.valueOf(ToUnsignedInt(numBits));
		
		BigInteger result = BigInteger.ZERO;
		long bitsRemaining = numBits;
		int shift = 0;
		
		while (bitsRemaining > 0) {
			long bitsToRead = Math.min(32, bitsRemaining);
			long chunk = ToUnsignedInt(bitsToRead);
			result = result.or(BigInteger.valueOf(chunk).shiftLeft(shift));
			shift += (int)bitsToRead;
			bitsRemaining -= bitsToRead;
		}
		
		return result;
      }
      
      // Small_unsigned decompression 
      private void decompressSmallUnsignedFrame(short[] pixels, int imageSize, int block, int prolixBits) {
		
		
		int effectiveBlock = Math.min(block, 24); 
		int bits = 0;
		int max = 0;
		
		for (int from = 0; from < imageSize; from += effectiveBlock) {
			int to = Math.min(imageSize, from + effectiveBlock);
			int blockSize = to - from;
			
			
			int[] result = getMaxValueSmallUnsigned(max, bits);
			max = result[0];
			bits = result[1];
			

		
			switch (max) {
				case 0:
					// All zeros
					Arrays.fill(pixels, from, to, (short)0);
					break;
				case 1:
					// 1-bit per value
					for (int j = from; j < to; j++) {
						pixels[j] = (short)ToShort(1);
					}
					break;
				case 2:
					// Base-3 encoding
					decompressBase3BlockSmall(pixels, from, to, effectiveBlock);
					break;
				case 3:
					// 2-bit per value  
					for (int j = from; j < to; j++) {
						pixels[j] = (short)ToShort(2);
					}
					break;
				case 7:
					// 3-bit per value
					for (int j = from; j < to; j++) {
						pixels[j] = (short)ToShort(3);
					}
					break;
				default:	
					if (max < 7) {
						// Base-(max+1) encoding for small values
						decompressBaseNBlockSmall(pixels, from, to, max, effectiveBlock);
					} else if (bits == prolixBits) {

						decompressSmallUnsignedMaskedData(pixels, from, imageSize, effectiveBlock, prolixBits, max, bits);
						return; 
					} else {
						// Regular bit encoding for larger values
						for (int j = from; j < to; j++) {
							pixels[j] = (short)ToShort(bits);
						}
						max = 0xFFFF / 2;
					}
					break;
			}
		}
	}
	
	private int[] getMaxValueSmallUnsigned(int max, int bits) {
		int flag = ToShort(1);
		if (flag == 1 && max == 0) {
			return new int[]{max, bits};
		}
		
		flag = (flag << 1) + ToShort(1);
		if (flag == 0b11) {
			// Same as previous block
			return new int[]{max, bits};
		}
		if (flag == 0b10) {

				bits--;
				max--;
		} else if (flag == 0b01) {
			// Increment from previous
			bits++;
			max = (max == 6) ? max - 2 : max + 1;
		} else {
			// flag == 0b00: read new value
			max = bits = ToShort(3);
			if (bits == 7) {
				max = 0xFFFF / 2;
				bits = 3 + ToShort(3);
				if (bits == 10) {
					bits += ToShort(3);
					if (bits == 17) {
						bits += ToShort(6);
					}
				}
			}
		}
		return new int[]{max, bits};
	}	
	private void decompressBase3BlockSmall(short[] pixels, int from, int to, int effectiveBlock) {
		int blockSize = to - from;
		long numBits = (long)Math.ceil(effectiveBlock * Math.log(3) / Math.log(2));
		BigInteger val = ToBigInteger(numBits);
		BigInteger three = BigInteger.valueOf(3);
		for (int i = 0; i < blockSize; i++) {
			pixels[from + i] = (short)(val.mod(three).intValue());
			val = val.divide(three);
		}
	}
	

	private void decompressBaseNBlockSmall(short[] pixels, int from, int to, int max, int effectiveBlock) {
		int blockSize = to - from;
		int base = max + 1;
		long numBits = (long)Math.ceil(effectiveBlock * Math.log(base) / Math.log(2));
		BigInteger val = ToBigInteger(numBits);
		BigInteger bigBase = BigInteger.valueOf(base);
		for (int i = 0; i < blockSize; i++) {
			pixels[from + i] = (short)(val.mod(bigBase).intValue());
			val = val.divide(bigBase);
		}
	}
	
	

	private void decompressSmallUnsignedMaskedData(short[] pixels, int from, int imageSize, int effectiveBlock, int prolixBits, int max, int bits) {
		int to = Math.min(imageSize, from + effectiveBlock);
		
		for (int j = from; j < to; j++) {
			int value = ToShort(bits) - 1;
			pixels[j] = (short) Math.max(0, value);
		}
		
		from += effectiveBlock;
		
		max = 0xFFFF / 2;
		bits = prolixBits + 2;
		
		
		while (from < imageSize) {
			to = Math.min(imageSize, from + effectiveBlock);
			
			int[] result = getMaxValueSmallUnsigned(max, bits);
			max = result[0];
			bits = result[1];
			
			if (max >= 7) {
				// Read values with 'bits' bits each, then subtract 1
				for (int j = from; j < to; j++) {
					int value = ToShort(bits) - 1;
					pixels[j] = (short) Math.max(0, value);
				}
				max = 0xFFFF / 2;
			} else {

				int blockSize = to - from;
				int base = max + 1;
				long numBits = (long)Math.ceil(effectiveBlock * Math.log(base) / Math.log(2));
				BigInteger val = ToBigInteger(numBits);
				BigInteger bigBase = BigInteger.valueOf(base);
				for (int i = 0; i < blockSize; i++) {
					int decodedVal = val.mod(bigBase).intValue() - 1;
					pixels[from + i] = (short) Math.max(0, decodedVal);
					val = val.divide(bigBase);
				}
			}
			
			from += effectiveBlock;
			
			if (from < imageSize) {
				int masked = ToShort(1);
				if (masked == 0) {
					max = 0;
					bits = 0;
					break;
				}
			}
		}
	}
	

	private long integerPower(int base, int exp) {
		long result = 1;
		long LongBase = base;
		while (exp > 0) {
			if ((exp & 1) == 1) result *= LongBase;
			LongBase *= LongBase;
			exp >>= 1;
		}
		return result;
	}
	
	private int mostSignificantBit(long val) {
		int r = 0;
		while (val > 0) {
			val >>= 1;
			r++;
		}
		return r;
	}
 }

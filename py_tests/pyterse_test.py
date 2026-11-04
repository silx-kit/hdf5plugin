import unittest
import numpy as np
import io
import re
import shutil
import sys
import os
import tempfile
import subprocess

for root, dirs, files in os.walk('.'):
    if '__pycache__' in dirs:
        shutil.rmtree(os.path.join(root, '__pycache__'))

sys.path.append(os.path.join(os.getcwd(), 'build', 'pyterse/'))

from pyterse import Terse, TerseMode

class TestTerseLibrary(unittest.TestCase):
    def setUp(self):
        # Common test data
        self.test_data_1d = np.array([1, 2, 3, 4, 5], dtype=np.int32)
        self.test_data_2d = np.array([[1, 2, 3], [4, 5, 6]], dtype=np.int32)
        self.test_data_3d = np.array([[[1, 2], [3, 4]], [[5, 6], [7, 8]]], dtype=np.int32)

    def test_basic_construction(self):
        """Test basic Terse object construction"""
        terse = Terse()
        self.assertEqual(terse.number_of_frames, 0)
        self.assertEqual(terse.size, 0)

    def test_construction_with_data(self):
        """Test Terse construction with initial data"""
        terse = Terse(self.test_data_1d, TerseMode.SIGNED)
        self.assertEqual(terse.size, len(self.test_data_1d))
        self.assertTrue(terse.is_signed)

    def test_data_types(self):
        """Test different numpy data types"""
        for dtype in [np.int8, np.uint8, np.int16, np.uint16, np.int32, np.uint32, np.int64, np.uint64]:
            data = np.array([1, 2, 3], dtype=dtype)
            terse = Terse(data, TerseMode.SIGNED)
            result = terse.prolix()
            np.testing.assert_array_equal(result, data)
            self.assertEqual(result.dtype, data.dtype)

    @unittest.skipUnless(os.path.isdir("/proc/self/task"), "requires /proc/self/task (Linux)")
    def test_import_starts_no_threads(self):
        """Test that importing pyterse does not start the thread pool"""
        # The thread pool used to be created at program start-up, i.e. by
        # loading the module, instead of when it is used first.
        code = ("import os, sys, numpy\n"
                "sys.path.insert(0, sys.argv[1])\n"
                "before = len(os.listdir('/proc/self/task'))\n"
                "import pyterse\n"
                "print(before, len(os.listdir('/proc/self/task')))\n")
        path = os.path.dirname(sys.modules["pyterse"].__file__)
        before, after = subprocess.run([sys.executable, "-c", code, path], check=True, capture_output=True,
                                       text=True).stdout.split()
        self.assertEqual(after, before)

    def test_dimensions(self):
        """Test handling of different dimensional arrays"""
        # 1D
        terse_1d = Terse(self.test_data_1d)
        np.testing.assert_array_equal(terse_1d.prolix(), self.test_data_1d)
        
        # 2D
        terse_2d = Terse(self.test_data_2d)
        np.testing.assert_array_equal(terse_2d.prolix(), self.test_data_2d)
        
        # 3D
        terse_3d = Terse(self.test_data_3d)
        np.testing.assert_array_equal(terse_3d.prolix(), self.test_data_3d)

    def test_compression_modes(self):
        """Test different compression modes"""
        data = np.array([1, 2, 3], dtype=np.int32)
        for mode in [TerseMode.SIGNED, TerseMode.UNSIGNED, TerseMode.SMALL_UNSIGNED, TerseMode.DEFAULT]:
            terse = Terse(data, mode)
            np.testing.assert_array_equal(terse.prolix(), data)

    def test_header_value_lists(self):
        """Test the lists of values in the header"""
        # The values are separated by spaces, without a space after the last
        # value. Writing them compared with the end iterator advanced past the
        # end (undefined behaviour), which was always true and added a space.
        data = np.arange(2 * 24, dtype=np.uint16).reshape(2, 24)
        terse = Terse()
        for frame in data:
            terse.push_back(frame)
        stream = io.BytesIO()
        terse.write(stream)
        header = stream.getvalue().split(b"/>")[0].decode()
        self.assertIn('dimensions="24"', header)
        self.assertNotRegex(header, r'\s"')
        # Headers written with a space after the last value are read as well.
        old = re.sub(rb'(dimensions|memory_sizes_of_frames|metadata_string_sizes)="([^"]*)"', rb'\1="\2 "',
                     stream.getvalue(), count=3)
        self.assertNotEqual(old, stream.getvalue())
        for value in [stream.getvalue(), old]:
            np.testing.assert_array_equal(Terse(io.BytesIO(value)).prolix(), data)

    def test_frame_operations(self):
        """Test frame-based operations"""
        # Insert frames
        terse = Terse()
        terse.push_back(self.test_data_1d)
        terse.insert(0, self.test_data_1d)
        self.assertEqual(terse.number_of_frames, 2)
        
        # Access frame
        frame = terse.at(0)
        np.testing.assert_array_equal(frame.prolix(), self.test_data_1d)
        
        # Erase frame
        terse.erase(0)
        self.assertEqual(terse.number_of_frames, 1)

    def test_metadata(self):
        """Test metadata operations"""
        terse = Terse(self.test_data_1d)
        test_metadata = "<metadata>test</metadata>"
        terse.set_metadata(0, test_metadata)
        self.assertEqual(terse.metadata(0), test_metadata)

    def test_stream_metadata(self):
        """Test reading metadata from a stream"""
        # A short string is stored in the std::string object itself, a long
        # one in memory allocated by it.
        metadata = ["<m>short</m>", "<metadata>" + "long " * 20 + "</metadata>"]
        data = np.arange(2 * 24, dtype=np.uint16).reshape(2, 24)
        terse = Terse()
        for frame, frame_metadata in zip(data, metadata):
            terse.push_back(frame)
            terse.set_metadata(terse.number_of_frames - 1, frame_metadata)
        stream = io.BytesIO()
        terse.write(stream)
        stream.seek(0)
        terse_read = Terse(stream)
        self.assertEqual([terse_read.metadata(i) for i in range(2)], metadata)
        np.testing.assert_array_equal(terse_read.prolix(), data)

    def test_dimension_operations(self):
        """Test dimension-related operations"""
        terse = Terse(self.test_data_2d)
        original_dims = terse.dim()
        
        # Test dimension setting
        new_dims = [6, 1]  # Same total size as 2x3
        terse.set_dim(new_dims)
        self.assertEqual(terse.dim(), new_dims)
        
        # Test invalid dimension setting
        with self.assertRaises(ValueError):
            terse.set_dim([2, 2])  # Invalid total size

    def test_stream_operations(self):
        """Test stream reading and writing"""
        # Write to stream
        stream = io.BytesIO()
        terse_write = Terse(self.test_data_1d)
        terse_write.write(stream)
        
        # Read from stream
        stream.seek(0)
        terse_read = Terse(stream)
        np.testing.assert_array_equal(terse_read.prolix(), self.test_data_1d)

    @staticmethod
    def prolix_frames(*terses):
        """Decompress all frames of the Terse objects into one flat array"""
        # Frame by frame, as prolix() of a Terse object with several frames
        # but without dimensions (e.g. loaded from a file) expects one frame.
        return np.concatenate([terse.at(i).prolix().ravel()
                               for terse in terses for i in range(terse.number_of_frames)])

    def test_short_last_block(self):
        """Test decoding frames that end with a short block of small values"""
        # 52 values as written by the HDF5 filter of commit 93cf8e4, but with
        # frames of 16 instead of 2^18 values: a Terse object with 3 frames of
        # 16 values followed by a Terse object with the remaining 4 values. As
        # 16 (and 2^18) is not a multiple of the block size of 12, every frame
        # ends with a short block. The encoder pads the frames to a multiple of
        # 8 bytes with zeros; here the padding is filled with ones, which must
        # not change the decoded values.
        values = np.array([0, 0, 1, 2, 0, 2, 2, 1, 2, 1, 2, 1, 0, 0, 3, 1, 0, 2,
                           0, 2, 1, 4, 2, 1, 1, 0, 0, 0, 0, 0, 0, 1, 2, 4, 2, 2,
                           3, 3, 0, 0, 3, 0, 4, 3, 2, 0, 1, 3, 0, 1, 1, 2])
        frames = (b"\xfc\xff\x23\xea\x88\x12\x0e\xff"
                  b"\xfc\xff\xc3\xb4\xea\x03\x20\x08"
                  b"\xfc\xff\x43\xa9\xa3\x90\x4d\x1a")
        rest = b"\xfc\xff\x23\x21\xff\xff\xff\xff"
        for dtype in [np.uint16, np.uint32]:
            bits = 8 * np.dtype(dtype).itemsize
            with tempfile.TemporaryDirectory() as tmp:
                frames_file = os.path.join(tmp, "frames.trpx")
                rest_file = os.path.join(tmp, "rest.trpx")
                with open(frames_file, "wb") as f:
                    f.write(f'<Terse prolix_bits="{bits}" signed="0" block="12" number_of_values="16" number_of_frames="3" '
                            f'memory_sizes_of_frames="8 8 8 " memory_size="24" metadata_string_sizes="0 0 0 "/>'.encode() + frames)
                with open(rest_file, "wb") as f:
                    f.write(f'<Terse prolix_bits="{bits}" signed="0" block="12" number_of_values="4" number_of_frames="1" '
                            f'memory_sizes_of_frames="8 " memory_size="8" metadata_string_sizes="0 "/>'.encode() + rest)
                terse_frames = Terse.load(frames_file)
                terse_rest = Terse.load(rest_file)
            result = self.prolix_frames(terse_frames, terse_rest)
            self.assertEqual(result.dtype, dtype)
            np.testing.assert_array_equal(result, values)

    def test_hdf5_filter_frames(self):
        """Test round trip of an image split into frames like the HDF5 filter"""
        # An Eiger 4M sized image of small Poisson distributed values, split
        # into frames of 2^18 values (the last block of each frame is short)
        # and a Terse object with the rest.
        rng = np.random.default_rng(0)
        image = rng.poisson(5.0, size=(2167, 2070))
        chunk = 1 << 18
        for dtype in [np.uint16, np.uint32]:
            data = image.astype(dtype).ravel()
            terse_frames = Terse()
            pos = 0
            while pos + chunk < data.size:
                terse_frames.push_back(data[pos:pos + chunk])
                pos += chunk
            with tempfile.TemporaryDirectory() as tmp:
                frames_file = os.path.join(tmp, "frames.trpx")
                rest_file = os.path.join(tmp, "rest.trpx")
                terse_frames.save(frames_file)
                Terse(data[pos:]).save(rest_file)
                terse_frames = Terse.load(frames_file)
                terse_rest = Terse.load(rest_file)
            # Decode several times as wrong values may depend on the memory
            # after the frames.
            for _ in range(5):
                result = self.prolix_frames(terse_frames, terse_rest)
                np.testing.assert_array_equal(result, data)

    def test_saturated_module_gaps(self):
        """Test round trip of an image with saturated module gaps"""
        # An Eiger 4M like image of small Poisson distributed values with
        # module gaps set to the maximum value. With 2166 instead of 2167 rows
        # the number of values is a multiple of the block size of 12.
        rng = np.random.default_rng(0)
        image = rng.poisson(5.0, size=(2166, 2070))
        gaps = (np.arange(2166) % 551 >= 514)[:, None] | (np.arange(2070) % 1040 >= 1030)
        for dtype in [np.uint16, np.uint32]:
            data = np.where(gaps, np.iinfo(dtype).max, image).astype(dtype)
            np.testing.assert_array_equal(Terse(data).prolix(), data)

    def test_saturated_block_followed_by_small_values(self):
        """Test round trip of a saturated block followed by blocks of small values"""
        # The maximum value + 1 wraps around to 0 for 32 bit values.
        for dtype in [np.uint16, np.uint32]:
            data = np.repeat(np.array([np.iinfo(dtype).max, 0, 1, 0, 2], dtype=dtype), 12)
            np.testing.assert_array_equal(Terse(data).prolix(), data)

    def test_compression_settings(self):
        """Test compression settings"""
        terse = Terse()
        
        # Test block size
        terse.set_block_size(1024)
        self.assertEqual(terse.block_size(), 1024)
        
        # Test fast mode
        terse.set_fast(True)
        self.assertTrue(terse.fast())
        
        # Test small mode
        terse.set_small(True)
        self.assertTrue(terse.small())
        
        # Test degree of parallelism
        terse.set_dop(0.5)
        self.assertEqual(terse.dop(), 0.5)

    def test_random_bit_widths(self):
        """Test round trip of values with random bit widths"""
        # Blocks with random numbers of significant bits and saturated values
        # need more space than the uncompressed values. The buffer for the
        # compressed frame is grown while compressing, which could write past
        # its end. Only detected with AddressSanitizer, as the values are
        # still correct.
        rng = np.random.default_rng(0)
        for dtype in [np.uint8, np.uint16, np.uint32]:
            width = 8 * np.dtype(dtype).itemsize
            for n in range(1, 401):
                bits = rng.integers(1, width + 1, size=n)
                data = (rng.integers(0, 2**63, size=n, dtype=np.uint64) >> (63 - bits).astype(np.uint64)).astype(dtype)
                data[rng.random(n) < 0.125] = np.iinfo(dtype).max
                np.testing.assert_array_equal(Terse(data, TerseMode.UNSIGNED).prolix(), data)

    def test_full_range_32_bit_values(self):
        """Test round trip of 32 bit values using all bits"""
        # The bit masks were computed with (1ul << bits) - 1, which is
        # undefined for 32 bits where unsigned long has 32 bits (Windows).
        # Signed 32 bit values then came back wrong.
        rng = np.random.default_rng(0)
        for dtype, modes in [(np.uint32, [TerseMode.DEFAULT, TerseMode.UNSIGNED]),
                             (np.int32, [TerseMode.DEFAULT, TerseMode.SIGNED])]:
            info = np.iinfo(dtype)
            data = rng.integers(info.min, info.max, size=1200, endpoint=True, dtype=dtype)
            for mode in modes:
                np.testing.assert_array_equal(Terse(data, mode).prolix(), data)

    def test_error_handling(self):
        """Test error handling"""
        terse = Terse(self.test_data_1d)
        
        # Test invalid frame access
        with self.assertRaises(IndexError):
            terse.at(999)
        
        # Test invalid frame insertion
        with self.assertRaises(ValueError):
            terse.insert(999, self.test_data_1d)
        
        # Test dimension mismatch
        different_shape = np.array([[1, 2], [3, 4]], dtype=np.int32)
        with self.assertRaises(ValueError):
            terse.push_back(different_shape)

    def test_memory_management(self):
        """Test memory management features"""
        terse = Terse(self.test_data_1d)
        original_size = terse.terse_size
        terse.shrink_to_fit()
        self.assertLessEqual(terse.terse_size, original_size)

if __name__ == '__main__':
    unittest.main(verbosity=2)
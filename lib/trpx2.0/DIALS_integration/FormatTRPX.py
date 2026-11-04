from __future__ import annotations

import os
import re
import xml.etree.ElementTree as ET
import numpy as np
import sys


sys.path.append(os.path.join(os.getcwd(), 'build'))

from pyterse import Terse  

from scitbx.array_family import flex
from dxtbx.format.Format import Format
from dxtbx.model import ScanFactory
from dxtbx.model.beam import Probe


class FormatTRPX(Format):
    @staticmethod
    def understand(image_file):
        try:
            with FormatTRPX.open_file(image_file, "rb") as fh:
                header = fh.read(24)
                print(f"Header: {header}")  

        except OSError:
            return False

        # Check if header starts with '<Terse'
        is_trpx = header.startswith(b'<Terse')

        return is_trpx

    @staticmethod
    def _read_metadata(image_file):
        hd = {}
        with FormatTRPX.open_file(image_file, "rb") as f:
            header_data = b''
            while True:
                chunk = f.read(1024)
                if not chunk:
                    # End of file reached without finding '/>'
                    raise RuntimeError("Failed to find the end of the header.")
                header_data += chunk
                if b'/>' in header_data:
                    end_index = header_data.find(b'/>') + 2
                    header_data = header_data[:end_index]
                    break

            try:
                header_str = header_data.decode('utf-8')
            except UnicodeDecodeError as e:
                raise RuntimeError(f"Failed to decode header data as UTF-8: {e}")

            try:
                root = ET.fromstring(header_str)
            except ET.ParseError as e:
                raise RuntimeError(f"Failed to parse XML header: {e}")

            hd['prolix_bits'] = int(root.attrib.get('prolix_bits', '12'))
            hd['signed'] = int(root.attrib.get('signed', '0'))
            hd['block'] = int(root.attrib.get('block', '12'))
            hd['memory_size'] = int(root.attrib.get('memory_size', '0'))
            hd['number_of_values'] = int(root.attrib.get('number_of_values', '0'))
            hd['number_of_frames'] = int(root.attrib.get('number_of_frames', '1'))

            hd['pixel_size'] = (0.055, 0.055)
            hd['trusted_range'] = (0, 65535)
            hd['distance'] = 478.0
            hd['start_tilt'] = 0.0
            hd['delta_tilt'] = 0.0
            hd['exposure'] = 1.0

            if 'dimensions' in root.attrib:
                dimensions_str = root.attrib['dimensions']
                dimensions = list(map(int, dimensions_str.strip().split()))
                hd['image_size'] = dimensions
            else:
                num_values = hd['number_of_values']
                image_dimension = int(np.sqrt(num_values))
                hd['image_size'] = (image_dimension, image_dimension)

            hd['distance'] = float(root.attrib.get('distance', '478.0'))
            pixel_size_str = root.attrib.get('pixel_size', '0.055 0.055')
            hd['pixel_size'] = tuple(map(float, pixel_size_str.strip().split()))
            trusted_range_str = root.attrib.get('trusted_range', '0 65535')
            hd['trusted_range'] = tuple(map(float, trusted_range_str.strip().split()))

        return hd


    def _start(self):
        """Open the image file and read useful metadata into an internal dictionary"""
        self._header_dictionary = self._read_metadata(self._image_file)

    def _goniometer(self):
        """Dummy goniometer, 'vertical' as the images are viewed."""
        goniometer = self._goniometer_factory.known_axis((0, -1, 0))
        return goniometer

    def _detector(self):
        beam_centre = [
            (p * i) / 2
            for p, i in zip(
                self._header_dictionary["pixel_size"],
                self._header_dictionary["image_size"],
            )
        ]
        d = self._detector_factory.simple(
            "PAD",
            self._header_dictionary["distance"],
            beam_centre,
            "+x",
            "-y",
            self._header_dictionary["pixel_size"],
            self._header_dictionary["image_size"],
            self._header_dictionary["trusted_range"],
        )
        return d


    def _beam(self):
        """Unpolarized beam, default energy 200 keV"""
        beam = self._beam_factory.make_polarized_beam(
            sample_to_source=(0.0, 0.0, 1.0),
            wavelength=0.02508,
            polarization=(0, 1, 0),
            polarization_fraction=0.5,
            probe=Probe.electron,
        ) 
        return beam

    def _scan(self):
        """Scan model for this image, filling out any unavailable items with dummy values"""
        alpha = self._header_dictionary.get("start_tilt", 0.0)
        dalpha = self._header_dictionary.get("delta_tilt", 1.0)
        exposure = self._header_dictionary.get("exposure", 0.0)
        oscillation = (alpha, dalpha)
        fname = os.path.split(self._image_file)[-1]
        s = fname.split("_")[-1].split(".")[0]
        try:
            index = int(re.match(".*?([0-9]+)$", s).group(1))
        except AttributeError:
            index = 1
        scan = ScanFactory.make_scan((index, index), exposure, oscillation, {index: 0}) 
        return scan

    def get_raw_data(self):
        terse = Terse.load(self._image_file)
        decompressed_data = terse.prolix()
        raw_data_flex = flex.double(decompressed_data)
        return raw_data_flex

if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python script.py image_file.trpx")
        sys.exit(1)
    image_file = sys.argv[1]
    try:
        if FormatTRPX.understand(image_file):
            format_instance = FormatTRPX(image_file)
            format_instance._start()
            raw_data = format_instance.get_raw_data()
            print("Raw data:", raw_data)
        else:
            print("File format not understood.")
    except Exception as e:
        print(f"An error occurred: {e}")

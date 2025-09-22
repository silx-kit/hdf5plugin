# Note 1: In gnuplot, execute this script using the following command:
#         call "plot_stacked_bar.gp"
# Note 2: Change the input file name in the last line for different input.

reset
set title "Compression(C) and Decompression(D) Time for a 128^3 Cube"
set ylabel "Millisecond"
set xlabel "Bit-Per-Pixel"
set key reverse left 
set style data histogram
set style histogram rowstacked
set style fill solid border -1
set boxwidth 0.75
set grid ytics
# set xtics rotate by -45
plot '128_cube.result' using 2 title "XForm", '' using 3:xtic(1) title "SPECK"

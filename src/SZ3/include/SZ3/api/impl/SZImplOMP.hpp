#ifndef SZ3_IMPL_SZDISPATCHER_OMP_HPP
#define SZ3_IMPL_SZDISPATCHER_OMP_HPP

#include "SZ3/api/impl/SZDispatcher.hpp"
#include <cmath>
#include <memory>


#ifdef _OPENMP

#include <omp.h>

#endif
namespace SZ3 {
    template<class T, uint N>
    size_t SZ_compress_OMP(Config &conf, const T *data, uchar *cmpData, size_t cmpCap) {
        unsigned char *buffer_pos = cmpData;

#ifdef _OPENMP

        assert(N == conf.N);

        std::vector<uchar *> compressed_t;
        std::vector<size_t> cmp_size_t, cmp_start_t;
        std::vector<T> min_t, max_t;
        std::vector<Config> conf_t;
//    Timer timer(true);
        int nThreads = 1;
        double eb;
#pragma omp parallel
#pragma omp single
        {
            nThreads = omp_get_num_threads();
        }
        if (conf.dims[0] < nThreads) {
            nThreads = conf.dims[0];
            omp_set_num_threads(nThreads);
        }
//        printf("OpenMP threads = %d\n", nThreads);
        compressed_t.resize(nThreads);
        cmp_size_t.resize(nThreads + 1);
        cmp_start_t.resize(nThreads + 1);
        conf_t.resize(nThreads);
        min_t.resize(nThreads);
        max_t.resize(nThreads);
#pragma omp parallel
        {

            int tid = omp_get_thread_num();

            auto dims_t = conf.dims;
            int lo = tid * conf.dims[0] / nThreads;
            int hi = (tid + 1) * conf.dims[0] / nThreads;
            dims_t[0] = hi - lo;
            auto it = dims_t.begin();
            size_t num_t_base = std::accumulate(++it, dims_t.end(), (size_t) 1, std::multiplies<size_t>());
            size_t num_t = dims_t[0] * num_t_base;

//        T *data_t = data + lo * num_t_base;
            std::vector<T> data_t(data + lo * num_t_base, data + lo * num_t_base + num_t);
            if (conf.errorBoundMode != EB_ABS) {
                auto minmax = std::minmax_element(data_t.begin(), data_t.end());
                min_t[tid] = *minmax.first;
                max_t[tid] = *minmax.second;
#pragma omp barrier
#pragma omp single
                {
                    T range = *std::max_element(max_t.begin(), max_t.end()) - *std::min_element(min_t.begin(), min_t.end());
                    calAbsErrorBound<T>(conf, data, range);
//                timer.stop("OMP init");
//                timer.start();
                }
            }

            conf_t[tid] = conf;
            conf_t[tid].setDims(dims_t.begin(), dims_t.end());
            cmp_size_t[tid] = data_t.size() * sizeof(T);
            compressed_t[tid] = (uchar *) malloc(cmp_size_t[tid]);
            SZ_compress_dispatcher<T, N>(conf_t[tid], data_t.data(), compressed_t[tid], cmp_size_t[tid]);

#pragma omp barrier
#pragma omp single
            {
//            timer.stop("OMP compression");
//            timer.start();
                cmp_start_t[0] = 0;
                for (int i = 1; i <= nThreads; i++) {
                    cmp_start_t[i] = cmp_start_t[i - 1] + cmp_size_t[i - 1];
                }
                size_t bufferSize = sizeof(int) + (nThreads + 1) * Config::size_est() + cmp_start_t[nThreads];
//                buffer = new uchar[bufferSize];
//                buffer_pos = buffer;
                write(nThreads, buffer_pos);
                for (int i = 0; i < nThreads; i++) {
                    conf_t[i].save(buffer_pos);
                }
                write(cmp_size_t.data(), nThreads, buffer_pos);
            }

            memcpy(buffer_pos + cmp_start_t[tid], compressed_t[tid], cmp_size_t[tid]);
            free(compressed_t[tid]);
        }
        
        return buffer_pos - cmpData + cmp_start_t[nThreads];
//    timer.stop("OMP memcpy");

#endif
//        return (char *) buffer;
        return 0;
    }


    template<class T, uint N>
    void SZ_decompress_OMP(const Config &conf, const uchar *cmpData, size_t cmpSize, T *decData) {
#ifdef _OPENMP

        auto cmpr_data_pos = cmpData;
        int nThreads = 1;
        read(nThreads, cmpr_data_pos);
        omp_set_num_threads(nThreads);
//    printf("OpenMP threads = %d\n", nThreads);

        std::vector<Config> conf_t(nThreads);
        for (int i = 0; i < nThreads; i++) {
            conf_t[i].load(cmpr_data_pos);
        }

        std::vector<size_t> cmp_start_t, cmp_size_t;
        cmp_size_t.resize(nThreads);
        read(cmp_size_t.data(), nThreads, cmpr_data_pos);
        auto cmpr_data_p = cmpr_data_pos;

        cmp_start_t.resize(nThreads + 1);
        cmp_start_t[0] = 0;
        for (int i = 1; i <= nThreads; i++) {
            cmp_start_t[i] = cmp_start_t[i - 1] + cmp_size_t[i - 1];
        }

#pragma omp parallel
        {
            int tid = omp_get_thread_num();
            auto dims_t = conf.dims;
            int lo = tid * conf.dims[0] / nThreads;
            int hi = (tid + 1) * conf.dims[0] / nThreads;
            dims_t[0] = hi - lo;
            auto it = dims_t.begin();
            size_t num_t_base = std::accumulate(++it, dims_t.end(), (size_t) 1, std::multiplies<size_t>());

            SZ_decompress_dispatcher<T, N>(conf_t[tid], cmpr_data_p + cmp_start_t[tid], cmp_size_t[tid], decData + lo * num_t_base);
        }
#endif
    }
}

#endif

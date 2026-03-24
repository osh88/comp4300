#include "Compress.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <string>
#include <lzma.h>

void LZMA::decompressStream(std::istream& input, std::ostream& output) {
    lzma_stream stream = LZMA_STREAM_INIT;
    lzma_ret ret;

    // Инициализация декодера LZMA2
    ret = lzma_stream_decoder(&stream, UINT64_MAX, 0);
    if (ret != LZMA_OK) {
        throw std::runtime_error("LZMA decoder initialization error: " + std::to_string(ret));
    }

    // Буферы для работы
    std::vector<uint8_t> inBuffer(8192);
    std::vector<uint8_t> outBuffer(8192);
    lzma_action action = LZMA_RUN;

    try {
        while (true) {
            // Чтение входных данных
            if (stream.avail_in == 0) {
                input.read(reinterpret_cast<char*>(inBuffer.data()), inBuffer.size());
                stream.next_in = inBuffer.data();
                stream.avail_in = input.gcount();

                if (stream.avail_in == 0) {
                    action = LZMA_FINISH;
                } else {
                    action = LZMA_RUN;
                }
            }

            // Декомпрессия
            stream.next_out = outBuffer.data();
            stream.avail_out = outBuffer.size();

            ret = lzma_code(&stream, action);

            switch (ret) {
                case LZMA_STREAM_END:
                    break;
                case LZMA_OK:
                    break;
                case LZMA_DATA_ERROR:
                    throw std::runtime_error("Bad format or corrupted data (LZMA_DATA_ERROR)");
                case LZMA_MEM_ERROR:
                    throw std::runtime_error("Out of memory (LZMA_MEMORY_ERROR)");
                case LZMA_BUF_ERROR:
                    throw std::runtime_error("Buffer error (LZMA_BUF_ERROR)");
                default:
                    throw std::runtime_error("Unexpected error LZMA: " + std::to_string(ret));
            }

            // Запись выходных данных
            size_t processed = outBuffer.size() - stream.avail_out;
            if (processed > 0) {
                output.write(reinterpret_cast<const char*>(outBuffer.data()), processed);
            }

            if (ret == LZMA_STREAM_END) break;
        }
    } catch (...) {
        lzma_end(&stream);
        throw;
    }

    lzma_end(&stream);
}

void LZMA::compressStream(std::istream& input, std::ostream& output, uint32_t compressionLevel) {
    lzma_stream stream = LZMA_STREAM_INIT;
    lzma_ret ret;

    // Настройки сжатия LZMA2
    lzma_options_lzma options;
    lzma_filter filters[2] = {
        { .id = LZMA_FILTER_LZMA2, .options = &options },
        { .id = LZMA_VLI_UNKNOWN, .options = nullptr }  // Завершающий фильтр
    };

    // Инициализация параметров LZMA2
    if (lzma_lzma_preset(&options, compressionLevel)) {
        throw std::runtime_error("Bad compression level: " + std::to_string(compressionLevel));
    }

    // Инициализация кодировщика
    ret = lzma_stream_encoder(&stream, filters, LZMA_CHECK_CRC64);
    if (ret != LZMA_OK) {
        throw std::runtime_error("LZMA encoder initialization error: " + std::to_string(ret));
    }

    // Буферы для работы
    std::vector<uint8_t> inBuffer(8192);
    std::vector<uint8_t> outBuffer(8192);
    lzma_action action = LZMA_RUN;

    try {
        while (true) {
            // Чтение входных данных
            if (stream.avail_in == 0) {
                input.read(reinterpret_cast<char*>(inBuffer.data()), inBuffer.size());
                stream.next_in = inBuffer.data();
                stream.avail_in = input.gcount();

                if (input.eof()) {
                    action = LZMA_FINISH;
                } else {
                    action = LZMA_RUN;
                }
            }

            // Сжатие
            stream.next_out = outBuffer.data();
            stream.avail_out = outBuffer.size();

            ret = lzma_code(&stream, action);

            // Запись сжатых данных
            size_t processed = outBuffer.size() - stream.avail_out;
            if (processed > 0) {
                output.write(reinterpret_cast<const char*>(outBuffer.data()), processed);
            }

            // Проверка завершения
            if (ret == LZMA_STREAM_END) {
                break;
            }
            if (ret != LZMA_OK) {
                throw std::runtime_error("LZMA compression error: " + std::to_string(ret));
            }
        }
    } catch (...) {
        lzma_end(&stream);
        throw;
    }

    lzma_end(&stream);
}

#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    std::byte signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t data_size;
    int32_t h_resolution;
    int32_t v_resolution;
    uint32_t colors_used;
    uint32_t important_colors;
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

// напишите эту функцию
bool SaveBMP(const Path& file, const Image& image){
    ofstream out(file, ios::binary);

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);
    const int data_size = stride * h;  // Размер данных
    const int file_size = 14 + 40 + data_size;  // Общий размер файла (заголовки + данные)
    std::vector<char> buffer(stride);
    BitmapFileHeader header;
    header.signature[0] = static_cast<std::byte>('B');
    header.signature[1] = static_cast<std::byte>('M');
    header.file_size = file_size;
    header.reserved = 0;
    header.data_offset = 14 + 40;  // Смещение данных (после обоих заголовков)

    out.write(reinterpret_cast<char*>(&header), sizeof(header));

    BitmapInfoHeader info_header;

    info_header.header_size = 40;
    info_header.width = w;
    info_header.height = h;
    info_header.planes = 1;
    info_header.bits_per_pixel = 24;
    info_header.compression = 0;
    info_header.data_size = data_size;
    info_header.h_resolution = 11811; // 300 DPI
    info_header.v_resolution = 11811; // 300 DPI
    info_header.colors_used = 0; // Значение не определено
    info_header.important_colors = 0x1000000; // Все цвета значимы

    out.write(reinterpret_cast<char*>(&info_header), sizeof(info_header));

    for (int y = h - 1; y >= 0; --y) {
        // Заполнение буфера данными строки
        const Color* row = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buffer[x * 3 + 0] = static_cast<char>( row[x].b ); // Blue
            buffer[x * 3 + 1] = static_cast<char>( row[x].g ); // Green
            buffer[x * 3 + 2] = static_cast<char>( row[x].r ); // Red
        }
        // Заполнение padding нулями
        std::fill(buffer.begin() + w * 3, buffer.end(), static_cast<char>(std::byte{0}));
        // Запись буфера в файл
        out.write(buffer.data(), stride);

    }

    return out.good();
}

// напишите эту функцию
Image LoadBMP(const Path& file){
    // открываем поток с флагом ios::binary
    // поскольку будем читать данные в двоичном формате
    ifstream ifs(file, ios::binary);
    // читаем заголовок: он содержит формат, размеры изображения
    // и максимальное значение цвета
    //ifs >> sign >> w >> h >> color_max;
    BitmapFileHeader header;
    BitmapInfoHeader info_header;
    ifs.read(reinterpret_cast<char*>(&header), sizeof(header));
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    // мы поддерживаем изображения только формата P6
    // с максимальным значением цвета 255
    // Проверка подписи файла
    if (header.signature[0] != static_cast<std::byte>('B') || header.signature[1] != static_cast<std::byte>('M')) {
        return {};
    }


    int w = info_header.width;
    int h = info_header.height;
    Image result(w, h, Color::Black());

    //int row_size = w * 3;  // размер данных пикселей
    int stride = GetBMPStride( w );
    std::vector<char> buff( stride );
    ifs.seekg(header.data_offset);  // переходим к началу данных пикселей

    for (int y = h - 1; y >= 0 ; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);

        for (int x = 0; x < w ; ++x) {
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
        }
    }

    return result;
}

}  // namespace img_lib

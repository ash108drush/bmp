#include <img_lib.h>
#include <jpeg_image.h>
#include <ppm_image.h>

#include <filesystem>
#include <string_view>
#include <iostream>

using namespace std;
namespace img_interface {
enum class Format { JPEG,
                    PPM,
                    UNKNOWN };

class ImageFormatInterface {
public:
    virtual bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const = 0;
    virtual img_lib::Image LoadImage(const img_lib::Path& file) const = 0;
};

class InterfaceJPG: public ImageFormatInterface {
public:
   bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override {
       return img_lib::SaveJPEG(file, image);
    }
   img_lib::Image LoadImage(const img_lib::Path& file) const override{
        return img_lib::LoadJPEG(file);
   }
};

class InterfacePPM: public ImageFormatInterface {
public:
    bool SaveImage(const img_lib::Path& file, const img_lib::Image& image) const override{
        return img_lib::SavePPM(file, image);
    }

    img_lib::Image LoadImage(const img_lib::Path& file) const override{
        return img_lib::LoadPPM(file);
    }
};

Format GetFormatByExtension(const img_lib::Path& input_file) {
    const string ext = input_file.extension().string();
    if (ext == ".jpg"sv || ext == ".jpeg"sv) {
        return Format::JPEG;
    }

    if (ext == ".ppm"sv) {
        return Format::PPM;
    }

    return Format::UNKNOWN;
}

ImageFormatInterface* GetFormatInterface(const img_lib::Path& path) {
    Format format = GetFormatByExtension(path);
    if (format == Format::JPEG) {
        static InterfaceJPG jpgInterface;  // создаём статический объект
        return &jpgInterface;
    }
    if (format == Format::PPM) {
        static InterfacePPM ppmInterface;  // создаём статический объект
        return &ppmInterface;
    }
    return nullptr;  // если формат неизвестен
}

} //end namespace image interface

int main(int argc, const char** argv) {
    if (argc != 3) {
        cerr << "Usage: "sv << argv[0] << " <in_file> <out_file>"sv << endl;
        return 1;
    }

    img_lib::Path in_path = argv[1];
    img_lib::Path out_path =  argv[2];

    img_interface::ImageFormatInterface* in_interface = img_interface::GetFormatInterface(in_path);
    img_interface::ImageFormatInterface* out_interface = img_interface::GetFormatInterface(out_path);

    if (!in_interface) {
        cerr << "Unknown format of the input file"sv << endl;
        return 2;
    }
    if (!out_interface) {
        cerr << "Unknown format of the output file"sv << endl;
        return 3;
    }

    img_lib::Image image = in_interface->LoadImage(in_path);
    if (!image) {
        cerr << "Loading failed"sv << endl;
        return 4;
    }

    // Сохраняем изображение в новом формате
    if (!out_interface->SaveImage(out_path, image)) {
        cerr << "Saving failed"sv << endl;
        return 5;
    }


    cout << "Successfully converted"sv << endl;
}

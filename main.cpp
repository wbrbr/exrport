#include <iostream>
#include <vector>
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImathBox.h>
#include <assert.h>
#include <memory>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;


/// Notes:
/// - I think there is something wrong with gamma correction / colorspace stuff
/// - TODO: exposure (multiply the values before clamping / gamma correction)


/// Clamps between 0 and 1
float clamp(float x) {
    return fmin(1.f, fmax(0.f, x));
}

float linearToSRGB(float u) {
    return u <= 0.0031308f
           ? 12.92f * u
           : 1.0055f * powf(u, 1.f/2.4f) - 0.055f;
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <exr file> <output> <exposure>" << std::endl;
        exit(1);
    }

    Imf::RgbaInputFile input(argv[1]);
    Imath::Box2i box = input.dataWindow();
    assert(box.min == Imath::V2i(0,0));

    int width = box.max.x+1;
    int height = box.max.y+1;

    std::vector<Imf::Rgba> buf;
    buf.resize(width * height);

    input.setFrameBuffer(buf.data(), 1, width);
    input.readPixels(0, height-1);

    std::vector<unsigned char> byte_buf;
    byte_buf.reserve(buf.size()*4);
    for (Imf::Rgba px : buf) {
        unsigned char r = (unsigned char)(linearToSRGB(clamp(px.r)) * 255.99f);
        unsigned char g = (unsigned char)(linearToSRGB(clamp(px.g)) * 255.99f);
        unsigned char b = (unsigned char)(linearToSRGB(clamp(px.b)) * 255.99f);
        unsigned char a = (unsigned char)(clamp(px.a) * 255.99f);
        byte_buf.push_back(r);
        byte_buf.push_back(g);
        byte_buf.push_back(b);
        byte_buf.push_back(a);
    }

    stbi_write_png(argv[2], width, height, 4, byte_buf.data(), 0);

    return 0;
}

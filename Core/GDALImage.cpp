#include "GDALImage.h"
#include <stdexcept>

GDALImage::GDALImage()
    : _data{nullptr}
{}

GDALImage::GDALImage(const boost::filesystem::path &filePath, GDALAccess access)
    : GDALImage()
{
    _data = (GDALDataset *) GDALOpen(filePath.c_str(), access);
}

GDALImage::GDALImage(GDALDataset *data)
    : _data{data}
{}

GDALImage::GDALImage(GDALImage &&other) noexcept
{
    _data = other._data;
    other._data = nullptr;
}

GDALImage &GDALImage::operator=(GDALImage &&other) noexcept
{
    swap(*this, other);
    return *this;
}

GDALImage::~GDALImage() noexcept
{
    if (_data) {
        GDALClose(_data);
    }
}

void swap(GDALImage &lhs, GDALImage &rhs) noexcept
{
    std::swap(lhs._data, rhs._data);
}

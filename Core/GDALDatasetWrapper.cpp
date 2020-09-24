#include "GDALDatasetWrapper.h"
#include <stdexcept>

GDALDatasetWrapper::GDALDatasetWrapper()
    : _data{nullptr}
{}

GDALDatasetWrapper::GDALDatasetWrapper(GDALDataset *data)
    : _data{data}
{}

GDALDatasetWrapper::GDALDatasetWrapper(GDALDatasetWrapper &&other) noexcept
{
    _data = other._data;
    other._data = nullptr;
}

GDALDatasetWrapper &GDALDatasetWrapper::operator=(GDALDatasetWrapper &&other) noexcept
{
    swap(*this, other);
    return *this;
}

GDALDatasetWrapper::~GDALDatasetWrapper() noexcept
{
    if (_data) {
        GDALClose(_data);
    }
}

void swap(GDALDatasetWrapper &lhs, GDALDatasetWrapper &rhs) noexcept
{
    std::swap(lhs._data, rhs._data);
}

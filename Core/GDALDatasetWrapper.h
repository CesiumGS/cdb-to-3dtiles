#pragma once

#include "boost/filesystem.hpp"
#include "gdal_priv.h"

class GDALDatasetWrapper
{
public:
    GDALDatasetWrapper();

    GDALDatasetWrapper(GDALDataset *data);

    GDALDatasetWrapper(const GDALDatasetWrapper &) = delete;

    GDALDatasetWrapper(GDALDatasetWrapper &&) noexcept;

    GDALDatasetWrapper &operator=(const GDALDatasetWrapper &) = delete;

    GDALDatasetWrapper &operator=(GDALDatasetWrapper &&) noexcept;

    ~GDALDatasetWrapper() noexcept;

    inline const GDALDataset *data() const noexcept { return _data; }

    inline GDALDataset *data() noexcept { return _data; }

private:
    GDALDataset *_data;

    friend void swap(GDALDatasetWrapper &lhs, GDALDatasetWrapper &rhs) noexcept;
};

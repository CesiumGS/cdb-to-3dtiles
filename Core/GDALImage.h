#pragma once

#include "boost/filesystem.hpp"
#include "gdal_priv.h"

class GDALImage
{
public:
    GDALImage();

    GDALImage(const boost::filesystem::path &jp2Path, GDALAccess access);

    GDALImage(GDALDataset *data);

    GDALImage(const GDALImage &) = delete;

    GDALImage(GDALImage &&) noexcept;

    GDALImage &operator=(const GDALImage &) = delete;

    GDALImage &operator=(GDALImage &&) noexcept;

    ~GDALImage() noexcept;

    inline const GDALDataset *data() const noexcept { return _data; }

    inline GDALDataset *data() noexcept { return _data; }

private:
    GDALDataset *_data;

    friend void swap(GDALImage &lhs, GDALImage &rhs) noexcept;
};

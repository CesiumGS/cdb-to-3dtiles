# CDB to 3D Tiles

Convert Open Geospatial Consortium (OGC) CDB datasets to 3D Tiles OGC Community Standard for efficient streaming and rendering across multiple platforms and devices.

<img src="Doc/cdb23dtiles.jpg" width="100%" />

Converted tilesets faithfully match the level of detail and precision of the source CDB, with support for most common CDB layers and their associated feature attributes. See [Features](#clipboard-Features) for a full list of supported datasets and [Performance](#checkered_flag-Performance) for performance comparisons.

3D Tiles is designed for efficient runtime visualization and analytics. The pipeline preserves feature attributes from CDB, enabling runtime querying, styling, and analytics to gain deeper insights into the data.

<img src="Doc/downtown-San-Diego.png" width="100%" />

_View of downtown San Diego with terrain, imagery, clamped building models, instanced trees, and Coronado Bridge in the distance, loaded as 3D Tiles in [CesiumJS](https://cesium.com/cesiumjs/). See the live demo [here](https://sandcastle.cesium.com/standalone.html#c=xZZtU+M2EID/iiafkpkgW5bjFy7HFBLgnCGhR8LBUfeDsRUiYsupJIcLHf57ZTuvJLzcdUq/WLZWu/vsrrSWpoFTHjAJWkTQLOn0QRCGRAggUzBLMw5oykAgBJHCZ+Ua6KUMRmQYZLE8LBYP0jFh4DPwK2TWGd2ehvScdrzLRw/1qCc8dtEIW57ljSfX31odF6pFf0WnY7XIe+wlJ6Pu403S7SPavR/PzgZfzW67K8+vjh/PW2h8k5zcd6++/7hJLme9K4+etTqTG2Ws1/4qvCQeReq9O/j+0Bscou7gsNHDOozs66R3e+Ilvx/jowa961x3Lu0bpx0c6fbkSNxf3I7RFykH7WzsVz75zGfTgIMpJQ+EqygYeZhnA34r5qp+JSy+WymTAWWE+5XaUu84JtNA5mnaUC0H3B7QmKjsVf/2GQAZj/fBKosXRKgUhwQOeZoc5kn2oipyrIar1+o+e8qdlFhQhIQROOE0oZJOiYBBFFWXrlc0p/1uGpFYKJjXNHOYt2Dfi2tjx8xxAXiq+ewt5gXgGvKgnDniNLojHwiODetnwDcwt/EHnBDxsfSNn6cvKFfwX2YRT+94MBnNekQ+pHz8sRHg90ewjboK4yINov+FX//3/MOMhUX7EKP0AbfLMlXzjxoocJanHOaTKr58+JRLFmdpW7CxV18UF5vhmfRpi2kbfJ1uW/qmwbVqrVtam95loh+wKAyEjEmezkGaxrcB7xKWFTX9oyzcvH4ASPJD7qsfUj83tMxgfZmyOlDmlonwK/WFYsoEiUmolJfM1drKLtgsk+RZ0QrWhTvyNQxisbVuPQ3PFjzNcRbj7rByA2Bu4Zcj2In2KyFspuJ9Eaz5+c8D2FWq10ow58+HP/3FH7Y832GQEB7AYTwbpGXLiIiQlJXXgP2NBhNwqd4Chqt7htkwXAtByzIwMk1Tt+pgz7RtFyEEsWWaLrYbZh1g01AjgrZhGhZysWOVXSbllKgLyNzJMh8R5SR81bMOTd1FDWTqCLsmcg1dedlTs8hsOJaFsIltpJ6NYtZBhmMaucAybdd0asvCZJOXg9Oh4ejYMmwLWw11J7BQ6cPVTcdUhrBtG4ZjOXWgQ2zo2HFtrDsOxtiYeyi3SpTxRYR4fgXy2c4z99ZGfbHAlXqlKeQsJge5w99oMkm5zHt+FUJNkmQSByos7TYLx0TCUBT/y6a2UGlGdApo9HnHrRCEsborK8kwi+M+fSR+5aCpqfUbarGCouzufEp4HKiNX2A0R+jgrBRACJua+sydPteVZd9bs6uiGUk5EfuaJlYtskSDYZpod0EcEz7T/gE)_.

#

### :rocket: Getting Started

See [Getting Started](#getting-started) for installation, build, and usage instructions.

### :green_book: License

[Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0.html). CDB to 3D Tiles is free for both commercial and non-commercial use. See [LICENSE.md](./LICENSE.md) for more details.

The [San Diego CDB](https://gsa-temp-public.s3.us-east-1.amazonaws.com/CDB_san_diego_v4.1.zip) end-user license agreement can be found [here](./Doc/SanDiego_CDB_EULA.pdf).

### :heavy_plus_sign: Contributions

Pull requests are appreciated.  Please use the same [Contributor License Agreement (CLA)](https://github.com/CesiumGS/cesium/blob/master/CONTRIBUTING.md) used for [CesiumJS](https://cesium.com/cesiumjs/).

### :clipboard: Features

The following CDB features are supported:

#### Feature checklist

Dataset|CDB Name|Supported
--|--|--
Primary Elevation|`001_Elevation`|:heavy_check_mark:
Primary Imagery|`004_Imagery`|:heavy_check_mark:
Road Network|`201_RoadNetwork`|:heavy_check_mark:
Rail Road Network|`202_RailRoadNetwork`|:heavy_check_mark:
Power Line Network|`203_PowerLineNetwork`|:heavy_check_mark:
Hydrography Network|`204_HydrographyNetwork`|:heavy_check_mark:
Geotypical models|`101_GTFeature`, `500_GTModelGeometry`, `501_GTModelTexture`|:heavy_check_mark:
Geospecific models|`100_GSFeature`, `300_GSModelGeometry`, `301_GSModelTexture`|:heavy_check_mark:
Moving models|`600_MModelGeometry`, `601_GSModelTexture`|:x:
Min Max Elevation|`002_MinMaxElevation` `003_MaxCulture`|:x:
Geopolitical Boundaries|`102_GeoPolitical`|:x:
Vector Materials|`200_VectorMaterial`|:x:
Raster Materials|`005_RMTexture`, `006_RMDescriptor`|:x:
Navigation|`400_NavData`, `401_Navigation`|:x:
Bathymetry||:x:
Seasonal Imagery||:x:

#### Additional capabilities

Capability|Supported
--|--
Preserve instance and class attributes for models and vector layers|:heavy_check_mark:
Preserve geometry and texture quality with command line options for controlling mesh decimation|:heavy_check_mark:
Clamp models to the primary elevation dataset|:heavy_check_mark:
Clamp vector layers to the primary elevation dataset|:x:

#### Roadmap

* Windows support
* Performance improvements
* Automatic upload to Cesium ion
* Support more CDB datasets
* Clamp vector layers
* Output 3D Tiles Next (for interoperability with One World Terrain Well-Formed Format)

If you would like to provide feedback or accelerate the product roadmap for features you would like to see included, please contact [Shehzan Mohammed](mailto:shehzan@cesium.com).

#### CDB versions

All versions of CDB are supported. CDB 3.0 and CDB OGC 1.2 (draft) have been tested most during development.

### :checkered_flag: Performance

Performance numbers for [San Diego CDB](https://gsa-temp-public.s3.us-east-1.amazonaws.com/CDB_san_diego_v4.1.zip) measured on a Dell XPS 15 7590

Dataset|Time Elapsed|CDB Size|3D Tiles Size
--|--|--|--
Elevation and Imagery | 38 minutes | 20.3 GB | 17.1 GB
Road Network | 2 seconds | 166.8 MB | 121.3 MB
Hydrography Network | 0.2 seconds | 605 kB | 533.4 kB
GTModel | 0.8 seconds | 221.2 MB | 3.2 MB
GSModel | 9 minutes | 7.6 GB | 1.8 GB
**Total** | **47 minutes** | **28.3 GB** | **19.0 GB**

## Getting Started

### Prerequisites

- Linux (Windows support coming soon)
- C++ compiler that supports C++17 (tested on GCC 9.3.0)
- CMake version 3.15 or higher
- GDAL version 3.0.4 or higher
- OpenGL (needed by OpenSceneGraph)

To install GDAL 3.0.4 on Debian-based systems:

```bash
sudo add-apt-repository ppa:ubuntugis/ppa && sudo apt-get update
sudo apt-get update
sudo apt-get install libgdal-dev
```

To install OpenGL on Debian-based headless systems:

```bash
sudo apt-get install libgl1-mesa-dev
```

### Installing

Clone the repo with:
```
git clone --recurse-submodules git@github.com:CesiumGS/cdb-to-3dtiles.git
```

If `--recurse-submodules` is omitted, run the following command to update submodules:
```
git submodule update --init --recursive
```

### Building

The converter can be built on the command-line with CMake (given that you satisfy all [prerequisites](#prerequisites)):
```
cmake -B Build -S .
cmake --build Build --config Release -j 4
```

The executable can be found in the directory `Build/CLI/CDBConverter`

### Usage

```
Usage:

  CDBConverter [OPTION...]

  -i, --input arg               CDB directory
  -o, --output arg              3D Tiles output directory
      --combine arg             Combine converted datasets into one tileset.
                                Each dataset format is
                                {DatasetName}_{ComponentSelector1}_{ComponentSelector2}. Repeat this
                                option to group different dataset into
                                different tilesets.E.g:
                                --combine=Elevation_1_1,GSModels_1_1 --combine=GTModels_2_1,GTModels_1_1
                                will combine Elevation_1_1 and GSModels_1_1 into
                                one tileset. GTModels_2_1 and GTModels_1_1
                                will be combined into a different tileset
                                (default: Elevation_1_1,GSModels_1_1,GTModels_2_1,GTModels_1_1)
      --elevation-normal        Generate elevation normal
      --elevation-lod           Generate elevation and imagery based on
                                elevation LOD only
      --elevation-decimate-error arg
                                Set target error when decimating elevation
                                mesh. Target error is normalized to 0..1 (0.01
                                means the simplifier maintains the error to be
                                below 1% of the mesh extents) (default: 0.01)
      --elevation-threshold-indices arg
                                Set target percent of indices when decimating
                                elevation mesh (default: 0.3)
      --3d-tiles-next           Generate 3D Tiles Next
  -h, --help                    Print usage
```

### Example

The following command converts [San Diego CDB](https://gsa-temp-public.s3.us-east-1.amazonaws.com/CDB_san_diego_v4.1.zip) to 3D Tiles:
```
./Build/CLI/CDBConverter -i CDB_san_diego_v4.1 -o San_Diego
```

### Unit Tests

To run unit tests, run the following command:

```
./Build/Tests/Tests
```

### Docker

You can use Docker to simplify setting up the environment for building and testing. You must install [Docker Engine CE For Ubuntu](https://docs.docker.com/install/linux/docker-ce/ubuntu/) to do so.

The converter can be built with:

```bash
./Docker/build-container.sh
./Docker/build-cdb-to-3dtiles.sh
```

The executable can be found in the directory `Build/CLI/CDBConverter`

Run units tests with the following command:
```
./Docker/run-tests.sh
```

### 3D Tiles Structure

The converter generates multiple 3D Tilesets from the input CDB:

- Primary Elevation and Primary Imagery are combined into a single tileset
- Geotypical models are combined into a single tileset
- Geospecific models are combined into a single tileset
- Vector datasets are written as separate tilesets:
    - Road Network
    - Rail Road Network
    - Power Line Network
    - Hydrography Network

The generated tilesets will be placed in a GeoCell directory similar to how the CDB is organized. For example, the generated tilesets for San Diego will be placed in the `San_Diego/Tiles/N32/W118` directory.

A CDB tile can store different geometries depending on its component selectors. For example, a Hydrography Network tile can have lineal and polygon features within the same tile, which are differentiated by their second respective component selector in the file name. For this reason, a `tileset.json` will be placed in the directory name that make up of the two component selectors, `{Component Selector 1}_{Component Selector 2}`. For example:
- The lineal tileset of the Hydrography Network dataset will be placed in `HydrographyNetwork/2_3` directory 
- The polygon tileset of the Hydrography Network dataset will be placed in `HydrographyNetwork/2_5` directory

Below is the output directory of the converted San Diego 3D Tiles:

<p>
<img src="Doc/Converted-San-Diego-directory.png" width="60%" />&nbsp;
<br/>
</p>

### :sparkles: 3D Tiles Next Support

This converter now supports generating 3D Tiles Next. Use the `--3d-tiles-next` flag to enable this feature. The following extensions are currently supported:

- [EXT_feature_metadata](https://github.com/CesiumGS/glTF/blob/feature-metadata/extensions/2.0/Vendor/EXT_feature_metadata/0.0.0/README.md)
- [3DTILES_content_gltf](https://github.com/CesiumGS/3d-tiles/tree/3d-tiles-next/extensions/3DTILES_content_gltf/0.0.0)

*Note: currently only numerical types are supported in the metadata. Support for `STRING` is coming soon*

3D Tiles Next support is currently enabled for the following Datasets:
- Elevation
- Imagery
- GSModels
- Vector Datasets

#### Demo

[Live Demo](http://cesium-dev.s3-website-us-east-1.amazonaws.com/cesium/feature-metadata-feb/Apps/Sandcastle/index.html#c=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) of Yemen `GSModels` dataset.

<img src="https://user-images.githubusercontent.com/5172619/108842780-c9c43780-75a7-11eb-929e-e541a45d8534.png" width="100%">

## Releases

We release as often as needed. CDB To 3D Tiles strictly follows [semver](https://semver.org/).

- Update the project version number in [CMakeLists.txt](./CMakeLists.txt)
- Proofread [CHANGES.md](./CHANGES.md) and make any required updates
- Make sure all unit tests pass
- Commit and push the above changes to `master`
- Create a git tag for the version and push it:
  - `git tag -a 0.1.0 -m "0.1.0 release"`
  - `git push origin 0.1.0`

## Featured Demo

[Live Demo](https://sandcastle.cesium.com/standalone.html#c=xZZtU+M2EID/iiafkpkgW5bjFy7HFBLgnCGhR8LBUfeDsRUiYsupJIcLHf57ZTuvJLzcdUq/WLZWu/vsrrSWpoFTHjAJWkTQLOn0QRCGRAggUzBLMw5oykAgBJHCZ+Ua6KUMRmQYZLE8LBYP0jFh4DPwK2TWGd2ehvScdrzLRw/1qCc8dtEIW57ljSfX31odF6pFf0WnY7XIe+wlJ6Pu403S7SPavR/PzgZfzW67K8+vjh/PW2h8k5zcd6++/7hJLme9K4+etTqTG2Ws1/4qvCQeReq9O/j+0Bscou7gsNHDOozs66R3e+Ilvx/jowa961x3Lu0bpx0c6fbkSNxf3I7RFykH7WzsVz75zGfTgIMpJQ+EqygYeZhnA34r5qp+JSy+WymTAWWE+5XaUu84JtNA5mnaUC0H3B7QmKjsVf/2GQAZj/fBKosXRKgUhwQOeZoc5kn2oipyrIar1+o+e8qdlFhQhIQROOE0oZJOiYBBFFWXrlc0p/1uGpFYKJjXNHOYt2Dfi2tjx8xxAXiq+ewt5gXgGvKgnDniNLojHwiODetnwDcwt/EHnBDxsfSNn6cvKFfwX2YRT+94MBnNekQ+pHz8sRHg90ewjboK4yINov+FX//3/MOMhUX7EKP0AbfLMlXzjxoocJanHOaTKr58+JRLFmdpW7CxV18UF5vhmfRpi2kbfJ1uW/qmwbVqrVtam95loh+wKAyEjEmezkGaxrcB7xKWFTX9oyzcvH4ASPJD7qsfUj83tMxgfZmyOlDmlonwK/WFYsoEiUmolJfM1drKLtgsk+RZ0QrWhTvyNQxisbVuPQ3PFjzNcRbj7rByA2Bu4Zcj2In2KyFspuJ9Eaz5+c8D2FWq10ow58+HP/3FH7Y832GQEB7AYTwbpGXLiIiQlJXXgP2NBhNwqd4Chqt7htkwXAtByzIwMk1Tt+pgz7RtFyEEsWWaLrYbZh1g01AjgrZhGhZysWOVXSbllKgLyNzJMh8R5SR81bMOTd1FDWTqCLsmcg1dedlTs8hsOJaFsIltpJ6NYtZBhmMaucAybdd0asvCZJOXg9Oh4ejYMmwLWw11J7BQ6cPVTcdUhrBtG4ZjOXWgQ2zo2HFtrDsOxtiYeyi3SpTxRYR4fgXy2c4z99ZGfbHAlXqlKeQsJge5w99oMkm5zHt+FUJNkmQSByos7TYLx0TCUBT/y6a2UGlGdApo9HnHrRCEsborK8kwi+M+fSR+5aCpqfUbarGCouzufEp4HKiNX2A0R+jgrBRACJua+sydPteVZd9bs6uiGUk5EfuaJlYtskSDYZpod0EcEz7T/gE) of San Diego terrain, imagery, and models. Click individual models or vector features to see their metadata.

<img src="Doc/Bird-eye-san-diego.png" width="100%" />
<img src="Doc/San-Diego-RoadNetwork.png" width="100%" />

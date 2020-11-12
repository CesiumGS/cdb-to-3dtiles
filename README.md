# CDB to 3D Tiles

Convert Open Geospatial Consortium (OGC) CDB datasets to 3D Tiles OGC Community Standard for efficient streaming and rendering across multiple platforms and devices.

<img src="Doc/cdb23dtiles.jpg" width="100%" />

Converted tilesets faithfully match the level of detail and precision of the source CDB, with support for most common CDB layers and their associated feature attributes. See [Features](#clipboard-Features) for a full list of supported datasets and [Performance](#checkered_flag-Performance) for performance comparisons.

3D Tiles is designed for efficient runtime visualization and analytics. The pipeline preserves feature attributes from CDB, enabling runtime querying, styling, and analytics to gain deeper insights into the data.

Roadmap

* Windows support
* Performance improvements
* Automatic upload to Cesium ion
* Support more CDB datasets
* Clamp vector layers
* Output 3D Tiles Next (for interoperability with One World Terrain Well-Formed Format)

If you would like to provide feedback or accelerate the product roadmap for features you would like to see included, please contact [Shehzan Mohammed](mailto:shehzan@cesium.com).


<img src="Doc/downtown-San-Diego.png" width="100%" />

_View of downtown San Diego with terrain, imagery, clamped building models, instanced trees, and Coronado Bridge in the distance, loaded as 3D Tiles in [CesiumJS](https://cesium.com/cesiumjs/). See the live demo [here](https://sandcastle.cesium.com/standalone.html#c=xZZtU+M2EID/iiafkpkgW5bjFy7HFBLgnCGhR8LBUfeDsBUiYsupLIcLHf57ZTuvJLzcdUq/WLZWu/vsrrSWpoFTQbgELZqyLO70AQkCmqZAJmCWZAKwhAOSplSmPi/XQC/hMKRDkkXysFg8SMaUg8/Ar9BZZ3R7GrBz1vEuHz3UY17q8YtG0PIsbzy5/tbquFAt+is8HatF3mMvPhl1H2/ibh+x7v14djb4anbbXXl+dfx43kLjm/jkvnv1/cdNfDnrXXnsrNWZ3ChjvfbX1IujUajeu4PvD73BIeoODhs9rMPQvo57tyde/PsxPmqwu85159K+cdrkSLcnR+n9xe0YfZFy0M7GfuWTz30+JQJMGX2gQkXB6cM8G/BbMVf1K0Hx3Uq4JIxT4VdqS73jiE6JzNO0oVoOuD1gEVXZq/7tcwAyEe2DVRYvaKpSHFA4FEl8mCfZC6vIwVi3a3WfP+VOSiyYBpRTOBEsZpJNaQpJGFaXrlc0p/1uEtIoVTCvaeYwb8G+G9ewClwAnmo+f4t5AbiGPChnjgQL7+jHgps/Ab6BuY0/EJR+cNobP09fUK7gv8xCkdwJMhnNelQ+JGL8sRHg90ewjboK4yIh4f/Cr/97/mHGg6J9pKPkAbfLMlXzjxoocJanHOaTKr58+JRLFmdpW7CxV18UF5vhmfRpi2kbfJ1uW/qmwbVqrVtam95lok94GJBURjRP5yBJolsiupRnRU3/KAs3rx8Akv6Q++qH1M8NLTNYX6asDpS5ZSL8Sn2hmPCURjRQykvmam1lF2yWSYqsaAXrwh35GpIo3Vq3noZnC57mOItxd1i5ATC38MsR7ET7lRA2U/G+CNb8/OcB7CrVayWY8+fDn/7iD1ue74DEVBA4jGaDpGwZIU0l4+U1YH+jwRAh1RvhuLpnmA3DtRC0LAMj0zR1qw72TNt2EUIQW6bpYrth1gE2DTUiaBumYSEXO1bZZRLBqLqAzJ0s8xEyQYNXPevQ1F3UQKaOsGsi19CVlz01i8yGY1kIm9hG6tkoZh1kOKaRCyzTdk2ntixMNnk5OB0ajo4tw7aw1XBsbKHSh6ubjqkMYds2DMdy6kCH2NCx49pYd9RdBxtzD+VWCTOxiBDPr0A+33nm3tqoLxa4Uq80UzmL6EHu8DcWTxIh855fhVCTNJ5ERIWl3WbBmEoYpMX/sqktVJohmwIWft5xKwRBpO7KSjLMoqjPHqlfOWhqav2GWqSgGL87n1IREbXxC4zmCB2clQIIYVNTn7nT57qy7HtrdlU0Iykn6b6mpasWWaLBIIm1OxJFVMy0fwA)_.

### :rocket: Getting Started

See [Getting Started](#getting-started) for installation, build, and usage instructions.

### :green_book: License

[Apache 2.0](http://www.apache.org/licenses/LICENSE-2.0.html). CDB to 3D Tiles is free for both commercial and non-commercial use. See [LICENSE.md](./LICENSE.md) for more details.

The [San Diego CDB](https://gsa-temp-public.s3.us-east-1.amazonaws.com/CDB_san_diego_v4.1.zip) end-user license agreement can be found [here](./Doc/SanDiego_CDB_EULA.pdf).

### :sparkles: Contributions

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
You can use Docker to simplify setting up the environment for building and testing. You must install [Docker Engine CE For Ubuntu](https://docs.docker.com/install/linux/docker-ce/ubuntu/) to do so. Else you must install the libraries below.

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
If using Docker, the converter can be built with:
```
./Docker/build-container.sh
./Docker/build-cdb-to-3dtiles.sh
```

Without Docker, the converter can be built on the command-line with CMake (given that you satisfy all [prerequisites](#prerequisites)):
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

If using Docker, run the following command:
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


## Featured Demo

[Live Demo](https://sandcastle.cesium.com/standalone.html#c=xZZtU+M2EID/iiafkpkgW5bjFy7HFBLgnCGhR8LBUfeDsBUiYsupLIcLHf57ZTuvJLzcdUq/WLZWu/vsrrSWpoFTQbgELZqyLO70AQkCmqZAJmCWZAKwhAOSplSmPi/XQC/hMKRDkkXysFg8SMaUg8/Ar9BZZ3R7GrBz1vEuHz3UY17q8YtG0PIsbzy5/tbquFAt+is8HatF3mMvPhl1H2/ibh+x7v14djb4anbbXXl+dfx43kLjm/jkvnv1/cdNfDnrXXnsrNWZ3ChjvfbX1IujUajeu4PvD73BIeoODhs9rMPQvo57tyde/PsxPmqwu85159K+cdrkSLcnR+n9xe0YfZFy0M7GfuWTz30+JQJMGX2gQkXB6cM8G/BbMVf1K0Hx3Uq4JIxT4VdqS73jiE6JzNO0oVoOuD1gEVXZq/7tcwAyEe2DVRYvaKpSHFA4FEl8mCfZC6vIwVi3a3WfP+VOSiyYBpRTOBEsZpJNaQpJGFaXrlc0p/1uEtIoVTCvaeYwb8G+G9ewClwAnmo+f4t5AbiGPChnjgQL7+jHgps/Ab6BuY0/EJR+cNobP09fUK7gv8xCkdwJMhnNelQ+JGL8sRHg90ewjboK4yIh4f/Cr/97/mHGg6J9pKPkAbfLMlXzjxoocJanHOaTKr58+JRLFmdpW7CxV18UF5vhmfRpi2kbfJ1uW/qmwbVqrVtam95lok94GJBURjRP5yBJolsiupRnRU3/KAs3rx8Akv6Q++qH1M8NLTNYX6asDpS5ZSL8Sn2hmPCURjRQykvmam1lF2yWSYqsaAXrwh35GpIo3Vq3noZnC57mOItxd1i5ATC38MsR7ET7lRA2U/G+CNb8/OcB7CrVayWY8+fDn/7iD1ue74DEVBA4jGaDpGwZIU0l4+U1YH+jwRAh1RvhuLpnmA3DtRC0LAMj0zR1qw72TNt2EUIQW6bpYrth1gE2DTUiaBumYSEXO1bZZRLBqLqAzJ0s8xEyQYNXPevQ1F3UQKaOsGsi19CVlz01i8yGY1kIm9hG6tkoZh1kOKaRCyzTdk2ntixMNnk5OB0ajo4tw7aw1XBsbKHSh6ubjqkMYds2DMdy6kCH2NCx49pYd9RdBxtzD+VWCTOxiBDPr0A+33nm3tqoLxa4Uq80UzmL6EHu8DcWTxIh855fhVCTNJ5ERIWl3WbBmEoYpMX/sqktVJohmwIWft5xKwRBpO7KSjLMoqjPHqlfOWhqav2GWqSgGL87n1IREbXxC4zmCB2clQIIYVNTn7nT57qy7HtrdlU0Iykn6b6mpasWWaLBIIm1OxJFVMy0fwA) of San Diego terrain, imagery, and models. Click individual models or vector features to see their metadata.

<img src="Doc/Bird-eye-san-diego.png" width="100%" />
<img src="Doc/San-Diego-RoadNetwork.png" width="100%" />

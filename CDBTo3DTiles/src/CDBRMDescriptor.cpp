#include "CDBRMDescriptor.h"
#include "rapidxml_utils.hpp"

namespace CDBTo3DTiles {

CDBRMDescriptor::CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile)
    :_tile{tile}
{
  rapidxml::file<> xmlFile(xmlPath.c_str());
  _xml.parse<0>(xmlFile.data());
}

} // namespace CDBTo3DTiles

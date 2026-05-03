#include "geom/GeomPoint.h"

namespace aurora::geom {

const char* geomPointTranslationUnitName() {
  return "GeomPoint";
}

static_assert(sizeof(GeomPoint) == sizeof(DbUnit) * 2);

}  // namespace aurora::geom

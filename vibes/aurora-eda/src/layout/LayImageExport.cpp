#include "layout/LayImageExport.h"

#include "db/DbCellLib.h"
#include "db/DbLayer.h"
#include "db/DbShape.h"
#include "db/DbView.h"
#include "geom/GeomBox.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace aurora::layout {

namespace {

geom::GeomBox boundsOf(const db::DbView& view) {
  geom::GeomBox b{0, 0, 0, 0};
  bool first = true;
  for (auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& sb = static_cast<const db::DbRect*>(s)->box();
    if (first) { b = sb; first = false; }
    else {
      b = geom::GeomBox{std::min(b.left(), sb.left()), std::min(b.bottom(), sb.bottom()),
                        std::max(b.right(), sb.right()), std::max(b.top(), sb.top())};
    }
  }
  return b;
}

}  // namespace

bool LayImageExport::writeSvg(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path) const {
  std::ofstream o(path);
  if (!o) return false;
  auto bb = boundsOf(view);
  o << "<?xml version=\"1.0\"?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" "
       "viewBox=\"" << bb.left() << " " << bb.bottom() << " "
    << bb.width() << " " << bb.height() << "\">\n";
  for (auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& b = static_cast<const db::DbRect*>(s)->box();
    const auto* l = lib.findLayer(s->layerId());
    const std::string color = l ? l->color() : "#808080";
    o << "<rect x=\"" << b.left() << "\" y=\"" << b.bottom()
      << "\" width=\"" << b.width() << "\" height=\"" << b.height()
      << "\" fill=\"" << color << "\" fill-opacity=\"0.5\" stroke=\"" << color << "\"/>\n";
  }
  o << "</svg>\n";
  return true;
}

bool LayImageExport::writePdf(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path) const {
  std::ofstream o(path, std::ios::binary);
  if (!o) return false;

  auto bb = boundsOf(view);
  const double pageW = 612, pageH = 792;
  const double scale = bb.width() > 0
      ? std::min(pageW / bb.width(), pageH / bb.height()) * 0.9
      : 1.0;

  std::ostringstream content;
  content << "q\n" << scale << " 0 0 " << scale << " "
          << (-bb.left() * scale + 20) << " " << (-bb.bottom() * scale + 20) << " cm\n";
  for (auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& b = static_cast<const db::DbRect*>(s)->box();
    content << "0.5 0.5 0.5 rg\n";
    content << b.left() << " " << b.bottom() << " "
            << b.width() << " " << b.height() << " re\nf\n";
  }
  content << "Q\n";
  std::string cstr = content.str();

  o << "%PDF-1.4\n";
  std::vector<std::streampos> offsets;
  auto put = [&](const std::string& s) {
    offsets.push_back(o.tellp());
    o << s;
  };
  put("1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n");
  put("2 0 obj\n<< /Type /Pages /Kids [3 0 R] /Count 1 >>\nendobj\n");
  std::ostringstream page;
  page << "3 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [0 0 "
       << pageW << " " << pageH << "] /Contents 4 0 R >>\nendobj\n";
  put(page.str());
  std::ostringstream stream;
  stream << "4 0 obj\n<< /Length " << cstr.size() << " >>\nstream\n"
         << cstr << "endstream\nendobj\n";
  put(stream.str());
  std::streampos xref = o.tellp();
  o << "xref\n0 5\n0000000000 65535 f \n";
  for (auto off : offsets) {
    o << std::string(10 - std::to_string(static_cast<long long>(off)).size(), '0')
      << static_cast<long long>(off) << " 00000 n \n";
  }
  o << "trailer\n<< /Size 5 /Root 1 0 R >>\nstartxref\n" << static_cast<long long>(xref) << "\n%%EOF\n";
  (void)lib;
  return true;
}

bool LayImageExport::writePpm(const db::DbCellLib& lib, const db::DbView& view,
                              const std::filesystem::path& path, int widthPx) const {
  auto bb = boundsOf(view);
  const long long w = bb.width(), h = bb.height();
  if (w <= 0 || h <= 0) return false;
  const int heightPx = std::max(1, static_cast<int>(static_cast<double>(widthPx) * h / w));
  std::vector<unsigned char> img(widthPx * heightPx * 3, 240);

  for (auto sid : view.shapeIds()) {
    const auto* s = view.findShape(sid);
    if (!s || s->kind() != db::DbShapeKind::Rect) continue;
    const auto& b = static_cast<const db::DbRect*>(s)->box();
    const auto* layer = lib.findLayer(s->layerId());
    unsigned char r = 128, g = 128, bl = 128;
    if (layer) {
      const auto& col = layer->color();
      if (col.size() == 7 && col[0] == '#') {
        try {
          r = static_cast<unsigned char>(std::stoi(col.substr(1, 2), nullptr, 16));
          g = static_cast<unsigned char>(std::stoi(col.substr(3, 2), nullptr, 16));
          bl = static_cast<unsigned char>(std::stoi(col.substr(5, 2), nullptr, 16));
        } catch (...) {}
      }
    }
    int x0 = static_cast<int>((b.left() - bb.left()) * widthPx / w);
    int y0 = static_cast<int>((b.bottom() - bb.bottom()) * heightPx / h);
    int x1 = static_cast<int>((b.right() - bb.left()) * widthPx / w);
    int y1 = static_cast<int>((b.top() - bb.bottom()) * heightPx / h);
    x0 = std::clamp(x0, 0, widthPx - 1);
    x1 = std::clamp(x1, 0, widthPx);
    y0 = std::clamp(y0, 0, heightPx - 1);
    y1 = std::clamp(y1, 0, heightPx);
    for (int y = y0; y < y1; ++y) {
      for (int x = x0; x < x1; ++x) {
        int idx = (y * widthPx + x) * 3;
        img[idx] = r; img[idx+1] = g; img[idx+2] = bl;
      }
    }
  }

  std::ofstream o(path, std::ios::binary);
  if (!o) return false;
  o << "P6\n" << widthPx << " " << heightPx << "\n255\n";
  o.write(reinterpret_cast<const char*>(img.data()), img.size());
  return true;
}

}  // namespace aurora::layout

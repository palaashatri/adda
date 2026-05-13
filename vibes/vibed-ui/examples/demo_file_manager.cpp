// MIT License
// Copyright (c) 2026 Palaash
//
// demo_file_manager.cpp — Windows Explorer-style file manager demo.
// Uses only the C++20 standard library (std::filesystem) — no external deps.
// Rendering is done directly via SoftwareRenderer for maximum performance.

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX          // prevent windows.h from defining min/max macros
#  include <windows.h>
#endif

#include "core/Application.h"
#include "core/Event.h"
#include "platform/Platform.h"
#include "render/legacy/SoftwareRenderer.h"

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Layout constants
// ---------------------------------------------------------------------------
static constexpr int kWinW      = 1100;
static constexpr int kWinH      = 720;
static constexpr int kTopH      = 44;    // address / toolbar row
static constexpr int kSideW     = 190;   // quick-access sidebar
static constexpr int kColHdrH   = 26;    // column header row
static constexpr int kStatusH   = 28;    // bottom status bar
static constexpr int kRowH      = 22;    // file list row height

static constexpr int kListY     = kTopH + kColHdrH;
static constexpr int kListH     = kWinH - kListY - kStatusH;
static constexpr int kListX     = kSideW;
static constexpr int kListW     = kWinW - kSideW;
static constexpr int kVisRows   = kListH / kRowH;

// Column x-offsets (relative to kListX)
static constexpr int kColName   = 8;
static constexpr int kColSize   = 420;
static constexpr int kColType   = 530;
static constexpr int kColDate   = 680;

// ---------------------------------------------------------------------------
// Colour palette
// ---------------------------------------------------------------------------
static constexpr uint32_t kBg          = 0xFF1E1E1EUL;
static constexpr uint32_t kTopBg       = 0xFF2D2D30UL;
static constexpr uint32_t kSideBg      = 0xFF252526UL;
static constexpr uint32_t kSideSel     = 0xFF37373DUL;
static constexpr uint32_t kColHdrBg    = 0xFF252526UL;
static constexpr uint32_t kRowEven     = 0xFF1E1E1EUL;
static constexpr uint32_t kRowOdd      = 0xFF222222UL;
static constexpr uint32_t kRowSel      = 0xFF264F78UL;
static constexpr uint32_t kRowHover    = 0xFF2A3F5AUL;
static constexpr uint32_t kStatusBg    = 0xFF2D2D30UL;
static constexpr uint32_t kBorder      = 0xFF3F3F46UL;
static constexpr uint32_t kText        = 0xFFD4D4D4UL;
static constexpr uint32_t kTextDim     = 0xFF808080UL;
static constexpr uint32_t kTextAccent  = 0xFF569CD6UL;
static constexpr uint32_t kFolderClr   = 0xFFE8BE6DUL;
static constexpr uint32_t kFileClr     = 0xFF9CDCFEUL;
static constexpr uint32_t kBtnBg       = 0xFF3C3C3CUL;
static constexpr uint32_t kBtnHover    = 0xFF505050UL;

// ---------------------------------------------------------------------------
// Data types
// ---------------------------------------------------------------------------
enum class SortKey { Name, Size, Type, Date };

struct FileEntry {
    std::string name;
    bool        isDir  = false;
    uintmax_t   size   = 0;
    std::string ext;   // lowercase extension without dot, or "" for folders
    std::string date;  // "YYYY-MM-DD"
    fs::path    full;
};

struct QuickEntry {
    std::string label;
    fs::path    path;
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string fmtSize(uintmax_t bytes) {
    if (bytes == 0) return "";
    if (bytes < 1024ULL)              return std::to_string(bytes) + " B";
    if (bytes < 1024ULL * 1024)       return std::to_string(bytes / 1024) + " KB";
    if (bytes < 1024ULL * 1024 * 1024) return std::to_string(bytes / (1024 * 1024)) + " MB";
    return std::to_string(bytes / (1024ULL * 1024 * 1024)) + " GB";
}

static std::string fmtDate(const fs::file_time_type& ft) {
    using namespace std::chrono;
    auto sc = time_point_cast<system_clock::duration>(
        ft - fs::file_time_type::clock::now() + system_clock::now());
    std::time_t t = system_clock::to_time_t(sc);
    std::tm tm   = {};
#if defined(_MSC_VER)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char buf[12];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
                  tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
    return buf;
}

static std::string toLower(std::string s) {
    for (char& c : s) { c = static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }
    return s;
}

static std::vector<FileEntry> loadDir(const fs::path& dir, SortKey key, bool ascending) {
    std::vector<FileEntry> entries;
    std::error_code ec;
    for (const auto& de : fs::directory_iterator(dir, ec)) {
        FileEntry fe;
        fe.full  = de.path();
        fe.name  = de.path().filename().string();
        fe.isDir = de.is_directory(ec);
        if (!fe.isDir) {
            fe.size = de.file_size(ec);
            if (ec) fe.size = 0;
            fe.ext = toLower(de.path().extension().string());
            if (!fe.ext.empty() && fe.ext[0] == '.') fe.ext = fe.ext.substr(1);
        }
        auto ft = de.last_write_time(ec);
        if (!ec) fe.date = fmtDate(ft);
        entries.push_back(std::move(fe));
    }

    auto cmp = [&](const FileEntry& a, const FileEntry& b) -> bool {
        // Folders always precede files
        if (a.isDir != b.isDir) return a.isDir > b.isDir;
        bool less = false;
        switch (key) {
            case SortKey::Name: less = toLower(a.name) < toLower(b.name); break;
            case SortKey::Size: less = a.size < b.size;                   break;
            case SortKey::Type: less = a.ext  < b.ext;                    break;
            case SortKey::Date: less = a.date < b.date;                   break;
        }
        return ascending ? less : !less;
    };
    std::sort(entries.begin(), entries.end(), cmp);
    return entries;
}

static std::string safeGetenv(const char* name) {
#if defined(_MSC_VER)
    char* val = nullptr;
    std::size_t len = 0;
    if (_dupenv_s(&val, &len, name) == 0 && val != nullptr) {
        std::string s(val);
        free(val);
        return s;
    }
    return {};
#else
    const char* v = std::getenv(name);
    return v ? std::string(v) : std::string{};
#endif
}

static std::vector<QuickEntry> buildQuickAccess() {
    std::vector<QuickEntry> qa;
    std::error_code ec;

#if defined(_WIN32)
    std::string homeStr = safeGetenv("USERPROFILE");
    if (homeStr.empty()) homeStr = safeGetenv("HOME");
    if (!homeStr.empty()) {
        const char* home = homeStr.c_str();  // used below
        fs::path hp(home);
        qa.push_back({"Home",      hp});
        qa.push_back({"Desktop",   hp / "Desktop"});
        qa.push_back({"Documents", hp / "Documents"});
        qa.push_back({"Downloads", hp / "Downloads"});
        qa.push_back({"Pictures",  hp / "Pictures"});
        qa.push_back({"Music",     hp / "Music"});
        qa.push_back({"Videos",    hp / "Videos"});
    }
    // Add visible drive letters
    for (char d = 'A'; d <= 'Z'; ++d) {
        std::string root;
        root += d; root += ":\\";
        fs::path rp(root);
        if (fs::exists(rp, ec) && !ec) {
            qa.push_back({root, rp});
        }
    }
#else
    std::string unixHome = safeGetenv("HOME");
    qa.push_back({"Home", fs::path(unixHome.empty() ? "/home" : unixHome)});
    qa.push_back({"Root", fs::path("/")});
    qa.push_back({"Tmp",  fs::path("/tmp")});
#endif
    return qa;
}

// Truncate a string so it fits within maxChars characters (adds "…" if cut)
static std::string truncate(const std::string& s, std::size_t maxChars) {
    if (s.size() <= maxChars) return s;
    return s.substr(0, maxChars - 1) + ">";
}

// ---------------------------------------------------------------------------
// Drawing helpers (thin wrappers around renderer)
// ---------------------------------------------------------------------------
static void drawTextC(render::SoftwareRenderer& r, int x, int y, const std::string& text, uint32_t color) {
    r.setTextColor(color);
    r.drawText(x, y, text);
}

static void drawBtn(render::SoftwareRenderer& r, int x, int y, int w, int h,
                    const std::string& label, bool hovered) {
    r.drawRect(x, y, w, h, hovered ? kBtnHover : kBtnBg);
    r.drawRect(x, y, w, 1, kBorder);
    r.drawRect(x, y + h - 1, w, 1, kBorder);
    r.drawRect(x, y, 1, h, kBorder);
    r.drawRect(x + w - 1, y, 1, h, kBorder);
    drawTextC(r, x + 6, y + 6, label, kText);
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main() {
    core::Application app;
    app.setWindowSize(kWinW, kWinH);
    if (!app.initialize()) return 1;

    render::SoftwareRenderer renderer(kWinW, kWinH);

    // Detect HDR display and enable HDR pipeline if supported
    platform::Platform* plt = platform::activePlatform();
    if (plt && plt->isHdrCapable()) {
        renderer.setHdrMode(true);
        plt->setHdrEnabled(true);
    }

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------
    std::vector<QuickEntry>  quickAccess  = buildQuickAccess();
    std::vector<fs::path>    history;         // visited paths (back-stack)
    std::vector<fs::path>    forwardStack;    // forward navigation stack
    int                      sideHover   = -1;

    fs::path      cwd;
    {
        std::error_code ec;
        cwd = fs::current_path(ec);
        if (ec) cwd = fs::path(".");
    }

    SortKey  sortKey       = SortKey::Name;
    bool     sortAscending = true;
    int      selected      = -1;
    int      scrollOffset  = 0;
    int      hoverRow      = -1;
    bool     dirty         = true;

    // Search / rename state
    bool        searchActive = false;
    std::string searchText;
    bool        renameActive = false;
    std::string renameText;

    // Status message (transient)
    std::string statusMsg;

    std::vector<FileEntry> entries;

    auto navigate = [&](const fs::path& target) {
        std::error_code ec;
        if (!fs::is_directory(target, ec) || ec) {
            statusMsg = "Cannot open: " + target.string();
            dirty = true;
            return;
        }
        if (!cwd.empty()) {
            history.push_back(cwd);
            forwardStack.clear();
        }
        cwd          = target;
        selected     = -1;
        scrollOffset = 0;
        searchActive = false;
        searchText.clear();
        renameActive = false;
        entries      = loadDir(cwd, sortKey, sortAscending);
        statusMsg.clear();
        dirty = true;
    };

    auto refresh = [&]() {
        entries = loadDir(cwd, sortKey, sortAscending);
        if (selected >= static_cast<int>(entries.size())) selected = -1;
        dirty = true;
    };

    navigate(cwd);  // initial load (doesn't push history because history is empty)

    // Button hover state
    bool hovBack = false, hovFwd = false, hovUp = false, hovRefresh = false;

    // -----------------------------------------------------------------------
    // Event loop
    // -----------------------------------------------------------------------
    bool running = true;
    while (running) {
        if (platform::activePlatform()) {
            platform::activePlatform()->pumpEvents();
        }

        bool hadEvent = false;
        core::Event ev;
        while (core::EventQueue::poll(ev)) {
            hadEvent = true;

            if (ev.type == core::EventType::Quit) {
                running = false;
                break;
            }

            // ----------------------------------------------------------------
            // Mouse events
            // ----------------------------------------------------------------
            if (ev.type == core::EventType::MouseMove) {
                const int mx = ev.x, my = ev.y;
                // Sidebar hover
                const int newSideHover = (mx < kSideW && my >= kTopH && my < kWinH - kStatusH)
                    ? (my - kTopH) / 26 : -1;
                if (newSideHover != sideHover) { sideHover = newSideHover; dirty = true; }

                // Toolbar button hover
                bool nb = (mx >= 4   && mx < 44  && my >= 6 && my < 34);
                bool nf = (mx >= 50  && mx < 90  && my >= 6 && my < 34);
                bool nu = (mx >= 96  && mx < 136 && my >= 6 && my < 34);
                bool nr = (mx >= 142 && mx < 182 && my >= 6 && my < 34);
                if (nb!=hovBack||nf!=hovFwd||nu!=hovUp||nr!=hovRefresh) {
                    hovBack=nb; hovFwd=nf; hovUp=nu; hovRefresh=nr; dirty=true;
                }

                // List hover
                if (mx >= kListX && my >= kListY && my < kListY + kListH) {
                    const int row = (my - kListY) / kRowH;
                    if (row != hoverRow) { hoverRow = row; dirty = true; }
                } else if (hoverRow != -1) { hoverRow = -1; dirty = true; }
            }

            if (ev.type == core::EventType::MouseDown) {
                const int mx = ev.x, my = ev.y;

                // Toolbar buttons
                if (my >= 6 && my < 34) {
                    if (mx >= 4 && mx < 44) {   // Back
                        if (!history.empty()) {
                            forwardStack.push_back(cwd);
                            fs::path prev = history.back();
                            history.pop_back();
                            cwd          = prev;
                            selected     = -1;
                            scrollOffset = 0;
                            entries      = loadDir(cwd, sortKey, sortAscending);
                            dirty = true;
                        }
                    } else if (mx >= 50 && mx < 90) {  // Forward
                        if (!forwardStack.empty()) {
                            history.push_back(cwd);
                            fs::path nxt = forwardStack.back();
                            forwardStack.pop_back();
                            cwd          = nxt;
                            selected     = -1;
                            scrollOffset = 0;
                            entries      = loadDir(cwd, sortKey, sortAscending);
                            dirty = true;
                        }
                    } else if (mx >= 96 && mx < 136) {  // Up
                        fs::path parent = cwd.parent_path();
                        if (parent != cwd) navigate(parent);
                    } else if (mx >= 142 && mx < 182) { // Refresh
                        refresh();
                    }
                }

                // Sidebar click
                if (mx < kSideW && my >= kTopH) {
                    const int idx = (my - kTopH) / 26;
                    if (idx >= 0 && idx < static_cast<int>(quickAccess.size())) {
                        navigate(quickAccess[idx].path);
                    }
                }

                // Column header sort toggle
                if (my >= kTopH && my < kTopH + kColHdrH && mx >= kListX) {
                    const int lx = mx - kListX;
                    SortKey newKey = sortKey;
                    if (lx < kColSize)       newKey = SortKey::Name;
                    else if (lx < kColType)  newKey = SortKey::Size;
                    else if (lx < kColDate)  newKey = SortKey::Type;
                    else                     newKey = SortKey::Date;
                    if (newKey == sortKey) sortAscending = !sortAscending;
                    else { sortKey = newKey; sortAscending = true; }
                    refresh();
                }

                // File list click
                if (mx >= kListX && my >= kListY && my < kListY + kListH) {
                    const int row = (my - kListY) / kRowH + scrollOffset;
                    if (row >= 0 && row < static_cast<int>(entries.size())) {
                        if (selected == row) {
                            // Double-click effect (same row clicked twice)
                            if (entries[row].isDir) {
                                navigate(entries[row].full);
                            }
                        } else {
                            selected = row;
                            dirty = true;
                        }
                    }
                }
            }

            // ----------------------------------------------------------------
            // Keyboard events
            // ----------------------------------------------------------------
            if (ev.type == core::EventType::KeyDown) {
                if (renameActive) {
                    const int code = ev.textCode != 0 ? ev.textCode : ev.keyCode;
                    if (code == 13 || code == 10) {  // Enter — commit rename
                        if (selected >= 0 && selected < static_cast<int>(entries.size())
                            && !renameText.empty()) {
                            std::error_code ec;
                            fs::path dest = entries[selected].full.parent_path() / renameText;
                            fs::rename(entries[selected].full, dest, ec);
                            if (!ec) { statusMsg = "Renamed to " + renameText; refresh(); }
                            else     { statusMsg = "Rename failed: " + ec.message(); dirty = true; }
                        }
                        renameActive = false;
                    } else if (code == 27) {  // Escape
                        renameActive = false; dirty = true;
                    } else if (code == 8 || code == 127) {  // Backspace
                        if (!renameText.empty()) renameText.pop_back();
                        dirty = true;
                    } else if (code >= 32 && code <= 126) {
                        renameText.push_back(static_cast<char>(code));
                        dirty = true;
                    }
                    continue;
                }

                if (searchActive) {
                    const int code = ev.textCode != 0 ? ev.textCode : ev.keyCode;
                    if (code == 13 || code == 10 || code == 27) {
                        searchActive = false; dirty = true;
                    } else if (code == 8 || code == 127) {
                        if (!searchText.empty()) searchText.pop_back();
                        // Find first match
                        selected = -1;
                        const std::string lq = toLower(searchText);
                        for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
                            if (toLower(entries[i].name).find(lq) != std::string::npos) {
                                selected = i;
                                scrollOffset = std::max(0, i - kVisRows / 2);
                                break;
                            }
                        }
                        dirty = true;
                    } else if (code >= 32 && code <= 126) {
                        searchText.push_back(static_cast<char>(code));
                        const std::string lq = toLower(searchText);
                        selected = -1;
                        for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
                            if (toLower(entries[i].name).find(lq) != std::string::npos) {
                                selected = i;
                                scrollOffset = std::max(0, i - kVisRows / 2);
                                break;
                            }
                        }
                        dirty = true;
                    }
                    continue;
                }

                // Normal key handling
                const int vk = ev.keyCode;
                const int ch = ev.textCode;

                if (vk == VK_UP || vk == 38) {
                    if (selected > 0) { --selected; dirty = true; }
                    if (selected < scrollOffset) scrollOffset = selected;
                }
                if (vk == VK_DOWN || vk == 40) {
                    if (selected < static_cast<int>(entries.size()) - 1) { ++selected; dirty = true; }
                    if (selected >= scrollOffset + kVisRows) scrollOffset = selected - kVisRows + 1;
                }
                if (vk == VK_PRIOR) {  // Page Up
                    selected     = std::max(0, selected - kVisRows);
                    scrollOffset = std::max(0, scrollOffset - kVisRows);
                    dirty = true;
                }
                if (vk == VK_NEXT) {  // Page Down
                    selected     = std::min(static_cast<int>(entries.size()) - 1, selected + kVisRows);
                    scrollOffset = std::min(std::max(0, static_cast<int>(entries.size()) - kVisRows),
                                            scrollOffset + kVisRows);
                    dirty = true;
                }
                if (vk == VK_HOME) { selected = 0; scrollOffset = 0; dirty = true; }
                if (vk == VK_END) {
                    selected = std::max(0, static_cast<int>(entries.size()) - 1);
                    scrollOffset = std::max(0, selected - kVisRows + 1);
                    dirty = true;
                }
                if (vk == 13 || vk == 10) {  // Enter — open
                    if (selected >= 0 && selected < static_cast<int>(entries.size())) {
                        if (entries[selected].isDir) navigate(entries[selected].full);
                    }
                }
                if (vk == VK_BACK || vk == 8) {  // Backspace — go up
                    fs::path parent = cwd.parent_path();
                    if (parent != cwd) navigate(parent);
                }
                if (vk == VK_DELETE || vk == 46) {  // Delete
                    if (selected >= 0 && selected < static_cast<int>(entries.size())) {
                        std::error_code ec;
                        fs::remove_all(entries[selected].full, ec);
                        if (!ec) { statusMsg = "Deleted: " + entries[selected].name; refresh(); }
                        else     { statusMsg = "Delete failed: " + ec.message(); dirty = true; }
                    }
                }
                if (vk == VK_F5 || vk == 0x74) { refresh(); }
                if (vk == VK_F2 || vk == 0x71) {  // F2 — rename
                    if (selected >= 0 && selected < static_cast<int>(entries.size())) {
                        renameText   = entries[selected].name;
                        renameActive = true;
                        dirty = true;
                    }
                }

                // Ctrl+F — search
                if (ch == 6 || (vk == 'F' && (GetKeyState(VK_CONTROL) & 0x8000))) {
                    searchActive = true;
                    searchText.clear();
                    dirty = true;
                }
                // Escape — cancel search/rename
                if (vk == VK_ESCAPE || vk == 27) {
                    searchActive = false; renameActive = false; dirty = true;
                }
            }

            if (ev.type == core::EventType::MouseMove || ev.type == core::EventType::MouseDown) {
                dirty = true;
            }
        }

        if (!dirty) {
            std::this_thread::sleep_for(std::chrono::milliseconds(8));
            continue;
        }
        dirty = false;

        // -------------------------------------------------------------------
        // Render
        // -------------------------------------------------------------------
        renderer.clear(kBg);

        // --- Top toolbar ---------------------------------------------------
        renderer.drawRect(0, 0, kWinW, kTopH, kTopBg);
        renderer.drawRect(0, kTopH - 1, kWinW, 1, kBorder);

        drawBtn(renderer,   4, 6, 40, 28, "<", hovBack && !history.empty());
        drawBtn(renderer,  50, 6, 40, 28, ">", hovFwd  && !forwardStack.empty());
        drawBtn(renderer,  96, 6, 40, 28, "^", hovUp);
        drawBtn(renderer, 142, 6, 40, 28, "R", hovRefresh);

        // Address bar
        renderer.drawRect(190, 8, kWinW - 300, 28, 0xFF2A2A2AUL);
        renderer.drawRect(190, 8, kWinW - 300, 1, kBorder);
        renderer.drawRect(190, 35, kWinW - 300, 1, kBorder);
        const std::string pathStr = truncate(cwd.string(), 60);
        drawTextC(renderer, 198, 15, pathStr, kText);

        // Search box or indicator
        if (searchActive) {
            renderer.drawRect(kWinW - 102, 8, 98, 28, 0xFF333355UL);
            drawTextC(renderer, kWinW - 96, 15, "?" + searchText + "|", kTextAccent);
        } else {
            renderer.drawRect(kWinW - 102, 8, 98, 28, 0xFF2A2A2AUL);
            drawTextC(renderer, kWinW - 96, 15, "Ctrl+F", kTextDim);
        }

        // --- Sidebar -------------------------------------------------------
        renderer.drawRect(0, kTopH, kSideW, kWinH - kTopH - kStatusH, kSideBg);
        renderer.drawRect(kSideW - 1, kTopH, 1, kWinH - kTopH - kStatusH, kBorder);

        drawTextC(renderer, 8, kTopH + 6, "Quick Access", kTextDim);

        for (int i = 0; i < static_cast<int>(quickAccess.size()); ++i) {
            const int ey = kTopH + 26 + i * 26;
            if (ey + 26 > kWinH - kStatusH) break;

            const bool isActive = (quickAccess[i].path == cwd);
            if (isActive || sideHover == i + 1) {
                renderer.drawRect(0, ey, kSideW - 1, 26, isActive ? kSideSel : 0xFF2E2E2EUL);
            }
            const std::string label = truncate(quickAccess[i].label, 20);
            drawTextC(renderer, 14, ey + 6, label, isActive ? kTextAccent : kText);
        }

        // --- Column headers ------------------------------------------------
        renderer.drawRect(kListX, kTopH, kListW, kColHdrH, kColHdrBg);
        renderer.drawRect(kListX, kTopH + kColHdrH - 1, kListW, 1, kBorder);

        auto colLabel = [&](const char* label, SortKey sk, int relX) {
            std::string txt = label;
            if (sortKey == sk) txt += sortAscending ? " ^" : " v";
            drawTextC(renderer, kListX + relX, kTopH + 5, txt, kText);
        };
        colLabel("Name",     SortKey::Name, kColName);
        colLabel("Size",     SortKey::Size, kColSize);
        colLabel("Type",     SortKey::Type, kColType);
        colLabel("Modified", SortKey::Date, kColDate);

        // Separator lines
        renderer.drawRect(kListX + kColSize - 4, kTopH, 1, kColHdrH, kBorder);
        renderer.drawRect(kListX + kColType - 4, kTopH, 1, kColHdrH, kBorder);
        renderer.drawRect(kListX + kColDate - 4, kTopH, 1, kColHdrH, kBorder);

        // --- File list -----------------------------------------------------
        const int visibleCount = std::min(kVisRows,
            static_cast<int>(entries.size()) - scrollOffset);

        for (int vi = 0; vi < kVisRows; ++vi) {
            const int row = vi + scrollOffset;
            const int ry  = kListY + vi * kRowH;

            uint32_t rowBg = (vi % 2 == 0) ? kRowEven : kRowOdd;
            if (row == selected)   rowBg = kRowSel;
            else if (vi == hoverRow) rowBg = kRowHover;

            renderer.drawRect(kListX, ry, kListW, kRowH, rowBg);

            if (row >= static_cast<int>(entries.size())) continue;
            const FileEntry& fe = entries[row];

            // Icon colour: folder vs file
            const uint32_t iconClr = fe.isDir ? kFolderClr : kFileClr;
            renderer.drawRect(kListX + kColName, ry + 6, 8, 10, iconClr);

            // Name (with optional rename edit)
            if (renameActive && row == selected) {
                renderer.drawRect(kListX + kColName + 12, ry + 2, 300, kRowH - 4, 0xFF1A1A3AUL);
                drawTextC(renderer, kListX + kColName + 14, ry + 5, renameText + "|", kTextAccent);
            } else {
                const std::string name = truncate(fe.name, 36);
                drawTextC(renderer, kListX + kColName + 12, ry + 5, name,
                          fe.isDir ? kFolderClr : kText);
            }

            // Size
            if (!fe.isDir) {
                drawTextC(renderer, kListX + kColSize, ry + 5, fmtSize(fe.size), kTextDim);
            }

            // Type
            const std::string typeStr = fe.isDir ? "Folder" : (fe.ext.empty() ? "File" : fe.ext);
            drawTextC(renderer, kListX + kColType, ry + 5, truncate(typeStr, 12), kTextDim);

            // Date
            drawTextC(renderer, kListX + kColDate, ry + 5, fe.date, kTextDim);
        }

        // Scrollbar
        if (static_cast<int>(entries.size()) > kVisRows) {
            const float ratio      = static_cast<float>(kVisRows) / entries.size();
            const float offsetRatio = static_cast<float>(scrollOffset) / entries.size();
            const int sbH = static_cast<int>(kListH * ratio);
            const int sbY = kListY + static_cast<int>(kListH * offsetRatio);
            renderer.drawRect(kWinW - 6, kListY, 6, kListH, 0xFF2A2A2AUL);
            renderer.drawRect(kWinW - 5, sbY, 4, sbH, 0xFF555555UL);
        }

        // --- Status bar ----------------------------------------------------
        renderer.drawRect(0, kWinH - kStatusH, kWinW, kStatusH, kStatusBg);
        renderer.drawRect(0, kWinH - kStatusH, kWinW, 1, kBorder);

        std::string status;
        if (!statusMsg.empty()) {
            status = statusMsg;
        } else {
            int numFolders = 0, numFiles = 0;
            for (const auto& e : entries) { e.isDir ? ++numFolders : ++numFiles; }
            status = std::to_string(entries.size()) + " items  |  "
                   + std::to_string(numFolders) + " folders, "
                   + std::to_string(numFiles) + " files";
            if (selected >= 0 && selected < static_cast<int>(entries.size())) {
                const FileEntry& se = entries[selected];
                status += "  |  " + se.name;
                if (!se.isDir) status += "  " + fmtSize(se.size);
            }
        }
        drawTextC(renderer, 8, kWinH - kStatusH + 7, truncate(status, 110), kTextDim);

        // HDR / VRR badge
        if (plt && plt->isHdrCapable()) {
            drawTextC(renderer, kWinW - 100, kWinH - kStatusH + 7, "HDR", kTextAccent);
        }
        const std::string rateStr = std::to_string(plt ? plt->displayRefreshRate() : 60) + "Hz";
        drawTextC(renderer, kWinW - 60, kWinH - kStatusH + 7, rateStr, kTextDim);

        // Present (DwmFlush is called inside WinPlatform::blit for VRR sync)
        renderer.present();

        (void)visibleCount;
        std::this_thread::sleep_for(std::chrono::milliseconds(4));
    }

    app.shutdown();
    return 0;
}

#ifndef EXP_ELECTRON_HPP
#define EXP_ELECTRON_HPP

#include <numbers>
#include <optional>
#include <string_view>
#include <vector>

constexpr auto square(double x) -> double { return x * x; }

struct DateMasurate {
    double tensiune;
    double curent;
    double raza;
};

// Val teoretica
inline constexpr double kElectron_sarcina_teoretic{1.758'820'008'38e11};

// C-te
inline constexpr double kMu_vid{4e-7 * std::numbers::pi};
inline constexpr int kBobina_spire{154};
inline constexpr double kBobina_raza{0.2};
// Factor care apare des, de evitat magic numbers
inline constexpr double kAlpha{
    (125.0 / 32.0) * (square(kBobina_raza) / square(kMu_vid * kBobina_spire))};
auto calc_sarcina(const DateMasurate& masura) -> double;

auto slope(const std::vector<double>& x, const std::vector<double>& y)
    -> double;
auto mean(const std::vector<double>& vals) -> double;
auto std_deviation(const std::vector<double>& vals) -> double;
auto citire_opt(std::string_view msg) -> std::optional<double>;
auto citire(std::string_view msg) -> double;
auto print_tabel(const std::vector<DateMasurate>& tab, std::string_view tag_x,
                 const std::vector<double>& x, std::string_view tag_y,
                 const std::vector<double>& y) -> void;
auto print_rez(double sarcina) -> void;
auto tabel_01() -> void;
auto tabel_02() -> void;
auto tabel_03() -> void;
auto tabel_04() -> void;
auto tabel_05() -> void;

#endif

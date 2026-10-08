#include "exp_electron.hpp"
#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <fmt/format.h>
#include <iostream>
#include <numeric>
#include <ranges>
// Cool ranges

auto calc_sarcina(const DateMasurate& masura) -> double
{ return kAlpha * (masura.tensiune / square(masura.raza * masura.curent)); }

auto slope(const std::vector<double>& x, const std::vector<double>& y) -> double
{
    return std::transform_reduce(x.begin(), x.end(), y.begin(), 0.0) /
           std::transform_reduce(x.begin(), x.end(), x.begin(), 0.0);
}
auto mean(const std::vector<double>& vals) -> double
{
    return std::reduce(vals.begin(), vals.end()) /
           static_cast<double>(vals.size());
}

auto std_deviation(const std::vector<double>& vals) -> double
{
    const double m{mean(vals)}; // I_m
    const std::size_t n{vals.size()};
    const double suma_patrate{std::transform_reduce(
        vals.begin(), vals.end(), 0.0, std::plus{},
        [m](double x) -> double { return square(x - m); })};

    return std::sqrt(suma_patrate / static_cast<double>(n * (n - 1)));
}

// printing (mostly vibe)
auto citire_opt(std::string_view msg) -> std::optional<double>
{
    for (std::string str;;) {
        fmt::print("  {}: ", msg);
        std::fflush(stdout);
        if (!(std::cin >> str)) { std::exit(EXIT_FAILURE); }
        if (str == "-") { return std::nullopt; }
        std::ranges::replace(str, ',', '.');

        double val{};
        const char *const first{std::to_address(str.cbegin())};
        const char *const last{std::to_address(str.cend())};
        if (const auto [pos, ec]{std::from_chars(first, last, val)};
            ec == std::errc{} && pos == last && val > 0.0) {
            return val;
        }

        fmt::println("  '{}' nu e pozitiv", str);
    }
}

auto citire(std::string_view msg) -> double
{
    for (;;) {
        if (const auto val{citire_opt(msg)}) { return *val; }
    }
}

auto print_tabel(const std::vector<DateMasurate>& tab, std::string_view tag_x,
                 const std::vector<double>& x, std::string_view tag_y,
                 const std::vector<double>& y) -> void
{
    assert(tab.size() == x.size() && x.size() == y.size());

    fmt::println("\n  {:>6} {:>6} {:>6} | {:>10} {:>10} | {:>10}", "r (cm)",
                 "U (V)", "I (A)", tag_x, tag_y, "e/m (C/kg)");

    for (const auto&& [masurat, x_i, y_i] : std::views::zip(tab, x, y)) {
        fmt::println(
            "  {:>6.1f} {:>6.0f} {:>6.2f} | {:>10.4g} {:>10.4g} | {:>10.3e}",
            100.0 * masurat.raza, masurat.tensiune, masurat.curent, x_i, y_i,
            calc_sarcina(masurat));
    }
}

auto print_rez(double sarcina) -> void
{
    fmt::println("  e/m = {:.4e} C/kg   ({:+.1f} % fata de valoarea teoretica)",
                 sarcina,
                 100.0 * ((sarcina / kElectron_sarcina_teoretic) - 1.0));
}

auto tabel_01() -> void
{
    fmt::println("\n-- Tabel 1: U = 160 V, r = 4 cm, I masurat de 5 ori");

    std::vector<double> curenti{};
    for (int i{1}; i <= 5; ++i) {
        curenti.push_back(citire(fmt::format("I{} (A)", i)));
    }

    const double i_m{mean(curenti)};
    const double sigma{std_deviation(curenti)};
    const double b{std::pow(0.8, 1.5) * kMu_vid * kBobina_spire * i_m /
                   kBobina_raza};
    const double sarcina{
        calc_sarcina({.tensiune = 160.0, .curent = i_m, .raza = 0.04})};
    fmt::println("\n  Im = {:.4f} A   sigma_Im = {:.4f} A   eps_Im = {:.2f} %  "
                 " B = {:.4e} T",
                 i_m, sigma, 100.0 * sigma / i_m, b);
    fmt::println("  sigma_e/m = {:.3e} C/kg", 2.0 * (sigma / i_m) * sarcina);
    print_rez(sarcina);
}

auto tabel_02() -> void // I = a * (1/r), U fix
{
    constexpr double u{160.0};
    fmt::println("\n-- Tabel 2: U = 160 V, I pentru fiecare raza");

    std::vector<DateMasurate> tab;
    std::vector<double> x;
    std::vector<double> y;

    for (const double r_cm : {5.0, 4.0, 3.0, 2.0}) {
        tab.push_back({
            .tensiune = u,
            .curent   = citire(fmt::format("r = {} cm, I (A)", r_cm)),
            .raza     = r_cm / 100.0,
        });
        x.push_back(1.0 / tab.back().raza);
        y.push_back(tab.back().curent);
    }
    print_tabel(tab, "1/r (1/m)", x, "I (A)", y);
    const double a{slope(x, y)};
    fmt::println("  panta a = {:.4e} A*m", a);
    print_rez(kAlpha * u / square(a));
}

auto tabel_03() -> void // I^2 = b * U, r fix
{
    constexpr double r{0.03};

    fmt::println("\n-- Tabel 3: r = 3 cm, I pentru fiecare tensiune");

    std::vector<DateMasurate> tab;
    std::vector<double> x;
    std::vector<double> y;

    for (const double u : {120.0, 160.0, 200.0, 240.0, 280.0}) {
        tab.push_back({.tensiune = u,
                       .curent   = citire(fmt::format("U = {} V, I (A)", u)),
                       .raza     = r});
        x.push_back(u);
        y.push_back(square(tab.back().curent));
    }
    print_tabel(tab, "U (V)", x, "I^2 (A^2)", y);

    const double b{slope(x, y)};
    fmt::println("  panta b = {:.4e} A^2/V", b);
    print_rez(kAlpha / (square(r) * b));
}

auto tabel_04() -> void // U = c * r^2, I fixat
{
    constexpr double i{1.70};

    fmt::println("\n-- Tabel 4: I = 1.70 A, U pentru fiecare raza");

    std::vector<DateMasurate> tab;
    std::vector<double> x;
    std::vector<double> y;

    for (const double r_cm : {3.0, 3.5, 4.0, 4.5, 5.0}) {
        tab.push_back(
            {.tensiune = citire(fmt::format("r = {} cm, U (V)", r_cm)),
             .curent   = i,
             .raza     = r_cm / 100.0});
        x.push_back(square(tab.back().raza));
        y.push_back(tab.back().tensiune);
    }
    print_tabel(tab, "r^2 (m^2)", x, "U (V)", y);

    const double c{slope(x, y)};
    fmt::println("  panta c = {:.4e} V/m^2", c);
    print_rez(kAlpha * c / square(i));
}

auto tabel_05() -> void // I^2 = d * (U/r^2); "-" sare peste o celula
{
    fmt::println("\n-- Tabel 5: I pentru fiecare U si r ('-' = nemasurat)");

    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> sarcini;
    std::string grila;

    // NOLINTNEXTLINE
    for (double u{120.0}; u <= 300.0; u += 20.0) {
        grila += fmt::format("  {:>5.0f}", u);
        for (const double r_cm : {5.0, 4.0, 3.0, 2.0}) {
            const auto i{
                citire_opt(fmt::format("U = {} V, r = {} cm, I (A)", u, r_cm))};
            if (!i) {
                grila += fmt::format(" | {:>5} {:>10}", "-", "-");
                continue;
            }
            const DateMasurate m{
                .tensiune = u, .curent = *i, .raza = r_cm / 100.0};
            x.push_back(u / square(m.raza));
            y.push_back(square(*i));
            sarcini.push_back(calc_sarcina(m));
            grila += fmt::format(" | {:>5.2f} {:>10.3e}", *i, sarcini.back());
        }
        grila += '\n';
    }

    fmt::print("\n  {:>5}", "U (V)");
    for (const int r_cm : {5, 4, 3, 2}) {
        fmt::print(" | {:>16}", fmt::format("r={}cm: I, e/m", r_cm));
    }
    fmt::print("\n{}", grila);

    if (sarcini.size() < 2) { return; }

    const double sarcina{mean(sarcini)};
    const double sigma{std_deviation(sarcini)};
    fmt::println("  K = {}   sigma = {:.3e} C/kg   eps = {:.2f} %",
                 sarcini.size(), sigma, 100.0 * sigma / sarcina);
    print_rez(sarcina);

    const double d{slope(x, y)};
    fmt::println("  panta d = {:.4e} A^2*m^2/V", d);
    print_rez(kAlpha / d);
}

auto main() -> int
{
    tabel_01();
    tabel_02();
    tabel_03();
    tabel_04();
    fmt::print("\nTabel 5? [D/N] ");
    std::fflush(stdout);
    if (std::string rasp; std::cin >> rasp && (rasp == "d" || rasp == "D")) {
        tabel_05();
    }
}

//
// Created by sigsegv on 10/31/24.
//

#include "Duration.h"
#include "TimeConst.h"
#include <limits>

template <typename Y, typename M, typename D, typename H, typename I, typename S> constexpr std::string DurationToString(Y years, M months, D days, H hours, I minutes, S seconds) {
    auto noTime = hours == 0 && minutes == 0 && seconds == 0;
    auto noYM = years == 0 && months == 0;
    if (noYM && days == 0 && noTime) {
        return "PT0M";
    }
    std::string dstr{"P"};
    if (!noYM) {
        if (years != 0) {
            if (years < 0) {
                years = static_cast<typeof(years)>(0) - years;
                dstr.append("-");
            }
            AppendAsString(dstr, years);
            dstr.append("Y");
        }
        if (months != 0) {
            if (months < 0) {
                months = static_cast<typeof(months)>(0) - months;
                dstr.append("-");
            }
            AppendAsString(dstr, months);
            dstr.append("M");
        }
    } else if (noTime && (days % 7) == 0) {
        days /= 7;
        if (days < 0) {
            days = static_cast<typeof(days)>(0) - days;
            dstr.append("-");
        }
        AppendAsString(dstr, days);
        dstr.append("W");
        return dstr;
    }
    if (days != 0) {
        if (days < 0) {
            days = static_cast<typeof(days)>(0) - days;
            dstr.append("-");
        }
        AppendAsString(dstr, days);
        dstr.append("D");
    }
    if (noTime) {
        return dstr;
    }
    dstr.append("T");
    if (hours != 0) {
        if (hours < 0) {
            hours = static_cast<typeof(hours)>(0) - hours;
            dstr.append("-");
        }
        AppendAsString(dstr, hours);
        dstr.append("H");
    }
    if (minutes != 0) {
        if (minutes < 0) {
            minutes = static_cast<typeof(minutes)>(0) - minutes;
            dstr.append("-");
        }
        AppendAsString(dstr, minutes);
        dstr.append("M");
    }
    if (seconds != 0) {
        if (seconds < 0) {
            seconds = static_cast<typeof(seconds)>(0) - seconds;
            dstr.append("-");
        }
        AppendAsString(dstr, seconds);
        dstr.append("S");
    }
    return dstr;
}

static_assert(DurationToString(0, 0, 0, 0, 0, 0) == "PT0M");
static_assert(DurationToString(1, 1, 0, 0, 0, 0) == "P1Y1M");
static_assert(DurationToString(-1, -1, 0, 0, 0, 0) == "P-1Y-1M");
static_assert(DurationToString(0, 0, 1, 0, 0, 0) == "P1D");
static_assert(DurationToString(0, 0, -1, 0, 0, 0) == "P-1D");
static_assert(DurationToString(0, 0, 7, 0, 0, 0) == "P1W");
static_assert(DurationToString(0, 0, -7, 0, 0, 0) == "P-1W");
static_assert(DurationToString(0, 0, 0, 1, 1, 0) == "PT1H1M");
static_assert(DurationToString(0, 0, 0, -1, -1, 0) == "PT-1H-1M");
static_assert(DurationToString(0, 0, 0, 0, 0, 1) == "PT1S");
static_assert(DurationToString(0, 0, 0, 0, 0, -1) == "PT-1S");

enum class ParseDurationState {
    INIT=0,
    STARTED=1,
    YEARS=2,
    MONTHS=3,
    DAYS=4,
    WEEKS=5,
    TIME=6,
    HOURS=7,
    MINUTES=8,
    SECONDS=9
};

template <typename T, typename UT> constexpr T Negative(UT num) {
    static_assert(std::numeric_limits<UT>::min() == 0);
    static_assert(std::numeric_limits<T>::min() < 0);
    static_assert((std::numeric_limits<UT>::max() / 2) == std::numeric_limits<T>::max());
    T neg{0};
    neg -= num;
    return neg;
}

template <typename Y, typename M, typename D, typename H, typename I, typename S> constexpr bool ParseDuration(const std::string &dstr, Y &years, M &months, D &days, H &hours, I &minutes, S &seconds) {
    ParseDurationState state{ParseDurationState::INIT};
    uint64_t acc{0};
    Y y{0};
    M m{0};
    D d{0};
    H h{0};
    I i{0};
    S s{0};
    bool negative{false};
    bool active{false};
    for (auto ch : dstr) {
        if (state == ParseDurationState::INIT) {
            if (ch != 'P') {
                return false;
            }
            state = ParseDurationState::STARTED;
            continue;
        }
        if (ch == 'T') {
            if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::TIME)) {
                return false;
            }
            state = ParseDurationState::TIME;
            continue;
        }
        if (state == ParseDurationState::WEEKS || state == ParseDurationState::DAYS || state == ParseDurationState::SECONDS) {
            return false;
        }
        if (ch == '-') {
            if (negative || active) {
                return false;
            }
            negative = true;
            continue;
        }
        if (ch >= '0' && ch <= '9') {
            active = true;
            acc *= 10;
            acc += ch - '0';
            continue;
        }
        switch (ch) {
            case 'Y':
                if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::YEARS)) {
                    return false;
                }
                state = ParseDurationState::YEARS;
                if (!negative) {
                    y = static_cast<Y>(acc);
                } else {
                    y = static_cast<Y>(Negative<int64_t>(acc));
                }
                break;
            case 'M':
                if (static_cast<int>(state) < static_cast<int>(ParseDurationState::TIME)) {
                    if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::MONTHS)) {
                        return false;
                    }
                    uint64_t accYears = acc / 12;
                    acc = acc % 12;
                    int64_t dy = !negative ? static_cast<int64_t>(accYears) : Negative<int64_t>(accYears);
                    int8_t dm = !negative ? static_cast<int8_t>(acc) : Negative<int8_t>(static_cast<uint8_t>(acc));
                    y += dy;
                    m = dm;
                } else {
                    if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::MINUTES)) {
                        return false;
                    }
                    uint64_t accHours = acc / 60;
                    acc = acc % 60;
                    int64_t dh = !negative ? static_cast<int64_t>(accHours) : Negative<int64_t>(accHours);
                    int8_t di = !negative ? static_cast<int8_t>(acc) : Negative<int8_t>(static_cast<uint8_t>(acc));
                    dh += h;
                    h = dh;
                    i = di;
                }
                break;
            case 'D':
                if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::MONTHS)) {
                    return false;
                }
                {
                    uint64_t accYears = acc / daysPer400Years;
                    acc = acc % daysPer400Years;
                    int64_t dy = !negative ? static_cast<int64_t>(accYears) : Negative<int64_t>(accYears);
                    int32_t dd = !negative ? static_cast<int32_t>(acc) : Negative<int32_t>(static_cast<uint32_t>(acc));
                    dy *= 400;
                    y += dy;
                    d = dd;
                }
                break;
            case 'W':
                if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::WEEKS) || state == ParseDurationState::DAYS) {
                    return false;
                }
                {
                    acc *= 7;
                    uint64_t accYears = acc / daysPer400Years;
                    acc = acc % daysPer400Years;
                    int64_t dy = !negative ? static_cast<int64_t>(accYears) : Negative<int64_t>(accYears);
                    int32_t dd = !negative ? static_cast<int32_t>(acc) : Negative<int32_t>(static_cast<uint32_t>(acc));
                    dy *= 400;
                    y += dy;
                    d = dd;
                }
                break;
            case 'H':
                if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::HOURS)) {
                    return false;
                }
                {
                    int64_t dh = !negative ? static_cast<int64_t>(acc) : Negative<int64_t>(acc);
                    h = dh;
                }
                break;
            case 'S':
                if (static_cast<int>(state) >= static_cast<int>(ParseDurationState::SECONDS)) {
                    return false;
                }
                {
                    uint64_t accMinutes = acc / 60;
                    acc = acc % 60;
                    uint64_t accHours = accMinutes / 60;
                    accMinutes = accMinutes % 60;
                    int16_t dh = !negative ? static_cast<int64_t>(accHours) : Negative<int64_t>(accHours);
                    int16_t di = !negative ? static_cast<int16_t>(accMinutes) : Negative<int16_t>(static_cast<uint16_t>(accMinutes));
                    int8_t ds = !negative ? static_cast<int8_t>(acc) : Negative<int8_t>(static_cast<uint8_t>(acc));
                    di += i;
                    if (di > 60) {
                        di -= 60;
                        ++dh;
                    } else if (di < -60) {
                        di += 60;
                        --dh;
                    }
                    dh += h;
                    h = dh;
                    i = di;
                    s = ds;
                }
                break;
            default:
                return false;
        }
        acc = 0;
        negative = false;
        active = false;
    }
    if (negative || active || state == ParseDurationState::INIT) {
        return false;
    }
    years = y;
    months = m;
    days = d;
    hours = h;
    minutes = i;
    seconds = s;
    return true;
}

class DurationParser {
private:
    Duration duration;
public:
    constexpr DurationParser(const Duration &duration) : duration(duration) {}
    constexpr DurationParser(const std::string &str) : duration() {
        duration.valid = ParseDuration(str, duration.years, duration.months, duration.days, duration.hours, duration.minutes, duration.seconds);
    }
    constexpr explicit operator Duration () const {
        return duration;
    }
    constexpr explicit operator std::string () const {
        return DurationToString(duration.years, duration.months, duration.days, duration.hours, duration.minutes, duration.seconds);
    }
    constexpr explicit operator bool () const {
        return duration.valid;
    }
    constexpr typeof(duration.years) GetYears() const {
        return duration.years;
    }
    constexpr typeof(duration.months) GetMonths() const {
        return duration.months;
    }
    constexpr typeof(duration.days) GetDays() const {
        return duration.months;
    }
    constexpr bool NoYears() const {
        return duration.years == 0;
    }
    constexpr bool NoDays() const {
        return duration.days == 0;
    }
    constexpr bool NoTime() const {
        return duration.hours == 0 && duration.minutes == 0 && duration.seconds == 0;
    }
};

static_assert(std::string(DurationParser("PT0M")) == "PT0M");
static_assert(std::string(DurationParser("P1Y")) == "P1Y");
static_assert(std::string(DurationParser("P-1Y")) == "P-1Y");
static_assert(std::string(DurationParser("P1M")) == "P1M");
static_assert(std::string(DurationParser("P-1M")) == "P-1M");
static_assert(std::string(DurationParser("P13M")) == "P1Y1M");
static_assert(std::string(DurationParser("P-13M")) == "P-1Y-1M");
static_assert(std::string(DurationParser("P146096D")) == "P146096D");
static_assert(std::string(DurationParser("P-146096D")) == "P-146096D");
static_assert(std::string(DurationParser("P146097D")) == "P400Y");
static_assert(std::string(DurationParser("P-146097D")) == "P-400Y");
static_assert(std::string(DurationParser("P146098D")) == "P400Y1D");
static_assert(std::string(DurationParser("P-146098D")) == "P-400Y-1D");
static_assert(std::string(DurationParser("PT36H")) == "PT36H");
static_assert(std::string(DurationParser("PT-36H")) == "PT-36H");
static_assert(std::string(DurationParser("PT36M")) == "PT36M");
static_assert(std::string(DurationParser("PT-36M")) == "PT-36M");
static_assert(std::string(DurationParser("PT66M")) == "PT1H6M");
static_assert(std::string(DurationParser("PT-66M")) == "PT-1H-6M");
static_assert(std::string(DurationParser("PT36S")) == "PT36S");
static_assert(std::string(DurationParser("PT-36S")) == "PT-36S");
static_assert(std::string(DurationParser("PT66S")) == "PT1M6S");
static_assert(std::string(DurationParser("PT-66S")) == "PT-1M-6S");
static_assert(std::string(DurationParser("PT3599S")) == "PT59M59S");
static_assert(std::string(DurationParser("PT-3599S")) == "PT-59M-59S");
static_assert(std::string(DurationParser("PT3600S")) == "PT1H");
static_assert(std::string(DurationParser("PT-3600S")) == "PT-1H");
static_assert(std::string(DurationParser("PT3601S")) == "PT1H1S");
static_assert(std::string(DurationParser("PT-3601S")) == "PT-1H-1S");
static_assert(std::string(DurationParser("P3Y6M4DT12H30M5S")) == "P3Y6M4DT12H30M5S");

Duration Duration::FromString(const std::string &str) {
    DurationParser parser{str};
    return parser.operator Duration();
}

std::string Duration::ToString() const {
    if (!valid) {
        return {};
    }
    DurationParser parser{*this};
    return parser.operator std::string();
}

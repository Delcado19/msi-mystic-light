#pragma once
#include <QColor>
#include <cstring>

// Semantic chrome roles only; canvas artwork and output-state colors stay independent.
inline constexpr struct { const char* name; const char* value; } studioThemeColors[] = {
    {"@surround", "#06101E"},
    {"@chrome", "#091526"},
    {"@panel", "#0D1D30"},
    {"@input", "#071321"},
    {"@hover", "#19344B"},
    {"@selected", "#123D52"},
    {"@separator", "#294358"},
    {"@border", "#567A91"},
    {"@text", "#EAF7FF"},
    {"@secondary", "#AEC6D6"},
    {"@accent", "#45DDF5"},
    {"@section", "#122A40"},
};

inline QColor studioThemeColor(const char* role,int alpha=255)
{
    for(const auto& token:studioThemeColors) {
        if(std::strcmp(token.name,role)==0) {
            QColor color(token.value);color.setAlpha(alpha);return color;
        }
    }
    return {};
}

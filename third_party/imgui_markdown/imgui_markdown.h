#pragma once
#include "imgui.h"
#include <string>
#include <cstddef>

namespace ImGui {
    // Callback data structures
    struct MarkdownLinkCallbackData {
        const char* text;
        int textLength;
        const char* link;
        int linkLength;
        void* userData;
    };

    struct MarkdownTooltipCallbackData {
        const char* link;
        int linkLength;
        void* userData;
    };

    struct MarkdownImageCallbackData {
        const char* text;
        int textLength;
        const char* link;
        int linkLength;
        void* userData;
        ImVec2 imageSize;
    };

    struct MarkdownHeadingFormat {
        ImFont* font;
        bool separator;
    };

    // Main config structure
    struct MarkdownConfig {
        typedef void (*LinkCallback)(MarkdownLinkCallbackData data);
        typedef void (*TooltipCallback)(MarkdownTooltipCallbackData data);
        typedef void (*ImageCallback)(MarkdownImageCallbackData data);
    
        LinkCallback         linkCallback = nullptr;
        TooltipCallback      tooltipCallback = nullptr;
        ImageCallback        imageCallback = nullptr;
        const char*          linkIcon = "🔗";
        MarkdownHeadingFormat headingFormats[3];
    };

    // Function declarations
    void Markdown(const char* markdown_, size_t markdownLength_, const MarkdownConfig& mdConfig_);
    void Markdown(const std::string& markdown_, const MarkdownConfig& mdConfig_);
    void UnderLine(ImColor col_);
}
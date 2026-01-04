#include "ui/markdown_cell.hpp"
#include "imgui.h"
#include "imgui_markdown.h"

void drawMarkdownCell(std::string& content) {
    static char buffer[4096];
    strncpy(buffer, content.c_str(), sizeof(buffer));
    
    if (ImGui::InputTextMultiline("##MarkdownEditor", buffer, sizeof(buffer), ImVec2(-1, 120))) {
        content = buffer; 
    }

    ImGui::MarkdownConfig mdConfig;
    mdConfig.linkCallback = nullptr;
    mdConfig.tooltipCallback = nullptr;
    mdConfig.imageCallback = nullptr;
    mdConfig.linkIcon = "";
    mdConfig.headingFormats[0] = { ImGui::GetFont(), true };
    mdConfig.headingFormats[1] = { ImGui::GetFont(), true };
    mdConfig.headingFormats[2] = { ImGui::GetFont(), false };
    ImGui::Markdown(content.c_str(), content.length(), mdConfig);
}
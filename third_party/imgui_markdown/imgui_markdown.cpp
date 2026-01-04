#include "imgui_markdown.h"
#include <vector>
#include <string>
#include <algorithm>

void ImGui::Markdown(const char* markdown_, size_t markdownLength_, const MarkdownConfig& mdConfig_)
{
    // Basic markdown rendering implementation
    ImGui::TextUnformatted(markdown_, markdown_ + markdownLength_);
}

void ImGui::Markdown(const std::string& markdown_, const MarkdownConfig& mdConfig_)
{
    Markdown(markdown_.c_str(), markdown_.length(), mdConfig_);
}

void ImGui::UnderLine(ImColor col_)
{
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    min.y = max.y;
    ImGui::GetWindowDrawList()->AddLine(min, max, col_, 1.0f);
}

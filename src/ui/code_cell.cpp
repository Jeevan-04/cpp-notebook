#include "ui/code_cell.hpp"  // Changed from "src/ui/code_cell.hpp"
#include "imgui.h"

void drawCodeCell(TextEditor& editor, std::string& output) {
    ImGui::BeginChild("CodeEditor", ImVec2(0, 300), true);
    editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
    editor.SetShowWhitespaces(false);
    editor.Render("##CodeEditor");
    ImGui::EndChild();

    if (!output.empty()) {
        ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Output:");
        ImGui::BeginChild("Output", ImVec2(0, 100), true);
        ImGui::TextWrapped("%s", output.c_str());
        ImGui::EndChild();
    }
}

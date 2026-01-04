#include "ui/notebook.hpp"
#include "imgui.h"
#include <SDL.h>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <chrono>
#include <regex>
#include <filesystem>
#include <iostream>
#include <algorithm>

namespace NotebookUtils {

    // ==================== ExecutionContext Implementation ====================
    void ExecutionContext::reset() {
        globalCode.clear();
        includedHeaders.clear();
        functions.clear();
        classes.clear();
        variables.clear();
        mainCodeAccumulator.clear();
    }
    
    void ExecutionContext::addHeader(const std::string& header) {
        if (includedHeaders.find(header) == includedHeaders.end()) {
            includedHeaders.insert(header);
        }
    }
    
    void ExecutionContext::addFunction(const std::string& func) {
        functions.push_back(func);
    }
    
    void ExecutionContext::addClass(const std::string& cls) {
        classes.push_back(cls);
    }
    
    void ExecutionContext::addVariable(const std::string& var) {
        // Extract variable name to check if it already exists
        std::string trimmed = var;
        if (!trimmed.empty() && trimmed.back() == ';') trimmed.pop_back();
        
        std::istringstream iss(trimmed);
        std::string type, name;
        iss >> type >> name;
        
        // Remove everything after = or [ to get just the variable name
        size_t eqPos = name.find('=');
        if (eqPos != std::string::npos) {
            name = name.substr(0, eqPos);
        }
        size_t bracketPos = name.find('[');
        if (bracketPos != std::string::npos) {
            name = name.substr(0, bracketPos);
        }
        
        // Check if variable already exists and update it
        for (auto& existing : variables) {
            std::string existingTrimmed = existing;
            if (!existingTrimmed.empty() && existingTrimmed.back() == ';') existingTrimmed.pop_back();
            
            std::istringstream existingIss(existingTrimmed);
            std::string existingType, existingName;
            existingIss >> existingType >> existingName;
            
            size_t existingEqPos = existingName.find('=');
            if (existingEqPos != std::string::npos) {
                existingName = existingName.substr(0, existingEqPos);
            }
            size_t existingBracketPos = existingName.find('[');
            if (existingBracketPos != std::string::npos) {
                existingName = existingName.substr(0, existingBracketPos);
            }
            
            // If same variable name, update it
            if (existingName == name) {
                existing = var;
                return;
            }
        }
        
        // New variable, add it
        variables.push_back(var);
    }

    // ==================== CodeParser Implementation ====================
    CodeParser::ParseResult CodeParser::parse(const std::string& code, ExecutionContext& context) {
        std::istringstream stream(code);
        std::string line;
        ParseResult result;
        bool inBlockComment = false;
        bool inMainFunction = false;
        
        while (std::getline(stream, line)) {
            // Process comments and block comments
            if (inBlockComment) {
                size_t endPos = line.find("*/");
                if (endPos != std::string::npos) {
                    line = line.substr(endPos + 2);
                    inBlockComment = false;
                } else {
                    continue;
                }
            }
            
            size_t commentPos = line.find("//");
            if (commentPos != std::string::npos) {
                line = line.substr(0, commentPos);
            }
            
            size_t blockStart = line.find("/*");
            if (blockStart != std::string::npos) {
                size_t blockEnd = line.find("*/", blockStart + 2);
                if (blockEnd != std::string::npos) {
                    line = line.substr(0, blockStart) + line.substr(blockEnd + 2);
                } else {
                    line = line.substr(0, blockStart);
                    inBlockComment = true;
                    continue;
                }
            }
            
            // Trim whitespace
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
                return !std::isspace(ch);
            }));
            line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), line.end());
            
            if (line.empty()) continue;

            if (line.find("#include") == 0) {
                context.addHeader(line);
                result.headers += line + "\n";
            }
            else if (!inMainFunction && line.find("int main(") != std::string::npos) {
                result.hasMain = true;
                inMainFunction = true;
                result.mainCode += line + "\n";
            }
            else if (inMainFunction) {
                result.mainCode += line + "\n";
                if (line.find("return") != std::string::npos) {
                    result.hasReturn = true;
                }
                if (line.find("}") != std::string::npos) {
                    inMainFunction = false;
                }
            }
            else if (line.find("class ") != std::string::npos || 
                     line.find("struct ") != std::string::npos ||
                     line.find("template ") != std::string::npos ||
                     (line.find('(') != std::string::npos && line.find(')') != std::string::npos && line.find(';') == std::string::npos)) {
                context.globalCode += line + "\n";
                result.globalCode += line + "\n";
            }
            else if (line.find(';') != std::string::npos && 
                    line.find('=') == std::string::npos && 
                    line.find('(') == std::string::npos) {
                context.globalCode += line + "\n";
                result.globalCode += line + "\n";
            }
            else {
                result.mainCode += line + "\n";
            }
        }
        
        return result;
    }

    // ==================== ExecutionEngine Implementation ====================
    ExecutionEngine::ExecutionEngine() {
        reset();
    }
    
    void ExecutionEngine::reset() {
        context.reset();
    }
    
    std::string ExecutionEngine::execute(const std::string& code) {
        // Build complete program from all accumulated context
        std::string fullProgram;
        
        // 1. Headers
        fullProgram = "#include <iostream>\n";
        fullProgram += "#include <vector>\n";
        fullProgram += "#include <string>\n";
        fullProgram += "#include <map>\n";
        fullProgram += "#include <algorithm>\n";
        fullProgram += "#include <memory>\n";
        fullProgram += "#include <cmath>\n";
        fullProgram += "using namespace std;\n\n";
        
        // 2. Add all global variables ONCE (outside main)
        for (const auto& var : context.variables) {
            fullProgram += var + "\n";
        }
        fullProgram += "\n";
        
        // 3. Add all classes defined in previous cells
        for (const auto& cls : context.classes) {
            fullProgram += cls + "\n\n";
        }
        
        // 4. Add all functions defined in previous cells
        for (const auto& func : context.functions) {
            fullProgram += func + "\n\n";
        }
        
        // 5. Parse current cell code line-by-line to separate variables, functions, and regular code
        std::vector<std::string> variableDecls;
        std::vector<std::string> functionDefs;
        std::string regularCode;
        
        std::istringstream codeStream(code);
        std::string line;
        std::string currentFunction;
        bool inFunction = false;
        int braceDepth = 0;
        
        while (std::getline(codeStream, line)) {
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            
            if (trimmed.empty()) continue;
            
            // If we're building a function, accumulate lines
            if (inFunction) {
                currentFunction += line + "\n";
                
                // Count braces to know when function ends
                for (char c : line) {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }
                
                // Function complete when braces balance
                if (braceDepth == 0) {
                    functionDefs.push_back(currentFunction);
                    currentFunction.clear();
                    inFunction = false;
                }
                continue;
            }
            
            // Check for function definition start (return type + name + parentheses)
            bool isFunctionStart = false;
            if (trimmed.find("(") != std::string::npos && trimmed.find(")") != std::string::npos) {
                std::string firstWord = trimmed.substr(0, trimmed.find_first_of(" \t("));
                if (firstWord == "void" || firstWord == "int" || firstWord == "double" || 
                    firstWord == "float" || firstWord == "string" || firstWord == "bool" ||
                    firstWord == "char" || firstWord == "auto") {
                    isFunctionStart = true;
                }
            }
            
            if (isFunctionStart) {
                inFunction = true;
                currentFunction = line + "\n";
                braceDepth = 0;
                
                // Count braces in this line
                for (char c : line) {
                    if (c == '{') braceDepth++;
                    if (c == '}') braceDepth--;
                }
                
                // Check if one-liner function
                if (braceDepth == 0 && line.find("}") != std::string::npos) {
                    functionDefs.push_back(currentFunction);
                    currentFunction.clear();
                    inFunction = false;
                }
                continue;
            }
            
            // Check for variable declaration (type + name + semicolon, no parentheses)
            bool isVarDecl = false;
            if (trimmed.find(";") != std::string::npos && trimmed.find("(") == std::string::npos) {
                std::string firstWord = trimmed.substr(0, trimmed.find_first_of(" \t"));
                if (firstWord == "int" || firstWord == "double" || firstWord == "float" || 
                    firstWord == "string" || firstWord == "bool" || firstWord == "char" ||
                    firstWord == "auto" || firstWord.find("vector<") == 0) {
                    isVarDecl = true;
                }
            }
            
            if (isVarDecl) {
                variableDecls.push_back(trimmed);
                continue;
            }
            
            // Check if this is a variable reassignment (e.g., "a = 24;")
            bool isReassignment = false;
            if (trimmed.find('=') != std::string::npos && trimmed.back() == ';') {
                size_t eqPos = trimmed.find('=');
                std::string varName = trimmed.substr(0, eqPos);
                // Trim whitespace from varName
                varName.erase(0, varName.find_first_not_of(" \t"));
                varName.erase(varName.find_last_not_of(" \t") + 1);
                
                // Check if this variable exists in context
                for (const auto& existingVar : context.variables) {
                    // Extract variable name from declaration (e.g., "int a = 5;" -> "a")
                    size_t spacePos = existingVar.find(' ');
                    if (spacePos != std::string::npos) {
                        size_t nameStart = spacePos + 1;
                        size_t nameEnd = existingVar.find_first_of(" =;", nameStart);
                        std::string existingVarName = existingVar.substr(nameStart, nameEnd - nameStart);
                        
                        if (existingVarName == varName) {
                            // This is a reassignment! Extract type and create new declaration
                            std::string varType = existingVar.substr(0, spacePos);
                            std::string newDecl = varType + " " + trimmed;
                            // Update context directly (don't add to variableDecls to avoid duplication)
                            context.addVariable(newDecl);
                            isReassignment = true;
                            break;
                        }
                    }
                }
            }
            
            // Otherwise, it's regular code (function calls, statements, etc.)
            if (!isReassignment) {
                regularCode += line + "\n";
            }
        }
        
        // Add parsed variables to context
        for (const auto& var : variableDecls) {
            context.addVariable(var);
        }
        
        // Add parsed functions to context
        for (const auto& func : functionDefs) {
            context.addFunction(func);
        }
        
        // Add NEW variables from this cell to fullProgram (must be before functions!)
        for (const auto& var : variableDecls) {
            fullProgram += var + "\n";
        }
        if (!variableDecls.empty()) {
            fullProgram += "\n";
        }
        
        // Add NEW functions from this cell to fullProgram (after variables, before main)
        for (const auto& func : functionDefs) {
            fullProgram += func + "\n\n";
        }
        
        // Determine what to execute
        bool hasMainInCurrentCell = code.find("int main") != std::string::npos;
        
        if (hasMainInCurrentCell) {
            // Complete program with main
            fullProgram += code;
        } else if (!regularCode.empty()) {
            // Execute regular code in main
            fullProgram += "int main() {\n";
            fullProgram += regularCode;
            fullProgram += "    return 0;\n}\n";
        } else {
            // Just compile to check syntax (variables/functions only)
            fullProgram += "int main() { return 0; }\n";
        }
        
        // Generate temporary files
        std::string cppFile = generateTempFile(fullProgram);
        std::string outFile = cppFile + ".out";
        std::string errFile = cppFile + ".err";
        
        // Compile the code
        std::string compileCmd = "g++ -std=c++20 -Wall -Wextra " + cppFile + " -o " + outFile + " 2> " + errFile;
        int compileResult = std::system(compileCmd.c_str());
        
        if (compileResult != 0) {
            std::string errors = readFile(errFile);
            cleanupFiles(cppFile, outFile, errFile);
            return "[X] Compilation Error:\n" + errors;
        }
        
        // Execute the program
        auto [output, duration] = executeProgram(outFile, errFile);
        cleanupFiles(cppFile, outFile, errFile);
        
        if (output.empty()) {
            return "[OK] Success (" + std::to_string(duration) + "ms)";
        }
        return output;
    }
    
    std::string ExecutionEngine::generateTempFile(const std::string& content) {
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        std::string filename = "notebook_temp_" + std::to_string(timestamp) + ".cpp";
        std::ofstream file(filename);
        file << content;
        file.close();
        return filename;
    }
    
    std::string ExecutionEngine::readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) return "";
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
    
    void ExecutionEngine::cleanupFiles(const std::string& cppFile, const std::string& outFile, const std::string& errFile) {
        std::remove(cppFile.c_str());
        std::remove(outFile.c_str());
        std::remove(errFile.c_str());
    }
    
    std::pair<std::string, long> ExecutionEngine::executeProgram(const std::string& outFile, const std::string& errFile) {
        std::string output = readFile(errFile);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::string execCmd = "./" + outFile + " 2>&1";
        FILE* pipe = popen(execCmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                output += buffer;
            }
            pclose(pipe);
        }
        auto end = std::chrono::high_resolution_clock::now();
        
        long duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        return {output, duration};
    }

    // ==================== UI Implementation ====================
    void drawNotebook(std::vector<Cell>& cells, bool& darkMode, std::string& selectedFont) {
        static ExecutionEngine engine;
        static bool firstRun = true;
        static ImGui::MarkdownConfig mdConfig;
        static int focusNewCell = -1;
        static std::vector<std::string> mdBuffers;
        static int globalExecutionCount = 0;
        static bool showSidebar = true; // Sidebar toggle
        static std::string highlightedSymbol = "";
        static bool showVisualizer = false;
        
        if (firstRun) {
            cells.push_back({CODE, "", TextEditor(), "", 0.0f, true, 0});
            firstRun = false;
            
            mdConfig.linkCallback = nullptr;
            mdConfig.tooltipCallback = nullptr;
            mdConfig.imageCallback = nullptr;
            mdConfig.linkIcon = "";
            mdConfig.headingFormats[0] = { ImGui::GetFont(), true };
            mdConfig.headingFormats[1] = { ImGui::GetFont(), true };
            mdConfig.headingFormats[2] = { ImGui::GetFont(), false };
        }

        // Ensure markdown buffers match cell count
        while (mdBuffers.size() < cells.size()) {
            mdBuffers.push_back(std::string(8192, '\0'));
        }
        while (mdBuffers.size() > cells.size()) {
            mdBuffers.pop_back();
        }

        // Premium muted color scheme (inspired by Obsidian/VS Code/Warp)
        ImVec4 accentColor(0.45f, 0.62f, 0.82f, 1.0f);  // Softer blue
        ImVec4 successColor(0.52f, 0.75f, 0.54f, 1.0f); // Muted green
        ImVec4 errorColor(0.95f, 0.48f, 0.48f, 1.0f);   // Softer red
        ImVec4 bgDark(0.09f, 0.09f, 0.11f, 1.0f);       // Slightly blue-tinted dark
        ImVec4 cellBg(0.13f, 0.13f, 0.15f, 1.0f);       // Elevated cell background
        ImVec4 borderColor(0.22f, 0.22f, 0.25f, 1.0f);  // Subtle border
        ImVec4 shadowColor(0.0f, 0.0f, 0.0f, 0.3f);     // Shadow for elevation

        // Menu Bar
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Notebook", "Ctrl+N")) {
                    cells.clear();
                    mdBuffers.clear();
                    cells.push_back({CODE, "", TextEditor(), "", 0.0f, true, 0});
                    mdBuffers.push_back(std::string(8192, '\0'));
                    engine.reset();
                    globalExecutionCount = 0;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Clear All Outputs", "Ctrl+Shift+O")) {
                    for (auto& cell : cells) {
                        cell.output.clear();
                        cell.executionCount = 0;
                    }
                }
                if (ImGui::MenuItem("Reset Kernel", "Ctrl+Shift+R")) {
                    engine.reset();
                    globalExecutionCount = 0;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Insert")) {
                if (ImGui::MenuItem("Code Cell", "Ctrl+Shift+C")) {
                    cells.push_back({CODE, "", TextEditor(), "", 0.0f, true, 0});
                    mdBuffers.push_back(std::string(8192, '\0'));
                    focusNewCell = cells.size() - 1;
                }
                if (ImGui::MenuItem("Markdown Cell", "Ctrl+Shift+M")) {
                    cells.push_back({MARKDOWN, "# New Cell\n\nStart typing...", TextEditor(), "", 0.0f, true, 0});
                    mdBuffers.push_back(std::string(8192, '\0'));
                    focusNewCell = cells.size() - 1;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                ImGui::MenuItem("Show Sidebar", "Ctrl+B", &showSidebar);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                ImGui::TextDisabled("Keyboard Shortcuts:");
                ImGui::Separator();
                ImGui::Text("Cmd+C / Cmd+V - Copy/Paste");
                ImGui::Text("Cmd+Z / Cmd+Shift+Z - Undo/Redo");
                ImGui::Text("Cmd+A - Select All");
                ImGui::Text("Cmd+F - Find (searches code)");
                ImGui::Text("Shift+Enter - Run Cell");
                ImGui::Separator();
                ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.3f, 1.0f), "IMPORTANT:");
                ImGui::TextWrapped("Click INSIDE the code editor first,");
                ImGui::TextWrapped("then use keyboard shortcuts!");
                ImGui::Separator();
                ImGui::TextDisabled("Sidebar Features:");
                ImGui::Text("- Click variable/function to highlight");
                ImGui::Text("- View code visualizer");
                ImGui::Text("- Track execution order");
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        // Main Window
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
        ImGui::SetNextWindowPos({0, 20}, ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, bgDark);
        ImGui::Begin("##NotebookMain", nullptr, 
                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);

        // Modern Toolbar
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12, 6));
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.45f, 0.85f, 1.0f));
        
        if (ImGui::Button("+ Code")) {
            cells.push_back({CODE, "", TextEditor(), "", 0.0f, true, 0});
            mdBuffers.push_back(std::string(8192, '\0'));
            focusNewCell = cells.size() - 1;
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Markdown")) {
            cells.push_back({MARKDOWN, "# New Cell\n\nStart typing...", TextEditor(), "", 0.0f, true, 0});
            mdBuffers.push_back(std::string(8192, '\0'));
            focusNewCell = cells.size() - 1;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.6f, 0.6f, 0.6f, 0.8f));
        if (ImGui::Button("Clear Outputs")) {
            for (auto& cell : cells) cell.output.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart Kernel")) {
            engine.reset();
            globalExecutionCount = 0;
        }
        
        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();
        
        // Run All button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.5f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button("Run All Cells")) {
            for (size_t idx = 0; idx < cells.size(); ++idx) {
                if (cells[idx].type == CODE) {
                    auto start = std::chrono::high_resolution_clock::now();
                    try {
                        std::string code = cells[idx].editor.GetText();
                        if (!code.empty()) {
                            cells[idx].output = engine.execute(code);
                            cells[idx].executionCount = ++globalExecutionCount;
                        } else {
                            cells[idx].output = "[!] Empty cell";
                        }
                    } catch (const std::exception& e) {
                        cells[idx].output = std::string("[X] Exception: ") + e.what();
                    }
                    auto end = std::chrono::high_resolution_clock::now();
                    cells[idx].executionTime = std::chrono::duration<float, std::milli>(end - start).count();
                    cells[idx].outputVisible = true;
                }
            }
        }
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);
        
        ImGui::Separator();
        ImGui::Spacing();

        // Layout: Sidebar + Notebook
        if (showSidebar) {
            ImGui::BeginChild("Sidebar", ImVec2(280, 0), true);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.9f, 1.0f, 1.0f));
            ImGui::Text("OUTLINE & INSPECTOR");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Collect sections (markdown headings)
            if (ImGui::CollapsingHeader("📄 Sections", ImGuiTreeNodeFlags_DefaultOpen)) {
                bool foundSection = false;
                for (size_t i = 0; i < cells.size(); ++i) {
                    if (cells[i].type == MARKDOWN) {
                        std::string content = cells[i].content;
                        std::istringstream stream(content);
                        std::string line;
                        while (std::getline(stream, line)) {
                            if (line.size() > 0 && line[0] == '#') {
                                int level = 0;
                                while (level < line.size() && line[level] == '#') level++;
                                std::string heading = line.substr(level);
                                // Trim leading spaces
                                size_t start = heading.find_first_not_of(" \t");
                                if (start != std::string::npos) {
                                    heading = heading.substr(start);
                                }
                                
                                // Indent based on level
                                for (int l = 1; l < level; l++) ImGui::Indent(10.0f);
                                
                                ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "%s", heading.c_str());
                                
                                for (int l = 1; l < level; l++) ImGui::Unindent(10.0f);
                                
                                foundSection = true;
                            }
                        }
                    }
                }
                if (!foundSection) {
                    ImGui::TextDisabled("  (no headings)");
                }
            }
            
            ImGui::Spacing();
            
            // Functions
            if (ImGui::CollapsingHeader("⚡ Functions", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (engine.getContext().functions.empty()) {
                    ImGui::TextDisabled("  (none defined)");
                } else {
                    for (const auto& func : engine.getContext().functions) {
                        // Extract function signature (first line)
                        size_t bracePos = func.find('{');
                        std::string sig = (bracePos != std::string::npos) ? func.substr(0, bracePos) : func;
                        // Trim
                        size_t start = sig.find_first_not_of(" \t\n");
                        size_t end = sig.find_last_not_of(" \t\n");
                        if (start != std::string::npos && end != std::string::npos) {
                            sig = sig.substr(start, end - start + 1);
                        }
                        
                        // Extract function name
                        size_t parenPos = sig.find('(');
                        std::string funcName = sig;
                        if (parenPos != std::string::npos) {
                            std::string beforeParen = sig.substr(0, parenPos);
                            size_t lastSpace = beforeParen.find_last_of(" \t");
                            if (lastSpace != std::string::npos) {
                                funcName = beforeParen.substr(lastSpace + 1);
                            }
                        }
                        
                        // Make it clickable
                        bool isHighlighted = (highlightedSymbol == funcName);
                        if (isHighlighted) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
                        
                        if (ImGui::Selectable((std::string("⚡ ") + sig).c_str(), isHighlighted)) {
                            highlightedSymbol = funcName;
                        }
                        
                        if (isHighlighted) ImGui::PopStyleColor();
                    }
                }
            }
            
            ImGui::Spacing();
            
            // Variables
            if (ImGui::CollapsingHeader("🔢 Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (engine.getContext().variables.empty()) {
                    ImGui::TextDisabled("  (none defined)");
                } else {
                    for (const auto& var : engine.getContext().variables) {
                        std::string trimmed = var;
                        // Remove trailing semicolon
                        if (!trimmed.empty() && trimmed.back() == ';') trimmed.pop_back();
                        // Trim
                        size_t start = trimmed.find_first_not_of(" \t\n");
                        size_t end = trimmed.find_last_not_of(" \t\n");
                        if (start != std::string::npos && end != std::string::npos) {
                            trimmed = trimmed.substr(start, end - start + 1);
                        }
                        
                        // Extract variable name
                        std::istringstream iss(trimmed);
                        std::string type, varName;
                        iss >> type >> varName;
                        size_t eqPos = varName.find('=');
                        if (eqPos != std::string::npos) {
                            varName = varName.substr(0, eqPos);
                        }
                        
                        // Make it clickable
                        bool isHighlighted = (highlightedSymbol == varName);
                        if (isHighlighted) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
                        
                        if (ImGui::Selectable((std::string("🔢 ") + trimmed).c_str(), isHighlighted)) {
                            highlightedSymbol = varName;
                        }
                        
                        if (isHighlighted) ImGui::PopStyleColor();
                    }
                }
            }
            
            ImGui::Spacing();
            
            // Classes
            if (ImGui::CollapsingHeader("🏛️ Classes", ImGuiTreeNodeFlags_DefaultOpen)) {
                if (engine.getContext().classes.empty()) {
                    ImGui::TextDisabled("  (none defined)");
                } else {
                    for (const auto& cls : engine.getContext().classes) {
                        // Extract class name
                        size_t classPos = cls.find("class ");
                        size_t structPos = cls.find("struct ");
                        size_t nameStart = (classPos != std::string::npos) ? classPos + 6 : 
                                          (structPos != std::string::npos) ? structPos + 7 : 0;
                        std::string name = cls.substr(nameStart);
                        size_t bracePos = name.find('{');
                        if (bracePos != std::string::npos) {
                            name = name.substr(0, bracePos);
                        }
                        // Trim
                        size_t start = name.find_first_not_of(" \t\n");
                        size_t end = name.find_last_not_of(" \t\n");
                        if (start != std::string::npos && end != std::string::npos) {
                            name = name.substr(start, end - start + 1);
                        }
                        ImGui::TextColored(ImVec4(0.9f, 0.5f, 0.9f, 1.0f), "🏛️  %s", name.c_str());
                    }
                }
            }
            
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Code Visualizer
            if (ImGui::CollapsingHeader("𓁹 Code Visualizer", ImGuiTreeNodeFlags_None))
            {
                ImGui::TextDisabled("Visual execution flow:");
                ImGui::Spacing();
                
                if (ImGui::Button("Start Visualization")) {
                    showVisualizer = !showVisualizer;
                }
                
                if (showVisualizer) {
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.5f, 0.9f, 0.5f, 1.0f), "Execution Order:");
                    
                    // Show execution order of cells
                    for (size_t i = 0; i < cells.size(); ++i) {
                        if (cells[i].type == CODE && cells[i].executionCount > 0) {
                            ImGui::Text("  [%d] Cell %zu", cells[i].executionCount, i + 1);
                        }
                    }
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.9f, 0.7f, 0.4f, 1.0f), "Call Stack:");
                    
                    // Show functions in order
                    for (const auto& func : engine.getContext().functions) {
                        size_t nameStart = func.find_first_of(" \t") + 1;
                        size_t nameEnd = func.find('(');
                        if (nameEnd != std::string::npos && nameStart < nameEnd) {
                            std::string name = func.substr(nameStart, nameEnd - nameStart);
                            size_t lastSpace = name.find_last_of(" \t");
                            if (lastSpace != std::string::npos) {
                                name = name.substr(lastSpace + 1);
                            }
                            ImGui::Text("  -> %s()", name.c_str());
                        }
                    }
                    
                    ImGui::Spacing();
                    ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), "Memory State:");
                    ImGui::Text("  Variables: %zu", engine.getContext().variables.size());
                    ImGui::Text("  Functions: %zu", engine.getContext().functions.size());
                    ImGui::Text("  Classes: %zu", engine.getContext().classes.size());
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            // Variable Inspector Panel
            if (ImGui::CollapsingHeader("📊 Variable Inspector", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4)); // Compact spacing
                
                if (engine.getContext().variables.empty()) {
                    ImGui::TextDisabled("(none)");
                } else {
                    // Show each variable with compact, clean layout
                    for (const auto& var : engine.getContext().variables) {
                        std::string trimmed = var;
                        if (!trimmed.empty() && trimmed.back() == ';') trimmed.pop_back();
                        
                        // Parse type and name
                        std::istringstream iss(trimmed);
                        std::string type, name, rest;
                        iss >> type >> name;
                        std::getline(iss, rest);
                        
                        // Remove = and value part for display
                        size_t eqPos = name.find('=');
                        if (eqPos != std::string::npos) {
                            name = name.substr(0, eqPos);
                        }
                        
                        // Compact single-line display: icon name:type value
                        ImGui::TextColored(ImVec4(0.45f, 0.62f, 0.82f, 1.0f), "◆");
                        ImGui::SameLine(0, 4);
                        ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.95f, 1.0f), "%s", name.c_str());
                        ImGui::SameLine(0, 4);
                        ImGui::TextDisabled(":");
                        ImGui::SameLine(0, 4);
                        ImGui::TextColored(ImVec4(0.52f, 0.75f, 0.54f, 1.0f), "%s", type.c_str());
                        
                        if (!rest.empty()) {
                            ImGui::SameLine(0, 4);
                            ImGui::TextDisabled("%s", rest.c_str());
                        }
                    }
                }
                ImGui::PopStyleVar(); // End compact spacing
            }
            
            ImGui::EndChild();
            ImGui::SameLine();
        }

        // Scrollable notebook area
        ImGui::BeginChild("NotebookScroll", ImVec2(0, 0), false);
        
        // Show keyboard shortcut hint if a symbol is highlighted
        if (!highlightedSymbol.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.3f, 1.0f));
            ImGui::Text("Highlighting: '%s' - Use Cmd+F to find in cells", highlightedSymbol.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear##highlight")) {
                highlightedSymbol = "";
            }
            ImGui::Separator();
            ImGui::Spacing();
        }

        // Render cells
        for (size_t i = 0; i < cells.size(); ++i) {
            ImGui::PushID(static_cast<int>(i));
            
            // Cell wrapper
            ImGui::BeginGroup();
            
            // Left side - Cell number indicator
            ImGui::BeginGroup();
            float cellNumWidth = 60.0f;
            
            if (cells[i].type == CODE) {
                std::string cellLabel = cells[i].executionCount > 0 ? 
                    "[" + std::to_string(cells[i].executionCount) + "]" : "[ ]";
                ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
                ImGui::Text("%s", cellLabel.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::TextDisabled("[ ]");
            }
            ImGui::EndGroup();
            
            ImGui::SameLine();
            
            // Right: Cell content (auto-width)
            ImGui::BeginGroup();
            float contentWidth = ImGui::GetContentRegionAvail().x - 80; // Reserve space for right margin
            
            if (cells[i].type == CODE) {
                // === CODE CELL with elevation ===
                
                // Get draw list and split channels BEFORE rendering content
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                ImVec2 cellContentStart = ImGui::GetCursorScreenPos();
                
                // Split channels: 0 = background, 1 = foreground (content)
                drawList->ChannelsSplit(2);
                drawList->ChannelsSetCurrent(1); // Draw content in foreground
                
                // Add left padding
                ImGui::Indent(12.0f);
                
                // Action buttons row
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                
                float buttonY = ImGui::GetCursorPosY();
                
                // Run button
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.65f, 0.25f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.75f, 0.3f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.55f, 0.2f, 1.0f));
                if (ImGui::Button(("[RUN]##" + std::to_string(i)).c_str(), ImVec2(60, 0))) {
                    // Reset execution engine and re-run all cells from 0 to current
                    engine.reset();
                    globalExecutionCount = 0;
                    
                    // Clear outputs for cells after this one
                    for (size_t j = i + 1; j < cells.size(); ++j) {
                        cells[j].output.clear();
                        cells[j].executionCount = 0;
                    }
                    
                    // Re-execute all cells from 0 to current cell
                    for (size_t j = 0; j <= i; ++j) {
                        if (cells[j].type == CODE) {
                            auto start = std::chrono::high_resolution_clock::now();
                            try {
                                std::string code = cells[j].editor.GetText();
                                if (!code.empty()) {
                                    cells[j].output = engine.execute(code);
                                    cells[j].executionCount = ++globalExecutionCount;
                                } else {
                                    cells[j].output = "[!] Empty cell";
                                }
                            } catch (const std::exception& e) {
                                cells[j].output = std::string("[X] Exception: ") + e.what();
                            }
                            auto end = std::chrono::high_resolution_clock::now();
                            cells[j].executionTime = std::chrono::duration<float, std::milli>(end - start).count();
                            cells[j].outputVisible = true;
                        }
                    }
                }
                ImGui::PopStyleColor(3);
                
                ImGui::SameLine();
                if (ImGui::Button(("Copy##" + std::to_string(i)).c_str(), ImVec2(55, 0))) {
                    ImGui::SetClipboardText(cells[i].editor.GetText().c_str());
                }
                
                ImGui::SameLine();
                if (ImGui::Button(("Paste##" + std::to_string(i)).c_str(), ImVec2(55, 0))) {
                    const char* clipText = ImGui::GetClipboardText();
                    if (clipText) {
                        cells[i].editor.SetText(clipText);
                    }
                }
                
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Button(("Delete##" + std::to_string(i)).c_str(), ImVec2(60, 0)) && cells.size() > 1) {
                    cells.erase(cells.begin() + i);
                    mdBuffers.erase(mdBuffers.begin() + i);
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(2);
                    ImGui::Unindent(12.0f);
                    ImGui::EndGroup();
                    ImGui::EndGroup();
                    ImGui::PopID();
                    continue;
                }
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
                
                ImGui::Spacing();
                
                // Code editor - auto height based on content with larger font
                cells[i].editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
                cells[i].editor.SetShowWhitespaces(false);
                cells[i].editor.SetTabSize(4);
                
                int lineCount = cells[i].editor.GetTotalLines();
                float editorHeight = lineCount * ImGui::GetTextLineHeightWithSpacing() * 1.5f + 15;
                
                // Use much larger font for code (22pt)
                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // 22pt code font
                cells[i].editor.Render(("##CodeEdit" + std::to_string(i)).c_str(), 
                                      ImVec2(0, editorHeight), false);
                ImGui::PopFont();
                
                // Check for Shift+Enter to run cell
                if (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter) && 
                    (ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift))) {
                    // Reset execution engine and re-run all cells from 0 to current
                    engine.reset();
                    globalExecutionCount = 0;
                    
                    // Clear outputs for cells after this one
                    for (size_t j = i + 1; j < cells.size(); ++j) {
                        cells[j].output.clear();
                        cells[j].executionCount = 0;
                    }
                    
                    // Re-execute all cells from 0 to current cell
                    for (size_t j = 0; j <= i; ++j) {
                        if (cells[j].type == CODE) {
                            auto start = std::chrono::high_resolution_clock::now();
                            try {
                                std::string code = cells[j].editor.GetText();
                                if (!code.empty()) {
                                    cells[j].output = engine.execute(code);
                                    cells[j].executionCount = ++globalExecutionCount;
                                } else {
                                    cells[j].output = "[!] Empty cell";
                                }
                            } catch (const std::exception& e) {
                                cells[j].output = std::string("[X] Exception: ") + e.what();
                            }
                            auto end = std::chrono::high_resolution_clock::now();
                            cells[j].executionTime = std::chrono::duration<float, std::milli>(end - start).count();
                            cells[j].outputVisible = true;
                        }
                    }
                }
                
                // Output section
                if (!cells[i].output.empty()) {
                    ImGui::Spacing();
                    
                    bool isError = cells[i].output.find("[X]") != std::string::npos;
                    bool isWarning = cells[i].output.find("[!]") != std::string::npos;
                    
                    ImVec4 outputColor = isError ? errorColor : isWarning ? ImVec4(1.0f, 0.7f, 0.2f, 1.0f) : successColor;
                    
                    // Toggle output visibility
                    ImGui::PushStyleColor(ImGuiCol_Text, outputColor);
                    std::string toggleLabel = cells[i].outputVisible ? "v " : "> ";
                    toggleLabel += isError ? "Error" : isWarning ? "Warning" : "Output";
                    if (cells[i].executionTime > 0) {
                        toggleLabel += " (" + std::to_string((int)cells[i].executionTime) + "ms)";
                    }
                    
                    if (ImGui::Selectable((toggleLabel + "##toggle" + std::to_string(i)).c_str(), 
                                         false, 0, ImVec2(0, 20))) {
                        cells[i].outputVisible = !cells[i].outputVisible;
                    }
                    ImGui::PopStyleColor();
                    
                    if (cells[i].outputVisible) {
                        // Show output with much larger font
                        ImGui::PushStyleColor(ImGuiCol_Text, isError ? errorColor : successColor);
                        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // 22pt font for output
                        ImGui::TextWrapped("%s", cells[i].output.c_str());
                        ImGui::PopFont();
                        ImGui::PopStyleColor();
                        
                        ImGui::Spacing();
                        if (ImGui::SmallButton(("Copy Output##" + std::to_string(i)).c_str())) {
                            ImGui::SetClipboardText(cells[i].output.c_str());
                        }
                    }
                }
                
                // End cell padding
                ImGui::Unindent(12.0f);
                
                // Now draw background and shadow in channel 0 (background layer)
                ImVec2 cellContentEnd = ImGui::GetCursorScreenPos();
                
                // Calculate background rect to match full content width
                ImVec2 bgMin = ImVec2(cellContentStart.x - 4, cellContentStart.y - 8);
                ImVec2 bgMax = ImVec2(cellContentStart.x + contentWidth + 8, cellContentEnd.y + 8);
                
                // Switch to background layer
                drawList->ChannelsSetCurrent(0);
                
                // Shadow (below and slightly offset)
                ImVec2 shadowMin = ImVec2(bgMin.x + 2, bgMax.y + 1);
                ImVec2 shadowMax = ImVec2(bgMax.x + 2, bgMax.y + 3);
                drawList->AddRectFilled(shadowMin, shadowMax, ImGui::GetColorU32(shadowColor), 6.0f);
                
                // Background with rounded corners
                drawList->AddRectFilled(bgMin, bgMax, ImGui::GetColorU32(cellBg), 6.0f);
                
                // Border
                drawList->AddRect(bgMin, bgMax, ImGui::GetColorU32(borderColor), 6.0f, 0, 1.0f);
                
                // Merge channels
                drawList->ChannelsMerge();
                
                // Add vertical spacing after cell to prevent overlap
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 16);
                
            } else if (cells[i].type == MARKDOWN) {
                
                // Action buttons
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
                
                // Show both edit and preview side by side or stacked
                static bool editMode = true;
                
                // Toggle between edit and preview (on the left)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.6f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.7f, 0.9f));
                std::string toggleLabel = (editMode ? "[EDIT]##md" : "[VIEW]##md") + std::to_string(i);
                if (ImGui::Button(toggleLabel.c_str(), ImVec2(70, 0))) {
                    editMode = !editMode;
                }
                ImGui::PopStyleColor(2);
                
                ImGui::SameLine();
                if (ImGui::Button(("Copy##md" + std::to_string(i)).c_str(), ImVec2(55, 0))) {
                    ImGui::SetClipboardText(cells[i].content.c_str());
                }
                
                ImGui::SameLine();
                if (ImGui::Button(("Paste##md" + std::to_string(i)).c_str(), ImVec2(55, 0))) {
                    const char* clipText = ImGui::GetClipboardText();
                    if (clipText) {
                        cells[i].content = clipText;
                        strncpy(&mdBuffers[i][0], clipText, mdBuffers[i].size() - 1);
                    }
                }
                
                ImGui::SameLine();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 0.8f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
                if (ImGui::Button(("Delete##md" + std::to_string(i)).c_str(), ImVec2(60, 0)) && cells.size() > 1) {
                    cells.erase(cells.begin() + i);
                    mdBuffers.erase(mdBuffers.begin() + i);
                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(2);
                    ImGui::EndGroup();
                    ImGui::EndGroup();
                    ImGui::PopID();
                    continue;
                }
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
                
                ImGui::Spacing();
                
                // Live markdown rendering with editor
                if (mdBuffers[i].size() < cells[i].content.size() + 1024) {
                    mdBuffers[i].resize(cells[i].content.size() + 8192);
                }
                strncpy(&mdBuffers[i][0], cells[i].content.c_str(), mdBuffers[i].size() - 1);
                
                int lineCount = 1 + std::count(cells[i].content.begin(), cells[i].content.end(), '\n');
                float mdHeight = std::max(80.0f, std::min(500.0f, lineCount * ImGui::GetTextLineHeightWithSpacing() * 1.3f + 20));
                
                if (editMode) {
                    // Edit mode - show raw markdown with larger font
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 0.5f));
                    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); // Larger font
                    if (ImGui::InputTextMultiline(("##MD" + std::to_string(i)).c_str(), 
                                                &mdBuffers[i][0], 
                                                mdBuffers[i].size(), 
                                                ImVec2(-1, mdHeight),
                                                ImGuiInputTextFlags_AllowTabInput)) {
                        cells[i].content = mdBuffers[i].c_str();
                    }
                    ImGui::PopFont();
                    ImGui::PopStyleColor();
                } else {
                    // Preview mode - render markdown with proper styling
                    ImGui::BeginChild(("##MDPreview" + std::to_string(i)).c_str(), 
                                     ImVec2(-1, mdHeight), true, 
                                     ImGuiWindowFlags_HorizontalScrollbar);
                    
                    // Parse and render with custom heading sizes
                    std::istringstream stream(cells[i].content);
                    std::string line;
                    while (std::getline(stream, line)) {
                        if (line.empty()) {
                            ImGui::Spacing();
                            continue;
                        }
                        
                        // Check for headings
                        if (line[0] == '#') {
                            int level = 0;
                            while (level < line.size() && line[level] == '#') level++;
                            std::string text = line.substr(level);
                            // Trim
                            size_t start = text.find_first_not_of(" \t");
                            if (start != std::string::npos) {
                                text = text.substr(start);
                            }
                            
                            // Render with appropriate font size
                            if (level == 1) {
                                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]); // H1 - 24pt
                                ImGui::TextColored(ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "%s", text.c_str());
                                ImGui::PopFont();
                                ImGui::Separator();
                            } else if (level == 2) {
                                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]); // H2 - 20pt
                                ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.95f, 1.0f), "%s", text.c_str());
                                ImGui::PopFont();
                            } else if (level == 3) {
                                ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[4]); // H3 - 17pt
                                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.9f, 1.0f), "%s", text.c_str());
                                ImGui::PopFont();
                            } else {
                                ImGui::TextWrapped("%s", text.c_str());
                            }
                        } else {
                            // Regular text with larger font
                            ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
                            ImGui::TextWrapped("%s", line.c_str());
                            ImGui::PopFont();
                        }
                    }
                    
                    ImGui::EndChild();
                }
            }
            
            ImGui::EndGroup();
            ImGui::EndGroup();
            
            ImGui::PopID();
        }

        ImGui::EndChild();
        ImGui::End();
        ImGui::PopStyleColor();
    }
}
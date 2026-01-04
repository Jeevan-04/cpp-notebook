// notebook.hpp
#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include "TextEditor.h"
#include "imgui_markdown.h"

namespace NotebookUtils {
    enum CellType { CODE, MARKDOWN };

    struct Cell {
        CellType type;
        std::string content;
        TextEditor editor;
        std::string output;
        float executionTime = 0.0f;
        bool outputVisible = true;
        int executionCount = 0;
        std::vector<std::string> definedSymbols; // Functions, classes, variables defined in this cell
    };

    class ExecutionContext {
    public:
        std::string globalCode;
        std::unordered_set<std::string> includedHeaders;
        std::vector<std::string> functions;
        std::vector<std::string> classes;
        std::vector<std::string> variables;
        std::string mainCodeAccumulator;
        
        void reset();
        void addHeader(const std::string& header);
        void addFunction(const std::string& func);
        void addClass(const std::string& cls);
        void addVariable(const std::string& var);
    };

    class CodeParser {
    public:
        struct ParseResult {
            std::string headers;
            std::string globalCode;
            std::string mainCode;
            bool hasMain = false;
            bool hasReturn = false;
        };
        
        ParseResult parse(const std::string& code, ExecutionContext& context);
    };

    class ExecutionEngine {
    public:
        ExecutionEngine();
        std::string execute(const std::string& code);
        void reset();
        const ExecutionContext& getContext() const { return context; }
        
    private:
        ExecutionContext context;  // Add this member variable
        std::string generateTempFile(const std::string& content);
        std::string readFile(const std::string& path);
        void cleanupFiles(const std::string& cppFile, const std::string& outFile, const std::string& errFile);
        std::pair<std::string, long> executeProgram(const std::string& outFile, const std::string& errFile);
    };

    void drawNotebook(std::vector<Cell>& cells, bool& darkMode, std::string& selectedFont);
}
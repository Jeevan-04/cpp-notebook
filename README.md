# Interactive C++ Development Through Persistent Execution Contexts: A Notebook-Based Approach

**Jeevan**  
*January 2026*

![Version](https://img.shields.io/badge/version-0.1.0-blue)
![C++](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/platform-macOS-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

---

## Abstract

We present **cppnb**, an interactive notebook environment for C++ that fundamentally rethinks how compiled languages can be used for exploratory programming. Unlike traditional C++ development workflows that require complete program compilation, our system enables cell-based execution with persistent state—similar to interpreted languages like Python, but maintaining C++'s compile-time guarantees and performance characteristics.

The core innovation lies in our execution engine that maintains a global context across cell executions. When you define a variable in one cell, it remains accessible in all subsequent cells. When you update that variable, the change persists. This seemingly simple behavior requires solving a fundamental incompatibility: C++ is compiled, not interpreted. You can't just "remember" variables between compilations like you can with Python.

Our solution reconstructs a complete C++ program for each cell execution, incorporating all previously defined variables (with their current values), functions, and classes. This approach eliminates the traditional compile-link-run cycle while preserving type safety and enabling the use of templates, operator overloading, and other C++ features that make the language powerful.

Built on ImGui for immediate-mode rendering and SDL2 for cross-platform support, cppnb provides a modern interface with syntax highlighting, markdown documentation cells, variable inspection, and comprehensive error reporting. The system demonstrates that compiled languages can support interactive workflows without sacrificing their core strengths.

![Screenshot: Main Interface](https://github-production-user-asset-6210df.s3.amazonaws.com/142775323/531708364-754ccb44-330f-49ce-9758-9d3bac38048e.png?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAVCODYLSA53PQK4ZA%2F20260104%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20260104T083504Z&X-Amz-Expires=300&X-Amz-Signature=f78ee9318a95ae6a7a8b7db49f3fd4d1e6e851b3eca0f98d207603afbc2858b9&X-Amz-SignedHeaders=host)

---

## 1. Introduction

### 1.1 The Problem with C++ Development

C++ is powerful. You get templates, compile-time optimization, manual memory management, and performance that interpreted languages can't match. But there's a cost: the traditional edit-compile-run loop.

Want to test a function? Write it, compile the whole program, link libraries, run the executable, see if it works. Found a bug? Edit, recompile, relink, run again. This cycle is slow. It kills exploration. It makes C++ feel rigid compared to languages like Python where you can just type code and see results immediately.

Jupyter notebooks changed how people write Python. You write code in small chunks (cells), run each one independently, see results inline, and build up complex programs incrementally. Variables persist between cells. If you define `x = 5` in one cell and print it in another, it works. This is perfect for learning, prototyping, data analysis, and algorithm development.

But C++ doesn't work that way. C++ programs are compiled as complete units. There's no "state" that persists between compilations. Every run starts fresh.

### 1.2 Why This Matters

Interactive development isn't just convenient—it changes how you think. When feedback is instant, you experiment more. You try ideas you wouldn't bother with if testing required a full compile cycle. You learn faster because you can see results immediately.

Imagine teaching C++ to beginners. Instead of showing them a complete program with includes, main functions, and compilation commands, you could start with:

```cpp
int x = 42;
cout << "x is " << x << endl;
```

Just that. Two lines. Run it. See output. Now change `x`. Run again. See the change. This is how people learn Python. Why not C++?

Or imagine debugging a complex algorithm. Instead of adding print statements and recompiling, you could run it cell by cell, inspecting variables at each step, modifying values to test edge cases, all without leaving your environment.

### 1.3 Our Approach

We built cppnb to bring notebook-style interaction to C++. The key insight: you don't need to modify the C++ compiler. You just need to be smart about what you compile.

When you run a cell, our system:
1. Parses the cell code to detect variable declarations, function definitions, or regular statements
2. Updates an execution context that tracks all variables, functions, and classes defined so far
3. Generates a complete C++ program that includes everything from the context plus the current cell's code
4. Compiles this program using standard g++
5. Executes it and captures the output
6. Displays results inline with execution timing

This means each cell produces a complete, valid C++ program. But from the user's perspective, it feels like Python. Variables persist. Functions remain available. You build programs incrementally.

---

## 2. Architecture

### 2.1 The Execution Context Problem

Here's the fundamental challenge: when you compile a C++ program, you get an executable. When that executable finishes, everything is gone. Memory is deallocated. Variables disappear. To make variables "persist" across cells, we need a different model.

Think about how Python notebooks work. Python is interpreted. When you define a variable, it exists in the interpreter's memory. Run another cell, that variable is still there. The interpreter maintains global state.

C++ has no interpreter. So we fake it.

### 2.2 Context Reconstruction

Our `ExecutionContext` class maintains three maps:
- `variables`: name → declaration string (e.g., `"int x = 42"`)
- `functions`: name → full function definition
- `classes`: name → full class definition

When you write:
```cpp
int x = 42;
```

We detect this is a variable declaration. We store `"int x = 42"` in our variables map under key `"x"`. 

Later, if you write:
```cpp
x = 100;
```

We detect this is an assignment (not a declaration). We update the stored value to `"int x = 100"`. Now the context knows x equals 100.

When the next cell runs, we generate:
```cpp
#include <iostream>
#include <vector>
using namespace std;

int x = 100;  // From context

int main() {
    // Current cell code here
    return 0;
}
```

See what happened? We reconstructed x with its latest value. To the compiler, this is a fresh program. To the user, it looks like x persisted.

### 2.3 The Code Parser

Determining what a line of code means is surprisingly tricky. Consider:

```cpp
int x = 5;           // Variable declaration
x = 10;              // Assignment to existing variable  
int y = x + 5;       // Declaration using existing variable
void foo() { ... }   // Function definition
class Bar { ... };   // Class definition
cout << x;           // Statement using existing variable
```

Our parser needs to distinguish these cases. We use regex patterns and syntax analysis:

**Variable declarations** contain a type followed by an identifier and optional initialization:
```cpp
if (line matches /^\s*(int|double|string|vector<.*>|...)\s+(\w+)\s*=/)
    → It's a variable declaration
```

**Function definitions** start with a return type, name, and parameter list:
```cpp
if (line matches /^\s*(void|int|double|...)\s+(\w+)\s*\(/)
    → It's a function definition
```

**Class definitions** are marked by the `class` or `struct` keyword:
```cpp
if (line matches /^\s*(class|struct)\s+(\w+)/)
    → It's a class definition
```

Everything else is treated as a statement to be placed in `main()`.

This isn't perfect—C++ syntax is complex—but it handles common cases well enough for interactive use.

### 2.4 The Execution Engine

Once we've parsed the cell and updated the context, we need to compile and run code. Here's the pipeline:

**Step 1: Generate Source File**
```cpp
string generateProgram(const string& cellCode) {
    stringstream program;
    
    // Headers
    program << "#include <iostream>\n";
    program << "#include <vector>\n";
    program << "#include <string>\n";
    // ... more headers
    program << "using namespace std;\n\n";
    
    // Global variables from context
    for (auto& [name, decl] : context.variables) {
        program << decl << ";\n";
    }
    
    // Classes from context
    for (auto& [name, def] : context.classes) {
        program << def << "\n";
    }
    
    // Functions from context  
    for (auto& [name, def] : context.functions) {
        program << def << "\n";
    }
    
    // Main function with current cell code
    program << "int main() {\n";
    program << cellCode << "\n";
    program << "return 0;\n";
    program << "}\n";
    
    return program.str();
}
```

**Step 2: Write to Temporary File**
```cpp
string tempFile = "/tmp/notebook_temp_" + randomID() + ".cpp";
ofstream out(tempFile);
out << generatedProgram;
out.close();
```

**Step 3: Compile**
```cpp
string cmd = "g++ -std=c++20 -o " + executable + " " + tempFile + " 2>&1";
string compilerOutput = executeCommand(cmd);
```

**Step 4: Execute and Capture Output**
```cpp
if (compilationSuccessful) {
    auto start = chrono::high_resolution_clock::now();
    string output = executeCommand(executable);
    auto end = chrono::high_resolution_clock::now();
    double executionTime = duration(end - start).count();
    
    return {output, executionTime, ""};
} else {
    return {"", 0, compilerOutput};
}
```

This whole process takes milliseconds for simple code. The user clicks "Run" and sees results almost immediately.

---

## 3. User Interface

### 3.1 Why Immediate Mode GUI?

Most GUI frameworks (Qt, GTK, wxWidgets) use retained mode. You create widgets, they exist in memory, you update their state. This is stateful and complex.

ImGui uses immediate mode. Every frame, you describe what should be on screen. No widget objects. No state management. Just:

```cpp
if (ImGui::Button("Run")) {
    runCell();
}
```

That's it. The button exists only during this frame. Next frame, you call the same code, and the button appears again. If the user clicked it, `Button()` returns true.

This makes the UI code incredibly simple. Our entire notebook interface is basically:

```cpp
void drawNotebook() {
    ImGui::Begin("Notebook");
    
    for (auto& cell : cells) {
        if (cell.type == CODE) {
            drawCodeCell(cell);
        } else {
            drawMarkdownCell(cell);
        }
    }
    
    ImGui::End();
}
```

No complicated widget trees. No event handlers. Just loops and conditionals.

### 3.2 The Code Editor

For code editing, we use ImGuiColorTextEdit. It provides:
- Syntax highlighting for C++20 keywords
- Line numbers
- Cursor position tracking  
- Native copy/paste (Cmd+C/V on macOS)
- Undo/redo
- Auto-indentation

We configure it like this:

```cpp
TextEditor editor;
editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
editor.SetPalette(TextEditor::GetDarkPalette());
editor.SetTabSize(4);
editor.SetShowWhitespaces(false);

// Set font size for readability
ImGui::PushFont(codeFont);  // 18pt monospace
editor.Render("##code_editor");
ImGui::PopFont();
```

The editor feels like a real IDE. You can type code naturally, and it looks professional.

### 3.3 Markdown Cells

Documentation is crucial for notebooks. We support markdown cells that can be toggled between edit and preview modes.

In edit mode, you type markdown:
```markdown
# Introduction
This is a **test** with `code` inline.
```

In preview mode, imgui_markdown renders it with proper formatting:
- H1 headers at 24pt
- H2 headers at 20pt
- H3 headers at 17pt
- Bold, italic, inline code support

This lets you write literate programs—mixing explanation with code, just like research papers or documentation.

### 3.4 The Sidebar Inspector

On the left side, we show a collapsible sidebar with multiple panels:

**Sections Panel**: Lists all markdown headings hierarchically
**Variables Panel**: Shows all defined variables with types
**Functions Panel**: Lists function signatures  
**Classes Panel**: Shows defined classes/structs
**Variable Inspector**: Detailed view of variable types and values

This gives you a bird's-eye view of your notebook's structure. You can see what's defined, what types everything has, and navigate large notebooks easily.

---

## 4. Technical Deep Dive

### 4.1 Variable Update Semantics

The trickiest part was handling variable updates correctly. Consider:

```cpp
// Cell 1
int x = 5;

// Cell 2  
x = 10;

// Cell 3
int x = 15;  // Is this an error or an update?
```

In normal C++, cell 3 would be a redefinition error. But in a notebook, the user probably means "change x to 15," not "create a new variable."

Our solution: when we see `int x = 15` and x already exists in the context, we treat it as an update, not a redeclaration. We replace the stored value with the new initialization.

This required smart parsing:

```cpp
bool isVariableDeclaration(const string& line) {
    // Extract type and variable name
    if (matches(line, VARIABLE_PATTERN)) {
        string varName = extractVariableName(line);
        if (context.variables.count(varName)) {
            // Variable exists - treat as update
            context.variables[varName] = line;
            return true;  // But don't add to program twice
        }
        return true;  // New variable
    }
    return false;
}
```

This makes the notebook feel natural. You can rerun cells that declare variables without errors.

### 4.2 Function Overloading and Templates

C++ allows function overloading:

```cpp
int add(int a, int b) { return a + b; }
double add(double a, double b) { return a + b; }
```

And templates:

```cpp
template<typename T>
T maximum(T a, T b) { return a > b ? a : b; }
```

Our context needs to handle multiple functions with the same name. We store functions by signature, not just name:

```cpp
map<string, string> functions;
// Key: "add_int_int"
// Key: "add_double_double"  
// Key: "maximum_template"
```

When reconstructing the program, we emit all versions. The C++ compiler handles overload resolution normally.

### 4.3 Error Handling and Recovery

Compilation errors need to be clear. When g++ fails, it outputs cryptic messages with line numbers that don't match the user's cell (because we generated a program with context).

We parse compiler output and adjust line numbers:

```cpp
string adjustErrorMessages(const string& compilerOutput, int contextLines) {
    // Compiler says: "error at line 25"
    // But user's cell starts at line 25 in generated program
    // After removing context (first 20 lines), it's actually line 5
    
    for (auto& line : splitLines(compilerOutput)) {
        if (matches(line, ERROR_PATTERN)) {
            int lineNum = extractLineNumber(line);
            lineNum -= contextLines;  // Adjust to cell line number
            replacedLine = replaceLineNumber(line, lineNum);
        }
    }
    return adjustedOutput;
}
```

This makes errors appear to reference the user's code, not the generated program.

### 4.4 Performance Optimization

Compiling C++ is slow. For large contexts, each cell execution could take seconds. We optimize:

**1. Incremental Headers**
Only include headers that are actually used. We scan the context for STL types:

```cpp
set<string> usedHeaders;
if (contextHasVector) usedHeaders.insert("<vector>");
if (contextHasMap) usedHeaders.insert("<map>");
// ...
```

**2. Precompiled Headers (Future Work)**
We could precompile common headers and reuse them. Not implemented yet, but would significantly speed up compilation.

**3. Minimal Rebuilds**
If a cell doesn't modify the context (just runs a statement), we could cache the previous compilation. Also future work.

**4. Parallel Compilation**
For notebooks with independent cells, we could compile multiple cells in parallel. Requires dependency analysis.

---

## 5. Implementation Details

### 5.1 Project Structure

```
cppnb/
├── main.cpp              # Application entry point, SDL/OpenGL setup
├── src/
│   ├── ui/
│   │   ├── notebook.hpp  # Cell, ExecutionContext, ExecutionEngine
│   │   └── notebook.cpp  # Core implementation
│   └── utils/
│       ├── logger.hpp    # Debug logging utilities
│       └── logger.cpp
├── third_party/
│   ├── imgui/           # Immediate mode GUI library
│   ├── TextEditor/      # Syntax highlighting editor widget
│   ├── imgui_markdown/  # Markdown rendering
│   ├── implot/          # Plotting (future use)
│   └── glad/            # OpenGL function loader
└── CMakeLists.txt       # Build configuration
```

### 5.2 Key Classes

**Cell**
```cpp
struct Cell {
    enum Type { CODE, MARKDOWN };
    Type type;
    string content;
    string output;
    bool hasError;
    int executionCount;
    double executionTime;
    TextEditor editor;  // For code cells
};
```

**ExecutionContext**
```cpp
class ExecutionContext {
private:
    map<string, string> variables;
    map<string, string> functions;
    map<string, string> classes;
    set<string> headers;
    
public:
    void addVariable(const string& name, const string& decl);
    void addFunction(const string& signature, const string& definition);
    void addClass(const string& name, const string& definition);
    string generateProgram(const string& cellCode);
    void reset();  // Clear all context
};
```

**ExecutionEngine**
```cpp
class ExecutionEngine {
private:
    ExecutionContext context;
    
public:
    struct Result {
        string output;
        double executionTime;
        string error;
    };
    
    Result executeCell(const string& code);
    void resetContext();
};
```

### 5.3 Build System

We use CMake for cross-platform building:

```cmake
cmake_minimum_required(VERSION 3.10)
project(cppnb)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL2
find_package(SDL2 REQUIRED)

# ImGui
add_subdirectory(third_party/imgui)

# TextEditor  
add_subdirectory(third_party/TextEditor)

# Main executable
add_executable(cppnb
    main.cpp
    src/ui/notebook.cpp
    src/utils/logger.cpp
    third_party/glad/src/glad.c
)

target_link_libraries(cppnb
    SDL2::SDL2
    imgui
    TextEditor
    ${CMAKE_DL_LIBS}
)
```

This handles dependencies automatically. Just run `cmake .. && make`.

---

## 6. Usage Examples

### 6.1 Basic Variable Persistence

```cpp
// Cell 1: Declare variables
int x = 10;
double pi = 3.14159;
string name = "cppnb";
```

```cpp
// Cell 2: Use them
cout << "x = " << x << endl;
cout << "pi = " << pi << endl;
cout << "name = " << name << endl;
```

```cpp
// Cell 3: Modify and reuse
x *= 2;
cout << "x doubled = " << x << endl;
```

Output:
```
x = 10
pi = 3.14159
name = cppnb
x doubled = 20
```

### 6.2 Function Definitions

```cpp
// Cell 1: Define a function
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}
```

```cpp
// Cell 2: Use it
for (int i = 0; i < 10; i++) {
    cout << fibonacci(i) << " ";
}
cout << endl;
```

Output:
```
0 1 1 2 3 5 8 13 21 34
```

```cpp
// Cell 3: Define another function using the first
void printFibonacci(int count) {
    for (int i = 0; i < count; i++) {
        cout << fibonacci(i) << " ";
    }
    cout << endl;
}

printFibonacci(7);
```

Output:
```
0 1 1 2 3 5 8
```

### 6.3 Classes and Objects

```cpp
// Cell 1: Define a class
class Point {
public:
    double x, y;
    
    Point(double x, double y) : x(x), y(y) {}
    
    double distanceFromOrigin() {
        return sqrt(x*x + y*y);
    }
    
    void print() {
        cout << "(" << x << ", " << y << ")" << endl;
    }
};
```

```cpp
// Cell 2: Use the class
Point p1(3.0, 4.0);
p1.print();
cout << "Distance: " << p1.distanceFromOrigin() << endl;
```

Output:
```
(3, 4)
Distance: 5
```

### 6.4 Template Functions

```cpp
// Cell 1: Define template
template<typename T>
T maximum(T a, T b) {
    return (a > b) ? a : b;
}
```

```cpp
// Cell 2: Use with different types
cout << "max(10, 20) = " << maximum(10, 20) << endl;
cout << "max(3.14, 2.71) = " << maximum(3.14, 2.71) << endl;
cout << "max('a', 'z') = " << maximum('a', 'z') << endl;
```

Output:
```
max(10, 20) = 20
max(3.14, 2.71) = 3.14
max('a', 'z') = z
```

### 6.5 STL Containers

```cpp
// Cell 1: Create and populate a vector
vector<int> numbers = {1, 2, 3, 4, 5};
```

```cpp
// Cell 2: Modify it
numbers.push_back(6);
numbers.push_back(7);
```

```cpp
// Cell 3: Use algorithms
#include <algorithm>

auto it = find(numbers.begin(), numbers.end(), 4);
if (it != numbers.end()) {
    cout << "Found 4 at position " << (it - numbers.begin()) << endl;
}

int sum = accumulate(numbers.begin(), numbers.end(), 0);
cout << "Sum = " << sum << endl;
```

Output:
```
Found 4 at position 3
Sum = 28
```

---

## 7. Comparison with Existing Tools

### 7.1 Cling (ROOT's C++ Interpreter)

Cling is a true C++ interpreter based on Clang/LLVM. It provides a REPL for C++.

**Advantages of Cling:**
- True interpretation, no compilation delay
- Better integration with C++ semantics
- Backed by CERN, used in production

**Advantages of cppnb:**
- Full notebook interface with markdown, visualization
- Simpler architecture (no LLVM dependency)
- Easier to understand and modify
- Native GUI integration

### 7.2 Jupyter C++ Kernels (xeus-cling)

Jupyter with xeus-cling provides C++ notebooks in the web browser.

**Advantages of xeus-cling:**
- Runs in browser, no installation
- Integrates with Jupyter ecosystem
- Can use other Jupyter features

**Advantages of cppnb:**
- Native application, better performance
- No web server required
- Offline use
- Tighter integration with desktop workflows

### 7.3 Online Compilers (Compiler Explorer, godbolt)

Websites like godbolt.org let you write C++ and see compilation results.

**Advantages of online compilers:**
- No installation needed
- See assembly output
- Compare multiple compilers

**Advantages of cppnb:**
- Persistent state across cells
- Notebook workflow
- Local execution, no internet required
- Variable inspection and debugging features

---

## 8. Limitations and Future Work

### 8.1 Current Limitations

**1. No True Debugger**
We can't set breakpoints or step through code. Each cell runs to completion. Adding a debugger would require integrating GDB or LLDB.

**2. Limited Multi-file Support**
All code exists in a single context. You can't easily split code across multiple files or create proper modules.

**3. No Visualization**
While we have ImPlot integrated, we haven't exposed plotting functions yet. Adding visualization would make the tool much more powerful for data analysis.

**4. Compilation Speed**
For large contexts, compilation can be slow. Precompiled headers and caching could help.

**5. Error Messages**
While we adjust line numbers, error messages could be clearer. Better parsing and formatting would improve the user experience.

**6. No Package Management**
There's no way to easily install and use third-party libraries. Adding something like vcpkg integration would be useful.

### 8.2 Planned Enhancements

**Export/Import Notebooks**
Save and load notebooks in a custom `.cppnb` format. Also export to standalone `.cpp` files or markdown documents.

**Plotting and Visualization**
Integrate ImPlot properly. Allow cells to generate graphs, charts, and custom visualizations.

**Code Intelligence**
Add auto-completion, parameter hints, and real-time error checking using clangd or similar tools.

**Collaborative Features**
Share notebooks via URL. Real-time collaboration. Comments and annotations.

**Multi-language Support**
Mix C++ with Python, shell commands, or SQL queries in the same notebook.

**Better Error Recovery**
If a cell fails to compile, provide quick-fix suggestions. Highlight the specific line causing the error.

---

## 9. Getting Started

### 9.1 Installation

**macOS:**
```bash
# Install dependencies
brew install cmake sdl2

# Clone repository  
git clone https://github.com/Jeevan-04/cpp-notebook.git
cd cpp-notebook

# Build
mkdir build && cd build
cmake ..
make -j4

# Run
./cppnb
```

**Linux (Ubuntu/Debian):**
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install cmake libsdl2-dev libgl1-mesa-dev build-essential

# Clone and build
git clone https://github.com/Jeevan-04/cpp-notebook.git
cd cpp-notebook
mkdir build && cd build
cmake ..
make -j4

# Run  
./cppnb
```

**Windows:**
1. Install Visual Studio 2019 or later
2. Install CMake from cmake.org
3. Download SDL2 from libsdl.org
4. Clone repository and open in Visual Studio
5. Build and run

### 9.2 First Steps

1. **Start the application**
   ```bash
   ./cppnb
   ```

2. **Create your first cell**
   The interface starts with an empty code cell. Type:
   ```cpp
   int x = 42;
   cout << "Hello, C++! x = " << x << endl;
   ```

3. **Run the cell**
   Click the `[RUN]` button or press `Shift+Enter`

4. **See the output**
   Output appears below the cell with execution time

5. **Add another cell**
   Click `Insert → Code Cell` or press `Ctrl+Shift+C`

6. **Use the previous variable**
   In the new cell:
   ```cpp
   x *= 2;
   cout << "x doubled = " << x << endl;
   ```

7. **Run it**
   The variable persists! You'll see `x doubled = 84`

8. **Add documentation**
   Create a markdown cell with `Ctrl+Shift+M`:
   ```markdown
   # My First C++ Notebook
   
   This demonstrates variable persistence!
   ```

9. **Toggle preview**
   Click the eye icon to see rendered markdown

10. **Explore the sidebar**
    Press `Ctrl+B` to toggle the inspector showing all variables and functions

### 9.3 Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Shift+Enter` | Run current cell |
| `Ctrl+Shift+C` | Insert code cell |
| `Ctrl+Shift+M` | Insert markdown cell |
| `Ctrl+B` | Toggle sidebar |
| `Ctrl+Shift+R` | Reset kernel (clear all variables) |
| `Ctrl+Shift+O` | Clear all outputs |
| `Cmd+C` / `Cmd+V` | Copy/paste (inside cells) |
| `Cmd+Z` | Undo |
| `Cmd+Shift+Z` | Redo |

---

## 10. Conclusion

We've built a system that brings notebook-style interactivity to C++ without modifying the language or compiler. The key insight was to maintain a global execution context and reconstruct complete programs for each cell execution.

This approach has trade-offs. Compilation is slower than interpretation. Error messages can be confusing. But we get real C++—with templates, compile-time optimization, and type safety—in an interactive environment.

The system demonstrates that compiled languages can support exploratory programming. You don't need an interpreter to have interactive development. You just need clever compilation strategies.

cppnb makes C++ more accessible for learning, prototyping, and algorithm development. It shows that the rigid compile-link-run cycle isn't inevitable. With the right tools, C++ can be as interactive as Python, while keeping its performance and power.

The code is open source. Try it. Break it. Improve it. Build something cool.

---

## Appendix A: Original Feature Documentation

## 🌟 Features

### Core Functionality
- **📝 Code Cells** - Write and execute C++ code interactively
- **📄 Markdown Cells** - Document your work with rich text formatting
- **🔄 Persistent Context** - Variables, functions, and classes persist across cells
- **⚡ Fast Compilation** - Quick feedback with optimized build pipeline
- **🎨 Modern UI** - Clean, dark-themed interface with syntax highlighting

### Advanced Features
- **📊 Variable Inspector** - Real-time view of all defined variables with types
- **🗂️ Sidebar Outline** - Navigate through sections, functions, classes, and variables
- **⌨️ Native Keyboard Shortcuts** - Full Cmd+C/V/Z support
- **🔍 Smart Execution** - Automatic detection of variable declarations, functions, and classes
- **💾 Execution History** - Track execution order with cell counters
- **🎯 Error Handling** - Clear, color-coded error messages with line numbers

### UI Highlights
- **Large, Readable Fonts** - 18pt code editor, 24pt headings
- **Live Markdown Preview** - Toggle between edit and preview modes
- **Collapsible Output** - Clean interface with expandable results
- **Auto-sizing Cells** - Cells grow/shrink based on content
- **No Scrollbars** - Smooth, modern editing experience

---

## 🚀 Quick Start

### Prerequisites

**macOS**:
```bash
# Install dependencies via Homebrew
brew install cmake sdl2
```

**Linux** (Ubuntu/Debian):
```bash
sudo apt-get install cmake libsdl2-dev libgl1-mesa-dev
```

**Windows**:
- Install [CMake](https://cmake.org/download/)
- Install [SDL2](https://github.com/libsdl-org/SDL/releases)
- Visual Studio 2019 or later

### Building

```bash
# Clone the repository
cd /path/to/cppnb

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Build (parallel compilation)
make -j4

# Run
./cppnb
```

### Quick Test

Once the application starts:

1. **Cell 1**: Declare a variable
   ```cpp
   int x = 42;
   ```
   Click `[RUN]` or press `Shift+Enter`

2. **Cell 2**: Define a function
   ```cpp
   void greet() {
       cout << "Hello! x = " << x << endl;
   }
   ```

3. **Cell 3**: Call the function
   ```cpp
   greet();
   ```

4. **Cell 4**: Update the variable
   ```cpp
   x = 100;
   greet();
   ```

---

## 📖 User Guide

### Cell Types

#### Code Cells
- Execute C++ code
- Support variable declarations, functions, classes
- Show execution time and output
- Syntax highlighting with C++20 support

#### Markdown Cells
- Write documentation using Markdown syntax
- Support headings (`#`, `##`, `###`)
- Toggle between edit and preview modes
- Larger fonts for headings (H1=24pt, H2=20pt, H3=17pt)

### Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| `Cmd+C` | Copy (in editor) |
| `Cmd+V` | Paste (in editor) |
| `Cmd+Z` | Undo |
| `Cmd+Shift+Z` | Redo |
| `Cmd+A` | Select All |
| `Shift+Enter` | Run Current Cell |
| `Ctrl+B` | Toggle Sidebar |
| `Ctrl+Shift+C` | Insert Code Cell |
| `Ctrl+Shift+M` | Insert Markdown Cell |
| `Ctrl+Shift+R` | Reset Kernel |
| `Ctrl+Shift+O` | Clear All Outputs |

> **Note**: Click inside a cell to focus it before using keyboard shortcuts

### Menu Bar

#### File Menu
- **New Notebook** (`Ctrl+N`) - Clear all cells and start fresh
- Automatically saves execution context

#### Edit Menu
- **Clear All Outputs** (`Ctrl+Shift+O`) - Remove all cell outputs
- **Reset Kernel** (`Ctrl+Shift+R`) - Clear all variables, functions, and classes

#### Insert Menu
- **Code Cell** (`Ctrl+Shift+C`) - Add new code cell
- **Markdown Cell** (`Ctrl+Shift+M`) - Add new markdown cell

#### View Menu
- **Show Sidebar** (`Ctrl+B`) - Toggle outline panel

#### Help Menu
- View all keyboard shortcuts
- Quick reference guide

### Sidebar Features

#### [ Sections ]
- Displays all markdown headings from the notebook
- Hierarchical view with proper indentation
- Click to navigate (planned feature)

#### [ Functions ]
- Lists all defined functions with signatures
- Shows return types and parameters
- Color-coded in green

#### [ Variables ]
- Shows all global variables with types
- Color-coded in yellow/orange
- Updates in real-time

#### [ Classes ]
- Displays all defined classes and structs
- Color-coded in purple
- Shows class names

#### [ Variable Inspector ]
- Detailed view of variable types and values
- Shows initialization values
- Updates as variables are modified

---

## 🧠 How It Works

### Execution Model

**Persistent Context**: Unlike traditional C++ compilers, cppnb maintains a persistent execution context:

1. **Variables** are stored globally and persist across cells
2. **Functions** are accumulated and available to all subsequent cells
3. **Classes** are defined once and reused
4. **Each cell execution** rebuilds the complete program with:
   - All accumulated headers
   - All global variables (with latest values)
   - All class definitions
   - All function definitions
   - Current cell code in `main()`

### Variable Updates

```cpp
// Cell 1
int a = 5;        // a is stored globally

// Cell 2
void printA() {
    cout << "a = " << a << endl;
}

// Cell 3
printA();         // Output: a = 5

// Cell 4
a = 10;           // Updates global variable

// Cell 5
printA();         // Output: a = 10  ✅
```

**Smart Variable Handling**:
- `int a = 5;` → Declares new variable or updates existing one
- `a = 10;` → Updates existing variable value
- No redefinition errors!

### Compilation Pipeline

1. **Parse** current cell code
2. **Detect** variable declarations, functions, classes
3. **Update** execution context
4. **Build** complete C++ program:
   ```cpp
   #include <iostream>
   #include <vector>
   // ... other headers
   using namespace std;
   
   // All global variables
   int a = 10;
   
   // All classes
   class MyClass { ... };
   
   // All functions
   void myFunction() { ... }
   
   // Main with current cell code
   int main() {
       // Current cell code here
       return 0;
   }
   ```
5. **Compile** with `g++ -std=c++20`
6. **Execute** and capture output
7. **Display** results with execution time

---

## 🎨 UI Customization

### Font Sizes
- **Default UI**: 16pt
- **Code Editor**: 18pt
- **Output Text**: 18pt
- **Markdown H1**: 24pt
- **Markdown H2**: 20pt
- **Markdown H3**: 17pt

### Color Scheme
- **Success**: Green (`#33CC66`)
- **Error**: Red (`#FF5555`)
- **Warning**: Orange (`#FFAA33`)
- **Accent**: Blue (`#4D9FFF`)
- **Code**: Syntax-highlighted with C++ grammar

### Cell Appearance
- **Jupyter-style layout**: `In [n]:` labels on the left
- **Auto-sizing**: Cells grow based on content
- **No borders**: Clean, borderless editing
- **Rounded buttons**: Modern button styling

---

## 📁 Project Structure

```
cppnb/
├── main.cpp                    # Application entry point
├── CMakeLists.txt             # Build configuration
├── README.md                  # This file
├── src/
│   └── ui/
│       ├── notebook.hpp       # Cell and execution engine headers
│       └── notebook.cpp       # Core notebook implementation
├── third_party/
│   ├── imgui/                 # ImGui library
│   ├── imgui_markdown/        # Markdown rendering
│   ├── implot/                # Plotting library (future)
│   ├── TextEditor/            # Code editor widget
│   └── glad/                  # OpenGL loader
├── build/                     # Build artifacts
└── Fonts/                     # Font files (optional)
```

### Key Components

#### `ExecutionContext`
Manages persistent state across cells:
- `variables` - Global variable declarations
- `functions` - Function definitions
- `classes` - Class/struct definitions
- Smart update logic to prevent redefinitions

#### `ExecutionEngine`
Handles code compilation and execution:
- Builds complete C++ programs
- Manages temporary files
- Compiles with g++
- Captures output and errors
- Reports execution time

#### `CodeParser`
Analyzes cell code to determine:
- Is it a variable declaration?
- Is it a function definition?
- Is it a class definition?
- Is it a regular statement?

#### `drawNotebook()`
Main UI rendering function:
- Sidebar with outline and inspector
- Cell rendering (code and markdown)
- Button handlers
- Keyboard input processing

---

## 🔧 Advanced Usage

### Defining Functions

```cpp
// Cell 1: Define a function
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);
}

// Cell 2: Use it
for (int i = 0; i < 10; i++) {
    cout << fibonacci(i) << " ";
}
// Output: 0 1 1 2 3 5 8 13 21 34
```

### Creating Classes

```cpp
// Cell 1: Define a class
class Point {
public:
    int x, y;
    Point(int x, int y) : x(x), y(y) {}
    void print() {
        cout << "(" << x << ", " << y << ")" << endl;
    }
};

// Cell 2: Use it
Point p(10, 20);
p.print();
// Output: (10, 20)
```

### Using STL Containers

```cpp
// Cell 1: Create a vector
vector<int> numbers = {1, 2, 3, 4, 5};

// Cell 2: Modify it
numbers.push_back(6);
numbers.push_back(7);

// Cell 3: Print it
for (int n : numbers) {
    cout << n << " ";
}
// Output: 1 2 3 4 5 6 7
```

### Template Functions

```cpp
// Cell 1: Define template
template<typename T>
T maximum(T a, T b) {
    return (a > b) ? a : b;
}

// Cell 2: Use with different types
cout << maximum(10, 20) << endl;           // 20
cout << maximum(3.14, 2.71) << endl;       // 3.14
cout << maximum('a', 'z') << endl;         // z
```

---

## 🐛 Troubleshooting

### Common Issues

#### "Command not found: cmake"
**Solution**: Install CMake via Homebrew
```bash
brew install cmake
```

#### "Library not loaded: libSDL2"
**Solution**: Install SDL2
```bash
brew install sdl2
```

#### Keyboard shortcuts not working
**Solution**: 
1. Click inside the code editor to focus it
2. Make sure you're using Cmd (not Ctrl) on macOS
3. Check Help menu for correct shortcuts

#### Variable not updating
**Problem**: Using `int a = 5` in multiple cells
**Solution**: 
- First cell: `int a = 5;` (declares)
- Later cells: `a = 10;` (updates without `int`)
- Or use `int a = 10;` to redeclare (will update automatically)

#### Compilation errors
**Check**:
- Syntax is correct
- All necessary headers included
- Previous cells defined required functions/classes

#### Slow compilation
**Solution**:
- Use `-j4` flag when building: `make -j4`
- Clear old temp files: `rm -rf /tmp/notebook_temp_*.cpp*`

---

## 🚀 Future Enhancements

### Planned Features

#### Export/Import
- [ ] Save notebooks to `.cppnb` format
- [ ] Export to single `.cpp` file
- [ ] Export to `.md` with code blocks
- [ ] Import existing notebooks

#### Visualization
- [ ] Integrate ImPlot for graphs
- [ ] Image display support
- [ ] Data structure visualization
- [ ] Performance graphs

#### Debugger
- [ ] Breakpoint support
- [ ] Step-through execution
- [ ] Watch variables
- [ ] Call stack view

#### Code Intelligence
- [ ] Auto-completion
- [ ] IntelliSense
- [ ] Real-time syntax checking
- [ ] Parameter hints

#### Collaboration
- [ ] Share notebooks via URL
- [ ] Real-time collaboration
- [ ] Comments on cells
- [ ] Version control integration

#### Multi-language
- [ ] Python cells
- [ ] Shell command cells
- [ ] SQL queries
- [ ] Mix languages in one notebook

---

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. **Report Bugs**: Open an issue with detailed reproduction steps
2. **Suggest Features**: Share your ideas in the issues section
3. **Submit PRs**: Fork, create a branch, make changes, and submit a pull request
4. **Improve Docs**: Help make this README even better

### Development Setup

```bash
# Fork and clone
git clone https://github.com/yourusername/cppnb.git
cd cppnb

# Create a branch
git checkout -b feature/your-feature-name

# Make changes and test
mkdir build && cd build
cmake ..
make -j4
./cppnb

# Submit PR
git push origin feature/your-feature-name
```

---

## 🙏 Acknowledgments

Built with amazing open-source libraries:
- **[ImGui](https://github.com/ocornut/imgui)** - Immediate Mode GUI
- **[SDL2](https://www.libsdl.org/)** - Cross-platform multimedia
- **[TextEditor](https://github.com/BalazsJako/ImGuiColorTextEdit)** - Syntax highlighting editor
- **[imgui_markdown](https://github.com/juliettef/imgui_markdown)** - Markdown rendering
- **[ImPlot](https://github.com/epezent/implot)** - Plotting library
- **[glad](https://glad.dav1d.de/)** - OpenGL loader

---

**Made with ❤️ for the C++ community**

*Version 0.1.0 - March 2025*

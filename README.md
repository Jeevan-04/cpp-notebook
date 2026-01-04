# C++ Notebook (cppnb)

A modern, interactive C++ notebook environment inspired by Jupyter, built with C++20, ImGui, and SDL2. Write, execute, and visualize C++ code in an intuitive cell-based interface.

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![C++](https://img.shields.io/badge/C++-20-00599C?logo=cplusplus)
![Platform](https://img.shields.io/badge/platform-macOS-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)

---

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

## 📄 License

MIT License - see LICENSE file for details

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

## 📞 Support

- **Issues**: [GitHub Issues](https://github.com/yourusername/cppnb/issues)
- **Discussions**: [GitHub Discussions](https://github.com/yourusername/cppnb/discussions)
- **Email**: your.email@example.com

---

## ⭐ Star Us!

If you find this project useful, please consider giving it a star on GitHub!

---

**Made with ❤️ for the C++ community**

*Version 1.0.0 - November 2025*

# MikoView Makefile for Linux
# This provides convenient commands for building and managing the project

# Default build type
BUILD_TYPE ?= Release
BUILD_DIR ?= build
INSTALL_PREFIX ?= /usr/local

# Colors for output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[1;33m
BLUE = \033[0;34m
NC = \033[0m # No Color

# Default target
.PHONY: all
all: build

# Help target
.PHONY: help
help:
	@echo "$(BLUE)MikoView Build System$(NC)"
	@echo "Available targets:"
	@echo "  $(GREEN)build$(NC)          - Build the project (default: Release)"
	@echo "  $(GREEN)debug$(NC)          - Build in Debug mode"
	@echo "  $(GREEN)release$(NC)        - Build in Release mode"
	@echo "  $(GREEN)clean$(NC)          - Clean build directory"
	@echo "  $(GREEN)rebuild$(NC)        - Clean and build"
	@echo "  $(GREEN)install$(NC)        - Install the project"
	@echo "  $(GREEN)uninstall$(NC)      - Uninstall the project"
	@echo "  $(GREEN)run$(NC)            - Build and run the application"
	@echo "  $(GREEN)run-debug$(NC)      - Build and run in debug mode"
	@echo "  $(GREEN)deps$(NC)           - Install system dependencies"
	@echo "  $(GREEN)setup-dev$(NC)      - Setup development environment"
	@echo "  $(GREEN)format$(NC)         - Format source code"
	@echo "  $(GREEN)lint$(NC)           - Run static analysis"
	@echo "  $(GREEN)package$(NC)        - Create distribution package"
	@echo "  $(GREEN)renderer-dev$(NC)   - Start React development server"
	@echo "  $(GREEN)renderer-build$(NC) - Build React production bundle"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE={Debug|Release}  (default: Release)"
	@echo "  BUILD_DIR=<directory>       (default: build)"
	@echo "  INSTALL_PREFIX=<path>       (default: /usr/local)"
	@echo ""
	@echo "Examples:"
	@echo "  make build BUILD_TYPE=Debug"
	@echo "  make install INSTALL_PREFIX=~/.local"

# Configure CMake
.PHONY: configure
configure:
	@echo "$(YELLOW)Configuring CMake for $(BUILD_TYPE) build...$(NC)"
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_PREFIX) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
		..

# Build the project
.PHONY: build
build: configure
	@echo "$(YELLOW)Building MikoView ($(BUILD_TYPE))...$(NC)"
	@cd $(BUILD_DIR) && make -j$$(nproc)
	@echo "$(GREEN)Build completed successfully!$(NC)"

# Debug build
.PHONY: debug
debug:
	@$(MAKE) build BUILD_TYPE=Debug

# Release build
.PHONY: release
release:
	@$(MAKE) build BUILD_TYPE=Release

# Clean build directory
.PHONY: clean
clean:
	@echo "$(YELLOW)Cleaning build directory...$(NC)"
	@rm -rf $(BUILD_DIR)
	@echo "$(GREEN)Clean completed!$(NC)"

# Rebuild (clean + build)
.PHONY: rebuild
rebuild: clean build

# Install the project
.PHONY: install
install: build
	@echo "$(YELLOW)Installing MikoView to $(INSTALL_PREFIX)...$(NC)"
	@cd $(BUILD_DIR) && sudo make install
	@echo "$(GREEN)Installation completed!$(NC)"

# Uninstall the project
.PHONY: uninstall
uninstall:
	@echo "$(YELLOW)Uninstalling MikoView...$(NC)"
	@sudo rm -f $(INSTALL_PREFIX)/bin/MikoView
	@sudo rm -rf $(INSTALL_PREFIX)/include/mikoview
	@sudo rm -f $(INSTALL_PREFIX)/include/mikoview.hpp
	@sudo rm -f $(INSTALL_PREFIX)/lib/libmikoview_framework.a
	@echo "$(GREEN)Uninstallation completed!$(NC)"

# Run the application
.PHONY: run
run: build
	@echo "$(YELLOW)Running MikoView...$(NC)"
	@cd $(BUILD_DIR)/$(BUILD_TYPE) && ./MikoView

# Run in debug mode
.PHONY: run-debug
run-debug:
	@$(MAKE) run BUILD_TYPE=Debug

# Install system dependencies
.PHONY: deps
deps:
	@echo "$(YELLOW)Installing system dependencies...$(NC)"
	@if command -v apt-get >/dev/null 2>&1; then \
		sudo apt-get update && \
		sudo apt-get install -y \
			build-essential cmake ninja-build \
			libx11-dev libxrandr-dev libxinerama-dev \
			libxcursor-dev libxi-dev libgl1-mesa-dev \
			libasound2-dev libpulse-dev \
			pkg-config git curl wget \
			python3 python3-pip; \
	elif command -v dnf >/dev/null 2>&1; then \
		sudo dnf install -y \
			gcc-c++ cmake ninja-build \
			libX11-devel libXrandr-devel libXinerama-devel \
			libXcursor-devel libXi-devel mesa-libGL-devel \
			alsa-lib-devel pulseaudio-libs-devel \
			pkgconf git curl wget \
			python3 python3-pip; \
	elif command -v pacman >/dev/null 2>&1; then \
		sudo pacman -S --needed \
			base-devel cmake ninja \
			libx11 libxrandr libxinerama \
			libxcursor libxi mesa \
			alsa-lib libpulse \
			pkgconf git curl wget \
			python python-pip; \
	else \
		echo "$(RED)Unsupported package manager. Please install dependencies manually.$(NC)"; \
		exit 1; \
	fi
	@echo "$(GREEN)Dependencies installed successfully!$(NC)"

# Setup development environment
.PHONY: setup-dev
setup-dev: deps
	@echo "$(YELLOW)Setting up development environment...$(NC)"
	@# Install Node.js if not present
	@if ! command -v node >/dev/null 2>&1; then \
		curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash - && \
		sudo apt-get install -y nodejs; \
	fi
	@# Install Bun if not present
	@if ! command -v bun >/dev/null 2>&1; then \
		curl -fsSL https://bun.sh/install | bash && \
		export PATH="$$HOME/.bun/bin:$$PATH"; \
	fi
	@# Setup renderer dependencies
	@cd renderer/react && bun install
	@echo "$(GREEN)Development environment setup completed!$(NC)"

# Format source code
.PHONY: format
format:
	@echo "$(YELLOW)Formatting source code...$(NC)"
	@if command -v clang-format >/dev/null 2>&1; then \
		find . -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | \
		xargs clang-format -i -style=file; \
		echo "$(GREEN)Code formatting completed!$(NC)"; \
	else \
		echo "$(RED)clang-format not found. Please install it first.$(NC)"; \
	fi

# Run static analysis
.PHONY: lint
lint:
	@echo "$(YELLOW)Running static analysis...$(NC)"
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c++17 \
			--suppress=missingIncludeSystem \
			--suppress=unusedFunction \
			mikoview/ example/ 2>&1 | \
		grep -v "Checking " || true; \
		echo "$(GREEN)Static analysis completed!$(NC)"; \
	else \
		echo "$(RED)cppcheck not found. Please install it first.$(NC)"; \
	fi

# Create distribution package
.PHONY: package
package: release
	@echo "$(YELLOW)Creating distribution package...$(NC)"
	@mkdir -p dist
	@cd $(BUILD_DIR) && make package
	@cp $(BUILD_DIR)/*.tar.gz dist/ 2>/dev/null || true
	@cp $(BUILD_DIR)/*.deb dist/ 2>/dev/null || true
	@cp $(BUILD_DIR)/*.rpm dist/ 2>/dev/null || true
	@echo "$(GREEN)Package created in dist/ directory!$(NC)"

# React development server
.PHONY: renderer-dev
renderer-dev:
	@echo "$(YELLOW)Starting React development server...$(NC)"
	@cd renderer/react && bun run dev

# Build React production bundle
.PHONY: renderer-build
renderer-build:
	@echo "$(YELLOW)Building React production bundle...$(NC)"
	@cd renderer/react && bun run build
	@echo "$(GREEN)React build completed!$(NC)"

# Development workflow targets
.PHONY: dev
dev: debug renderer-dev

.PHONY: prod
prod: release renderer-build

# Quick development cycle
.PHONY: quick
quick:
	@$(MAKE) build BUILD_TYPE=Debug -j$$(nproc)
	@$(MAKE) run BUILD_TYPE=Debug

# Show build information
.PHONY: info
info:
	@echo "$(BLUE)MikoView Build Information$(NC)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Build Directory: $(BUILD_DIR)"
	@echo "Install Prefix: $(INSTALL_PREFIX)"
	@echo "CPU Cores: $$(nproc)"
	@echo "CMake Version: $$(cmake --version | head -n1)"
	@echo "GCC Version: $$(gcc --version | head -n1)"
	@if [ -f $(BUILD_DIR)/compile_commands.json ]; then \
		echo "Compile Commands: Available"; \
	else \
		echo "Compile Commands: Not generated"; \
	fi